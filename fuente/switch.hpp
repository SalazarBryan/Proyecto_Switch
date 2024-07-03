#ifndef SWITCH_H
#define SWITCH_H

#include <hls_stream.h>
#include <ap_int.h>

//antes de leer una prueba se debe de ajustar los parámetros del switch.

#define MAX_PORTS 8
#define MAX_MAC_ENTRIES 11
#define MIN_FRAME_SIZE 64
#define MAX_FRAMES_PER_PORT 3

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

extern MacEntry macTable[MAX_MAC_ENTRIES];
extern int entryCount;
extern hls::stream<ethernet_frame> port_buffers[MAX_PORTS][MAX_FRAMES_PER_PORT];
extern int buffer_count[MAX_PORTS];

bool compareMAC(const ap_uint<48>& mac1, const ap_uint<48>& mac2);
int findMacEntry(const ap_uint<48>& mac);
void moveEntryToFront(int index);
void updateMACEntry(int index, int newPort);
void addNewMACEntry(const ap_uint<48>& mac, int port);
void update_mac_table(const ap_uint<48>& src_mac, ap_uint<4> input_port);
void ensureMinimumFrameSize(ethernet_frame& frame);
bool isEthertypeSupported(const ap_uint<16>& eth_type);
void process_frame(ethernet_frame& frame, hls::stream<ethernet_frame> output_port_streams[MAX_PORTS], ap_uint<4> input_port);
void layer2_switch(hls::stream<ethernet_frame> input_port_streams[MAX_PORTS], hls::stream<ethernet_frame> output_port_streams[MAX_PORTS]);

#endif // SWITCH_H




/*
TRAMA 1
Dest MAC: 0x000000111111
Src MAC: 0x0123456789AB
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 2
Dest MAC: 0x010101010101
Src MAC: 0x0123456789CD
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 3
Dest MAC: 0x111111000000
Src MAC: 0x0123456789EF
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 4
Dest MAC: 0x000000000000
Src MAC: 0x0123456789BA
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 5
Dest MAC: 0x111111111111
Src MAC: 0x0123456789DC
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 6
Dest MAC: 0x101010101010
Src MAC: 0x0123456789FE
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 7
Dest MAC: 0x000111000111
Src MAC: 0x012345678912
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 8
Dest MAC: 0x111000111000
Src MAC: 0x012345678934
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 9
Dest MAC: 0x000000000001
Src MAC: 0x012345678956
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 10
Dest MAC: 0x111111111110
Src MAC: 0x012345678978
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101

TRAMA 11
Dest MAC: 0x100000000000
Src MAC: 0x01234567899A
Ethertype: 0x0800 (IPv4)
Datos: Todos ceros (480 bites)
CRC: 0x0101
 */
