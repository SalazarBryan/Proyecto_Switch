#ifndef SWITCH_H
#define SWITCH_H

#include <hls_stream.h>
#include <ap_int.h>

#define MAX_PORTS 8
#define MAX_MAC_ENTRIES 16
#define MIN_FRAME_SIZE 64
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
