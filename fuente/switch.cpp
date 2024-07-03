#include "switch.hpp"
#include <cstdio> // Incluir la biblioteca estándar de E/S para printf

MacEntry macTable[MAX_MAC_ENTRIES];
int entryCount = 0;
int buffer_count[MAX_PORTS] = {0};

void printMacTable() {
    printf("Contenido de la tabla MAC:\n");
    for (int i = 0; i < entryCount; i++) {
        printf("Entrada %d: MAC = %llx, Puerto = %u\n", i + 1, macTable[i].macAddress.to_uint64(), macTable[i].port.to_uint());
    }
    printf("--------------------------------------\n");
}



bool compareMAC(const ap_uint<48>& mac1, const ap_uint<48>& mac2) {
    return mac1 == mac2;
}

int findMacEntry(const ap_uint<48>& mac) {
    for (int i = 0; i < entryCount; i++) {
        if (compareMAC(macTable[i].macAddress, mac)) {
            printf("MAC encontrada en la tabla: �?ndice = %d, MAC = %llx\n", i, mac.to_uint64());
            return i;
        }
    }
    printf("MAC no encontrada en la tabla: MAC = %llx\n", mac.to_uint64());
    return -1;
}

void moveEntryToFront(int index) {
    MacEntry temp = macTable[index];
    for (int i = index; i > 0; i--) {
        macTable[i] = macTable[i - 1];
    }
    macTable[0] = temp;
}

void updateMACEntry(int index, int newPort) {
    macTable[index].port = newPort;
    moveEntryToFront(index);
    printf("Entrada MAC actualizada y movida al frente: MAC = %llx, Puerto = %u\n", macTable[0].macAddress.to_uint64(), newPort);
}

void addNewMACEntry(const ap_uint<48>& mac, int port) {
    if (entryCount == MAX_MAC_ENTRIES) {
        moveEntryToFront(MAX_MAC_ENTRIES - 1);
    } else {
        moveEntryToFront(entryCount);
        entryCount++;
    }
    macTable[0].macAddress = mac;
    macTable[0].port = port;
    printf("Nueva entrada MAC añadida y movida al frente: MAC = %llx, Puerto = %u\n", mac, port);
}

void update_mac_table(const ap_uint<48>& src_mac, ap_uint<4> input_port) {
    printf("Actualizando tabla MAC: Src MAC = %llx, Puerto = %u\n", src_mac.to_uint64(), input_port.to_uint());
    int index = findMacEntry(src_mac);
    if (index != -1) {
        updateMACEntry(index, input_port);
        printf("Entrada MAC actualizada: �?ndice = %d\n", index);
    } else {
        addNewMACEntry(src_mac, input_port);
        printf("Nueva entrada MAC añadida: Src MAC = %llx, Puerto = %u\n", src_mac.to_uint64(), input_port.to_uint());
    }
}


void ensureMinimumFrameSize(ethernet_frame& frame) {
    int actualLength = 0;
    while (actualLength < 480 && frame.data[actualLength] != 0) {
        actualLength++;
    }

    if (actualLength < MIN_FRAME_SIZE) {
        for (int i = actualLength; i < MIN_FRAME_SIZE; i++) {
            frame.data[i] = 0;
        }
    }
}


bool isEthertypeSupported(const ap_uint<16>& eth_type) {
    return eth_type == 0x0800 ||  // IPv4
           eth_type == 0x0806 ||  // ARP
           eth_type == 0x86DD ||  // IPv6
           eth_type == 0x8100;    // VLAN
}

void process_frame(ethernet_frame& frame, hls::stream<ethernet_frame> output_port_streams[MAX_PORTS], ap_uint<4> input_port) {
    printf("Procesando trama: Dest MAC = %llx, Src MAC = %llx, Ethertype = %x\n", frame.dest_mac.to_uint64(), frame.src_mac.to_uint64(), frame.eth_type.to_uint());
    
    if (input_port < 0 || input_port >= MAX_PORTS) {
        printf("Error: Receiving port %u is out of valid range (0 to %d).\n", input_port.to_uint(), MAX_PORTS - 1);
        return;
    }

    if (!isEthertypeSupported(frame.eth_type)) {
        printf("Ethertype 0x%X not supported. Frame discarded.\n", frame.eth_type.to_uint());
        return;
    }

    update_mac_table(frame.src_mac, input_port);
    ensureMinimumFrameSize(frame);

    bool is_broadcast = (frame.dest_mac == 0xFFFFFFFFFFFF);
    if (is_broadcast) {
        for (int i = 0; i < MAX_PORTS; i++) {
            if (i != input_port) {
                output_port_streams[i].write(frame);
                printf("Trama de broadcast enviada a puerto %d\n", i);
            }
        }
    } else {
        int output_port = -1;
        int index = findMacEntry(frame.dest_mac);
        if (index != -1) {
            output_port = macTable[index].port;
        }

        if (output_port != -1 && output_port < MAX_PORTS && output_port != input_port) {
            output_port_streams[output_port].write(frame);
            printf("Trama enviada a puerto %d\n", output_port);
        } else if (output_port != -1 && output_port >= MAX_PORTS) {
            printf("Error: Output port %d is out of valid range (0 to %d). Frame discarded.\n", output_port, MAX_PORTS - 1);
        } else {
            for (int i = 0; i < MAX_PORTS; i++) {
                if (i != input_port) {
                    output_port_streams[i].write(frame);
                    printf("Trama enviada a puerto %d por desconocimiento de MAC destino\n", i);
                }
            }
        }
    }
    printMacTable(); // Imprimir la tabla MAC después de procesar una trama
}




void layer2_switch(hls::stream<ethernet_frame> input_port_streams[MAX_PORTS], hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]) {
    #pragma HLS INTERFACE axis port=input_port_streams depth=MAX_FRAMES_PER_PORT
    #pragma HLS INTERFACE axis port=output_port_streams depth=MAX_FRAMES_PER_PORT
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS

    static ap_uint<4> current_port = 0;

    while (true) {
        bool all_ports_empty = true;

        for (int i = 0; i < MAX_PORTS; i++) {
            #pragma HLS UNROLL
            ap_uint<4> port = (current_port + i) % MAX_PORTS;

            if (!input_port_streams[port].empty()) {
                ethernet_frame frame;
                input_port_streams[port] >> frame;
                process_frame(frame, output_port_streams, port);
                current_port = (port + 1) % MAX_PORTS;
                all_ports_empty = false;
                break;
            }
        }

        if (all_ports_empty) {
            break;
        }
    }
}


