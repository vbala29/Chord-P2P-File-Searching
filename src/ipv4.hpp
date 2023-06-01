/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */


#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>
#include <string>


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

        inline std::string Ipv4ToString() const {
            return std::to_string(byte1) + "." + std::to_string(byte2) + "." + std::to_string(byte3) + "." + std::to_string(byte4); 
        }

        //For maps used in penn-chord
        bool operator<(const Ipv4Address& other) const {
            if (this->byte1 < other.byte1) {
                return true;
            } else if (this->byte1 == other.byte1) {
                if (this->byte2 < other.byte2) {
                    return true;
                } else if (this->byte2 == other.byte2) {
                    if (this->byte3 < other.byte3) {
                        return true;
                    } else if (this->byte3 == other.byte3) {
                        if (this->byte4 < other.byte4) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }


    private:
        uint8_t byte1, byte2, byte3, byte4;

};

/**
 * @brief Operator overload for Ipv4Address
 * 
 * @param os 
 * @param ip 
 * @return std::ostream& 
 */
 inline std::ostream& operator<< (std::ostream& os, const Ipv4Address& ip) {
            std::string s = ip.Ipv4ToString();
            os << s;
            return os;
}

#endif