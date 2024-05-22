// unit_test.hpp

#ifndef MAC_H
#define MAC_H

#include <ap_int.h>

// Estructura de entrada MAC
struct MacEntry {
    ap_uint<48> macAddress;
    int port;
};

// Declaración de funciones
bool compareMAC(const ap_uint<48>& mac1, const ap_uint<48>& mac2);
int findMacEntry(const ap_uint<48>& mac);
void moveEntryToFront(int index);
void updateMACEntry(int index, int newPort);

// Variables globales (para simplicidad en este ejemplo)
extern MacEntry macTable[];
extern int entryCount;

#endif // MAC_H
