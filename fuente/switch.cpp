#include "switch.hpp"
#include <cstdio>

MacEntry macTable[MAX_MAC_ENTRIES];
int entryCount = 0;
int buffer_count[MAX_PORTS] = {0};

bool compareMAC(const ap_uint<48>& mac1, const ap_uint<48>& mac2) {
    return mac1 == mac2;
}

int findMacEntry(const ap_uint<48>& mac) {
    for (int i = 0; i < entryCount; i++) {
        if (compareMAC(macTable[i].macAddress, mac)) {
            return i;
        }
    }
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
}

void update_mac_table(const ap_uint<48>& src_mac, ap_uint<4> input_port) {
    int index = findMacEntry(src_mac);
    if (index != -1) {
        updateMACEntry(index, input_port);
    } else {
        addNewMACEntry(src_mac, input_port);
    }
}

void ensureMinimumFrameSize(ethernet_frame& frame) {
    int actualLength = 0;
    while (actualLength < 480 && frame.data[actualLength] != 0) {
        actualLength++;
    }

    printf("Actual length before padding: %d bytes\n", actualLength);

    if (actualLength < MIN_FRAME_SIZE) {
        for (int i = actualLength; i < MIN_FRAME_SIZE; i++) {
            frame.data[i] = 0;
        }
        printf("Padding applied up to %d bytes\n", MIN_FRAME_SIZE);
    }

    printf("Data after padding: ");
    for (int i = 0; i < MIN_FRAME_SIZE; i++) {
        printf("%02x ", frame.data[i]);
    }
    printf("\n");
}

bool isEthertypeSupported(const ap_uint<16>& eth_type) {
    return eth_type == 0x0800 ||  // IPv4
           eth_type == 0x0806 ||  // ARP
           eth_type == 0x86DD ||  // IPv6
           eth_type == 0x8100;    // VLAN
}

void process_frame(ethernet_frame& frame, hls::stream<ethernet_frame> output_port_streams[MAX_PORTS], ap_uint<4> input_port) {
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
        // Es un broadcast, enviar a todos los puertos excepto al de entrada
        for (int i = 0; i < MAX_PORTS; i++) {
            if (i != input_port) {
                output_port_streams[i].write(frame);
            }
        }
    } else {
        // Buscar el puerto de salida en la tabla MAC
        int output_port = -1;
        int index = findMacEntry(frame.dest_mac);
        if (index != -1) {
            output_port = macTable[index].port;
        }

        // Si se encuentra un puerto válido y no es el de entrada, enviar allí la trama
        if (output_port != -1 && output_port < MAX_PORTS && output_port != input_port) {
            output_port_streams[output_port].write(frame);
        } else if (output_port != -1 && output_port >= MAX_PORTS) {
            // Si se encuentra un puerto pero no es válido, mostrar un mensaje de error
            printf("Error: Output port %d is out of valid range (0 to %d). Frame discarded.\n", output_port, MAX_PORTS - 1);
        } else {
            // Si no se encuentra en la tabla MAC, enviar a todos los puertos excepto al de entrada
            for (int i = 0; i < MAX_PORTS; i++) {
                if (i != input_port) {
                    output_port_streams[i].write(frame);
                }
            }
        }
    }
}


void layer2_switch(hls::stream<ethernet_frame> input_port_streams[MAX_PORTS], hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]) {
    #pragma HLS INTERFACE axis port=input_port_streams
    #pragma HLS INTERFACE axis port=output_port_streams
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS

    static ap_uint<4> current_port = 0;

    bool all_ports_empty = false;

    while (!all_ports_empty) {
        bool frame_processed = false;
        all_ports_empty = true;

        for (int i = 0; i < MAX_PORTS; i++) {
            #pragma HLS UNROLL
            ap_uint<4> port = (current_port + i) % MAX_PORTS;

            if (!input_port_streams[port].empty()) {
                ethernet_frame frame;
                input_port_streams[port] >> frame;
                process_frame(frame, output_port_streams, port);
                frame_processed = true;
                current_port = port + 1;
                all_ports_empty = false;
                break;
            }
        }

        if (!frame_processed) {
            current_port = (current_port + 1) % MAX_PORTS;
        }
    }
}

