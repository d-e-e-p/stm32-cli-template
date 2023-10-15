#include <iostream>
#include <vector>

uint8_t calculateCRC8(const std::vector<uint8_t>& data) {
    uint8_t crc = 0;

    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

int test() {
    std::vector<uint8_t> data = {0x01, 0x23, 0x45, 0x67};
    uint8_t crc = calculateCRC8(data);

    std::cout << "CRC-8-ATM: 0x" << std::hex << static_cast<int>(crc) << std::endl;

    return 0;
}
