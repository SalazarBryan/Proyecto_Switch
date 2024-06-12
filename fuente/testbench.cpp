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
    // Inicializar todos los datos a cero
    for (int i = 0; i < 480; i++) {
        frame.data[i] = 0;
    }
    frame.frame_crc = crc;
    return frame;
}

// Función de auto-verificación (self-checking)
void selfCheck(hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]) {
    // Esta función debería implementar la lógica para verificar la corrección de las tramas de salida
    // Implementa aquí tu lógica de verificación
    std::cout << "Iniciando verificación del comportamiento de salida del switch..." << std::endl;
    // Nota: esta parte está simplificada y debe ser completada según los requisitos específicos del proyecto
}

// Función principal
int main() {
    hls::stream<ethernet_frame> input_port_streams[MAX_PORTS];
    hls::stream<ethernet_frame> output_port_streams[MAX_PORTS];

    // Crear tramas específicas
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

    // Enviar tramas a través de los puertos 0 y 1
    for (int i = 0; i < frames.size(); ++i) {
        int input_port = (i < 5) ? 0 : 1; // Las primeras 5 al puerto 0, las siguientes 6 al puerto 1
        input_port_streams[input_port].write(frames[i]);
        std::cout << "Trama " << i + 1 << " enviada a puerto " << input_port << std::endl;
    }

    // Procesar las tramas en el switch
    layer2_switch(input_port_streams, output_port_streams);

    // Ejecutar la verificación
    selfCheck(output_port_streams);

    return 0;
}
