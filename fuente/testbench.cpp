#include "switch.hpp"
#include <iostream>
#include <vector>
#include <hls_stream.h>
#include <ap_int.h>

// Funciï¿½n para crear una trama especï¿½fica
ethernet_frame createFrame(ap_uint<48> dest_mac, ap_uint<48> src_mac, ap_uint<16> eth_type, ap_uint<16> crc) {
    ethernet_frame frame;
    frame.dest_mac = dest_mac;
    frame.src_mac = src_mac;
    frame.eth_type = eth_type;
    // Inicializar todos los datos a cero
    for (int i = 0; i < 480; i++) {
        frame.data[i] = 0;
    }
    frame.frame_crc = crc;
    return frame;
}



// Funciï¿½n de auto-verificaciï¿½n (self-checking)
void selfCheck(hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]) {
    // Esta funciï¿½n deberï¿½a implementar la lï¿½gica para verificar la correcciï¿½n de las tramas de salida
    // Implementa aquï¿½ tu lï¿½gica de verificaciï¿½n
    std::cout << "Iniciando verificaciï¿½n del comportamiento de salida del switch..." << std::endl;
    // Nota: esta parte estï¿½ simplificada y debe ser completada segï¿½n los requisitos especï¿½ficos del proyecto
}



// FunciÃ³n para imprimir el estado del buffer de cada puerto
void printPortBuffers(hls::stream<ethernet_frame> port_streams[MAX_PORTS], const char* description) {
    std::cout << "Estado de los buffers despuÃ©s de " << description << ":\n";

    for (int i = 0; i < MAX_PORTS; i++) {
        std::cout << "Puerto " << i << ":\n";

        // Duplicar el stream temporalmente para lectura
        hls::stream<ethernet_frame> temp_stream;
        ethernet_frame frame;

        int count = 0;
        while (!port_streams[i].empty()) {
            port_streams[i].read(frame);
            temp_stream.write(frame);  // Almacenar en un stream temporal

            // Formatear la salida para cada frame
            std::cout << "\tTrama " << ++count << ": Dest MAC = " << std::hex << frame.dest_mac.to_uint64() << ", Src MAC = " << frame.src_mac.to_uint64() << ", Ethertype = " << std::hex << frame.eth_type.to_uint() << std::dec << "\n";
        }

        // Reinsertar los datos en el stream original
        while (!temp_stream.empty()) {
            temp_stream.read(frame);
            port_streams[i].write(frame);
        }
    }
}

// FunciÃ³n para imprimir estados intermedios
void printIntermediateBufferState(hls::stream<ethernet_frame>& port_stream, int port_number) {
    std::cout << "Estado intermedio del buffer del puerto " << port_number << ":\n";
    hls::stream<ethernet_frame> temp_stream;
    ethernet_frame frame;
    while (!port_stream.empty()) {
        port_stream.read(frame);
        temp_stream.write(frame);  // Copia temporal para no consumir el stream
        std::cout << "\tDest MAC: " << std::hex << frame.dest_mac.to_uint64()
                  << ", Src MAC: " << frame.src_mac.to_uint64()
                  << ", Ethertype: " << std::hex << frame.eth_type.to_uint() << std::dec << "\n";
    }
    while (!temp_stream.empty()) {  // Volver a llenar el stream original
        temp_stream.read(frame);
        port_stream.write(frame);
    }
}



// Funciï¿½n principal
int main() {
    hls::stream<ethernet_frame> input_port_streams[MAX_PORTS];
    hls::stream<ethernet_frame> output_port_streams[MAX_PORTS];

    int tramas_enviadas[MAX_PORTS] = {0}; // Array para contar las tramas enviadas a cada puerto

    // Crear tramas especï¿½ficas
    std::vector<ethernet_frame> frames = {
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
    };

    for (int i = 0; i < frames.size(); ++i) {
        int input_port = (i < 5) ? 0 : 1;  // Las primeras 5 al puerto 0, las siguientes 6 al puerto 1
        input_port_streams[input_port].write(frames[i]);
        tramas_enviadas[input_port]++; // Incrementar contador para el puerto correspondiente

        std::cout << "Trama " << i + 1 << " enviada a puerto " << input_port << std::endl;

        // Llamar a printPortBuffers cada vez que se envÃ­an 2 tramas al mismo puerto
        if (tramas_enviadas[input_port] % 2 == 0) { // Cada vez que el contador para un puerto es mÃºltiplo de 2
            printPortBuffers(input_port_streams, "envÃ­o de 2 tramas al mismo puerto");
        }
    }

    // Procesar las tramas en el switch
    layer2_switch(input_port_streams, output_port_streams);

    std::cout << "--------------------------------------------------\n";

    // Ejecutar la verificaciï¿½n
    selfCheck(output_port_streams);

    std::cout << "--------------------------------------------------\n";

    // Opcional: Imprimir el estado final de los buffers
    printPortBuffers(input_port_streams, "procesar tramas");

    return 0;
}
