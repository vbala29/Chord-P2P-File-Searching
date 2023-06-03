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


/** https://stackoverflow.com/questions/15653695/how-to-convert-ip-address-in-char-to-uint32-t-in-c
 * Convert human readable IPv4 address to UINT32
 * @param pDottedQuad   Input C string e.g. "192.168.0.1"
 * @param pIpAddr       Output IP address as UINT32
 * return 1 on success, else 0
 */
inline int ipStringToNumber (const char*       pDottedQuad,
                              unsigned int *    pIpAddr)
{
   unsigned int            byte3;
   unsigned int            byte2;
   unsigned int            byte1;
   unsigned int            byte0;
   char              dummyString[2];

   /* The dummy string with specifier %1s searches for a non-whitespace char
    * after the last number. If it is found, the result of sscanf will be 5
    * instead of 4, indicating an erroneous format of the ip-address.
    */
   if (sscanf (pDottedQuad, "%u.%u.%u.%u%1s",
                  &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
   {
      if (    (byte3 < 256)
           && (byte2 < 256)
           && (byte1 < 256)
           && (byte0 < 256)
         )
      {
         *pIpAddr  =   (byte3 << 24)
                     + (byte2 << 16)
                     + (byte1 << 8)
                     +  byte0;

         return 1;
      }
   }

   return 0;
}

#endif

