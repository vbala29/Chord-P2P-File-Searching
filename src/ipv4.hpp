/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#pragma once
#include <stdint.h>


class Ipv4Address {
    public:
        Ipv4Address() : Ipv4Address(0) {}

        Ipv4Address(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
            this->byte1 = byte1;
            this->byte2 = byte2;
            this->byte3 = byte3;
            this->byte4 = byte4;
        }

        Ipv4Address(uint32_t address) {
            this->byte1 = (address >> 24) & 0xFF;
            this->byte2 = (address >> 16) & 0xFF;
            this->byte3 = (address >> 8) & 0xFF;
            this->byte4 = address & 0xFF;
        }

        static Ipv4Address GetAny() {
            return Ipv4Address(0);
        }

        std::string Ipv4ToString() {
            return std::to_string(byte1) + "." + std::to_string(byte2) + "." + std::to_string(byte3) + "." + std::to_string(byte4); 
        }

    private:
        uint8_t byte1, byte2, byte3, byte4;

};