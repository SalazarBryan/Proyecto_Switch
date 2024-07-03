#include "switch.hpp"
#include <iostream>
#include <vector>
#include <hls_stream.h>
#include <ap_int.h>

// Función para crear una trama específica
ethernet_frame createFrame(ap_uint<48> dest_mac, ap_uint<48> src_mac, ap_uint<16> eth_type, ap_uint<16> crc) {
    ethernet_frame frame;
    frame.dest_mac = dest_mac;
    frame.src_mac = src_mac;
    frame.eth_type = eth_type;
    for (int i = 0; i < 480; i++) {
        frame.data[i] = 0;
    }
    frame.frame_crc = crc;
    return frame;
}

// Función de auto-verificación (self-checking)
void selfCheck(hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]) {
    std::cout << "Iniciando verificación del comportamiento de salida del switch..." << std::endl;

    // Verificar que las tramas se envían a los puertos correctos
    for (int i = 0; i < MAX_PORTS; i++) {
        hls::stream<ethernet_frame> temp_stream;
        ethernet_frame frame;
        while (!output_port_streams[i].empty()) {
            output_port_streams[i].read(frame);
            temp_stream.write(frame);
            std::cout << "Puerto " << i << ": Dest MAC = " << std::hex << frame.dest_mac.to_uint64() 
                      << ", Src MAC = " << frame.src_mac.to_uint64() 
                      << ", Ethertype = " << std::hex << frame.eth_type.to_uint() 
                      << ", CRC = " << frame.frame_crc.to_uint() << std::dec << std::endl;
        }
        while (!temp_stream.empty()) {
            temp_stream.read(frame);
            output_port_streams[i].write(frame);
        }
    }
}

// Función para imprimir el estado del buffer de cada puerto
void printPortBuffers(hls::stream<ethernet_frame> port_streams[MAX_PORTS], const char* description) {
    std::cout << "Estado de los buffers después de " << description << ":\n";

    for (int i = 0; i < MAX_PORTS; i++) {
        std::cout << "Puerto " << i << ":\n";

        hls::stream<ethernet_frame> temp_stream;
        ethernet_frame frame;
        int count = 0;
        while (!port_streams[i].empty()) {
            port_streams[i].read(frame);
            temp_stream.write(frame);
            std::cout << "\tTrama " << ++count << ": Dest MAC = " << std::hex << frame.dest_mac.to_uint64() 
                      << ", Src MAC = " << frame.src_mac.to_uint64() 
                      << ", Ethertype = " << std::hex << frame.eth_type.to_uint() 
                      << ", CRC = " << frame.frame_crc.to_uint() << std::dec << std::endl;
        }
        while (!temp_stream.empty()) {
            temp_stream.read(frame);
            port_streams[i].write(frame);
        }
    }
}

// Función principal
int main() {
    hls::stream<ethernet_frame> input_port_streams[MAX_PORTS];
    hls::stream<ethernet_frame> output_port_streams[MAX_PORTS];

    int tramas_enviadas[MAX_PORTS] = {0}; // Array para contar las tramas enviadas a cada puerto

    // Crear tramas específicas

    /* std::vector<ethernet_frame> frames = {
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x010101010101, 0x0123456789CD, 0x0800, 0x0101),
        createFrame(0x111111000000, 0x0123456789EF, 0x0800, 0x0101),
        createFrame(0x000000000000, 0x0123456789BA, 0x0800, 0x0101),
        createFrame(0x111111111111, 0x0123456789DC, 0x0800, 0x0101),
        createFrame(0x101010101010, 0x0123456789FE, 0x0800, 0x0101),
        createFrame(0x000111000111, 0x012345678912, 0x0800, 0x0101),
        createFrame(0x111000111000, 0x012345678934, 0x0800, 0x0101),
        createFrame(0x000000000001, 0x012345678956, 0x0800, 0x0101),
        createFrame(0x111111111110, 0x012345678978, 0x0800, 0x0101),
        createFrame(0x100000000000, 0x01234567899A, 0x0800, 0x0101)
    }; */

    std::vector<ethernet_frame> frames = {
        createFrame(0x000000111111, 0x0123456789AB, 0x1234, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x1234, 0x0101),
        createFrame(0x0123456789AB, 0x011111111111, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x000000111111, 0x0123456789AB, 0x0800, 0x0101),
        createFrame(0x100000000000, 0x01234567899A, 0x0800, 0x0101)
    };

    for (int i = 0; i < frames.size(); ++i) {
        int input_port = (i < 5) ? 0 : 1;  // Las primeras 5 al puerto 0, las siguientes 6 al puerto 1
        input_port_streams[input_port].write(frames[i]);
        tramas_enviadas[input_port]++; // Incrementar contador para el puerto correspondiente

        std::cout << "Trama " << i + 1 << " enviada a puerto " << input_port << std::endl;

        // Llamar a printPortBuffers cada vez que se envían 2 tramas al mismo puerto
        if (tramas_enviadas[input_port] % 2 == 0) { // Cada vez que el contador para un puerto es múltiplo de 2
            printPortBuffers(input_port_streams, "envío de 2 tramas al mismo puerto");
        }
    }

    // Procesar las tramas en el switch
    while (true) {
        bool all_ports_empty = true;
        for (int i = 0; i < MAX_PORTS; i++) {
            if (!input_port_streams[i].empty()) {
                all_ports_empty = false;
                break;
            }
        }
        if (all_ports_empty) {
            break;
        }
        layer2_switch(input_port_streams, output_port_streams);
    }

    std::cout << "--------------------------------------------------\n";

    // Ejecutar la verificación
    selfCheck(output_port_streams);

    std::cout << "--------------------------------------------------\n";

    // Opcional: Imprimir el estado final de los buffers
    printPortBuffers(output_port_streams, "procesar tramas");

    return 0;
}
