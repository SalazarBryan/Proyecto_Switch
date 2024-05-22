// test_mac.cpp

#include <iostream>
#include "unit_test.hpp"

void runTests() {
    // Configuración inicial de pruebas
    entryCount = 3;
    macTable[0] = {0x001122334455, 1};
    macTable[1] = {0x66778899AABB, 2};
    macTable[2] = {0xAABBCCDDEEFF, 3};

    // Prueba de compareMAC
    std::cout << "Testing compareMAC..." << std::endl;
    std::cout << (compareMAC(macTable[0].macAddress, 0x001122334455) ? "Passed" : "Failed") << std::endl;
    std::cout << (!compareMAC(macTable[0].macAddress, 0x66778899AABB) ? "Passed" : "Failed") << std::endl;

    // Prueba de findMacEntry
    std::cout << "Testing findMacEntry..." << std::endl;
    std::cout << (findMacEntry(0x001122334455) == 0 ? "Passed" : "Failed") << std::endl;
    std::cout << (findMacEntry(0x66778899AABB) == 1 ? "Passed" : "Failed") << std::endl;
    std::cout << (findMacEntry(0xAABBCCDDEEFF) == 2 ? "Passed" : "Failed") << std::endl;
    std::cout << (findMacEntry(0x112233445566) == -1 ? "Passed" : "Failed") << std::endl;

    // Prueba de moveEntryToFront
    std::cout << "Testing moveEntryToFront..." << std::endl;
    moveEntryToFront(2);
    std::cout << (macTable[0].macAddress == 0xAABBCCDDEEFF ? "Passed" : "Failed") << std::endl;
    std::cout << (macTable[1].macAddress == 0x001122334455 ? "Passed" : "Failed") << std::endl;
    std::cout << (macTable[2].macAddress == 0x66778899AABB ? "Passed" : "Failed") << std::endl;

    // Prueba de updateMACEntry
    std::cout << "Testing updateMACEntry..." << std::endl;
    updateMACEntry(1, 5);
    std::cout << (macTable[0].macAddress == 0x66778899AABB ? "Passed" : "Failed") << std::endl;
    std::cout << (macTable[0].port == 5 ? "Passed" : "Failed") << std::endl;
    std::cout << (macTable[1].macAddress == 0x001122334455 ? "Passed" : "Failed") << std::endl;
}

int main() {
    runTests();
    return 0;
}
