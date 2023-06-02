#ifndef BUFFERV2_HPP
#define BUFFERV2_HPP

#define BUFFER_SIZE 1024

#include <stdint.h>
#include <cstring>
#include <stdio.h>
#include <arpa/inet.h>

class BufferV2 {
    public:
        BufferV2();
        BufferV2 (BufferV2 &buffer);
        virtual ~BufferV2();

        void Write(const uint8_t* buffer, uint32_t size);
        void WriteU8(uint8_t data);
        void WriteU16(uint16_t data);
        void WriteHtonU32(uint32_t data);
        
        void Read(uint8_t* buffer, uint32_t size);
        uint8_t ReadU8();
        uint16_t ReadU16();
        uint32_t ReadNtohU32();

        size_t GetSerializedSize();
        uint32_t Serialize (uint8_t* buffer, uint32_t maxSize) const;
        
    
    private:
        uint8_t* m_buffer;
        uint32_t size;
};

#endif