#include "../include/BufferV2.h"

BufferV2::BufferV2() : size(0) {
    m_buffer = new uint8_t[BUFFER_SIZE];
}   

BufferV2::BufferV2(BufferV2 &buffer) {
    m_buffer = new uint8_t[BUFFER_SIZE];
    memcpy(m_buffer, buffer.m_buffer, buffer.size);
    size = buffer.size;
}

BufferV2::~BufferV2() {
    delete[] m_buffer;
}

void BufferV2::Write(const uint8_t* buffer, uint32_t size) {
    if (size + this->size > BUFFER_SIZE) {
        perror("Buffer overflow");
        return;
    }

    memcpy(m_buffer + this->size, buffer, size);
    this->size += size;
}

void BufferV2::WriteU8(uint8_t data) {
    Write(&data, sizeof(uint8_t));
}

void BufferV2::WriteU16(uint16_t data) {
    Write((uint8_t*) &data, sizeof(uint16_t));
}

void BufferV2::WriteHtonU32(uint32_t data) {
    uint32_t data_n = htonl(data);
    Write((uint8_t*) &data_n, sizeof(uint32_t));
}

void BufferV2::Read(uint8_t* buffer, uint32_t size) {
    if (size > this->size) {
        perror("Buffer underflow");
        return;
    }

    memcpy(buffer, m_buffer, size);
    this->size -= size;
    memcpy(m_buffer, m_buffer + size, this->size);
}

 uint8_t BufferV2::ReadU8() {
    uint8_t data;
    Read(&data, sizeof(uint8_t));
    return data;
}

 uint16_t BufferV2::ReadU16() {
    uint16_t data;
    Read((uint8_t*) &data, sizeof(uint16_t));
    return data;
}

 uint32_t BufferV2::ReadNtohU32() {
    uint32_t data;
    Read((uint8_t*) &data, sizeof(uint32_t));
    return ntohl(data);
}

size_t BufferV2::GetSerializedSize() {
    return size;
}

uint32_t BufferV2::Serialize (uint8_t* buffer, uint32_t maxSize) const {
    if (maxSize < size) {
        perror("Buffer not large enough");
        return 0;
    }

    memcpy(buffer, m_buffer, size);
    return size;
}