// unit_test.cpp

#include "unit_test.hpp"

MacEntry macTable[100]; // ejemplo de tabla MAC
int entryCount = 0;

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
