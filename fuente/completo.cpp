#include <hls_stream.h>
#include <ap_int.h>
#include <cstdio>

#define MAX_PORTS 8
#define MAX_MAC_ENTRIES 16
#define MIN_FRAME_SIZE 64  // Tamaño mínimo de la trama Ethernet
#define MAX_FRAMES_PER_PORT 8

struct ethernet_frame {
    ap_uint<48> dest_mac;
    ap_uint<48> src_mac;
    ap_uint<16> eth_type;
    ap_uint<8> data[480];
    ap_uint<16> frame_crc;
};

struct MacEntry {
    ap_uint<48> macAddress;
    ap_uint<4> port;
};


MacEntry macTable[MAX_MAC_ENTRIES];
int entryCount = 0;


// Define buffers para cada puerto
hls::stream<ethernet_frame> port_buffers[MAX_PORTS][MAX_FRAMES_PER_PORT];
int buffer_count[MAX_PORTS] = {0};  // Contador para cada buffer


// Función para comparar dos direcciones MAC
bool compareMAC(const ap_uint<48>& mac1, const ap_uint<48>& mac2) {
    return mac1 == mac2;
}

// Buscar una entrada en la tabla de direcciones MAC
int findMacEntry(const ap_uint<48>& mac) {
    for (int i = 0; i < entryCount; i++) {
        if (compareMAC(macTable[i].macAddress, mac)) {
            return i;
        }
    }
    return -1;
}

// Mover una entrada al frente de la tabla (LRU)
void moveEntryToFront(int index) {
    MacEntry temp = macTable[index];
    for (int i = index; i > 0; i--) {
        macTable[i] = macTable[i - 1];
    }
    macTable[0] = temp;
}

// Actualizar una entrada existente en la tabla de MAC
void updateMACEntry(int index, int newPort) {
    macTable[index].port = newPort;
    moveEntryToFront(index);
}

// Añadir una nueva entrada a la tabla de MAC
void addNewMACEntry(const ap_uint<48>& mac, int port) {
    if (entryCount == MAX_MAC_ENTRIES) {
        moveEntryToFront(MAX_MAC_ENTRIES - 1); // Reemplazar usando LRU
    } else {
        moveEntryToFront(entryCount);
        entryCount++;
    }
    macTable[0].macAddress = mac;
    macTable[0].port = port;
}

// Función para actualizar o añadir direcciones MAC en la tabla
void update_mac_table(const ap_uint<48>& src_mac, ap_uint<4> input_port) {
    int index = findMacEntry(src_mac);
    if (index != -1) {
        updateMACEntry(index, input_port);
    } else {
        addNewMACEntry(src_mac, input_port);
    }
}


// Función para asegurar el tamaño mínimo de la trama (padding)
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


// Verifica si el ethertype de una trama es soportado
bool isEthertypeSupported(const ap_uint<16>& eth_type) {
    return eth_type == 0x0800 || // IPv4
           eth_type == 0x0806 || // ARP
           eth_type == 0x86DD || // IPv6
           eth_type == 0x8100;   // VLAN
}

// Función para procesar una trama y decidir a qué puerto(s) reenviarla
void process_frame(hls::stream<ethernet_frame>& input_frame, hls::stream<ethernet_frame> output_port_streams[MAX_PORTS], ap_uint<4> input_port) {
    // Verificar si el puerto de recepción es válido
    if (input_port < 0 || input_port >= MAX_PORTS) {
        printf("Error: Receiving port %u is out of valid range (0 to %d).\n", input_port.to_uint(), MAX_PORTS - 1);
        return; // Salir de la función si el puerto no es válido
    }

    ethernet_frame frame = input_frame.read();

    // Verificar si el ethertype es soportado antes de procesar más
    if (!isEthertypeSupported(frame.eth_type)) {
        printf("Ethertype 0x%X not supported. Frame discarded.\n", frame.eth_type.to_uint());
        return; // Descartar la trama si el ethertype no es soportado
    }
    
    update_mac_table(frame.src_mac, input_port);
    ensureMinimumFrameSize(frame); // Asegurar el tamaño mínimo antes de reenviar
    
    bool is_broadcast = (frame.dest_mac == 0xFFFFFFFFFFFF);
    if (is_broadcast) {
        for (int i = 0; i < MAX_PORTS; i++) {
            if (i != input_port) {
                output_port_streams[i].write(frame);
            }
        }
    } else {
        int output_port = -1;
        int index = findMacEntry(frame.dest_mac);
        if (index != -1) {
            output_port = macTable[index].port;
        }

        if (output_port != -1 && output_port != input_port) {
            output_port_streams[output_port].write(frame);
        } else {
            // Si no se encuentra la MAC, realizar broadcast
            for (int i = 0; i < MAX_PORTS; i++) {
                if (i != input_port) {
                    output_port_streams[i].write(frame);
                }
            }
        }
    }
}

// Función principal que gestiona la lógica del switch de capa 2
void layer2_switch(
    hls::stream<ethernet_frame> input_port_streams[MAX_PORTS],
    hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]
) {
    #pragma HLS INTERFACE axis port=input_port_streams
    #pragma HLS INTERFACE axis port=output_port_streams
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS

    while (true)
    {
       
    
    // Leer de los puertos y almacenar en buffers si hay espacio
    for (int i = 0; i < MAX_PORTS; i++) {
        #pragma HLS UNROLL
        if (!input_port_streams[i].empty() && buffer_count[i] < MAX_FRAMES_PER_PORT) {
            ethernet_frame frame;
            input_port_streams[i] >> frame;  // Leer trama directamente
            port_buffers[i][buffer_count[i]].write(frame);  // Escribir trama en el buffer
            buffer_count[i]++;
        }
    }

    //revisar que se está implementando doble vez 

    // Arbitraje y procesamiento de tramas desde buffers
    for (int i = 0; i < MAX_PORTS; i++) {
        #pragma HLS UNROLL
        if (buffer_count[i] > 0) {
            ethernet_frame frame;
            port_buffers[i][0] >> frame;  // Leer la primera trama del buffer
            process_frame(port_buffers[i][0], output_port_streams, i);


    //revisar porque no se necesita
            // Rotar las tramas en el buffer
            for (int j = 0; j < buffer_count[i] - 1; j++) {
                ethernet_frame temp_frame;
                port_buffers[i][j + 1] >> temp_frame;  // Leer siguiente trama
                port_buffers[i][j].write(temp_frame);  // Escribir en la posición actual
            }
            buffer_count[i]--;
        }
    }
}
  }



por que está input_port y input_port_streams
