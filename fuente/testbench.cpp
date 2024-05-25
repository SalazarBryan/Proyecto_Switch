#include "switch.hpp"
#include <iostream>

void test_layer2_switch() {
	#pragma HLS STREAM variable=B depth=100 type=fifo
    hls::stream<ethernet_frame> input_port_streams[MAX_PORTS];
    hls::stream<ethernet_frame> output_port_streams[MAX_PORTS];

    //trabajar el self checking

    ethernet_frame test_frame;
    test_frame.dest_mac = 0xFFFFFFFFFFFF;
    test_frame.src_mac = 0x001122334455;
    test_frame.eth_type = 0x0800;
    for (int i = 0; i < 480; i++) {
        test_frame.data[i] = 0xAB;
    }
    test_frame.frame_crc = 0x1234;
    input_port_streams[0].write(test_frame);
    layer2_switch(input_port_streams, output_port_streams);

    //2 hilos --- 1 donde correr layer2_switch y el otro donde corre el test

    for (int i = 0; i < MAX_PORTS; i++) {
    	if (i != 0 && !output_port_streams[i].empty()) {
    		ethernet_frame output_frame = output_port_streams[i].read();
            std::cout << "Trama recibida en el puerto " << i << std::endl;
            std::cout << "Dest MAC: " << std::hex << output_frame.dest_mac.to_uint64() << std::endl;
            std::cout << "Src MAC: " << std::hex << output_frame.src_mac.to_uint64() << std::endl;
            std::cout << "Ethertype: " << std::hex << output_frame.eth_type.to_uint() << std::endl;
            std::cout << "CRC: " << std::hex << output_frame.frame_crc.to_uint() << std::endl;
            std::cout << "Datos: ";
            for (int j = 0; j < MIN_FRAME_SIZE; j++) {
                std::cout << std::hex << (int)output_frame.data[j] << " ";
            }
            	std::cout << std::endl;
        }
    }
}

int main() {
    test_layer2_switch();
    return 0;
}
