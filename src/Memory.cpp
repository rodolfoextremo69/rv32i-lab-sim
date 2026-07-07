#include "Memory.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

Memory::Memory(std::size_t sizeBytes)
    : data_(sizeBytes, 0) {}

std::size_t Memory::size() const {
    return data_.size();
}

void Memory::checkRange(uint32_t address, std::size_t bytes) const {
    if (static_cast<std::size_t>(address) + bytes > data_.size()) {
        throw std::out_of_range("Memory access out of range");
    }
}

void Memory::loadBinary(const std::string& path, uint32_t baseAddress) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Could not open binary file: " + path);
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 0) {
        throw std::runtime_error("Could not read file size");
    }

    checkRange(baseAddress, static_cast<std::size_t>(fileSize));

    file.read(reinterpret_cast<char*>(&data_[baseAddress]), fileSize);

    if (!file) {
        throw std::runtime_error("Could not read complete binary file");
    }

    std::cout << "Loaded " << fileSize
              << " bytes at address 0x00000000\n";
}

uint8_t Memory::read8(uint32_t address) const {
    checkRange(address, 1);
    return data_[address];
}

uint16_t Memory::read16(uint32_t address) const {
    checkRange(address, 2);

    return static_cast<uint16_t>(data_[address])
         | static_cast<uint16_t>(data_[address + 1] << 8);
}

uint32_t Memory::read32(uint32_t address) const {
    checkRange(address, 4);

    return static_cast<uint32_t>(data_[address])
         | (static_cast<uint32_t>(data_[address + 1]) << 8)
         | (static_cast<uint32_t>(data_[address + 2]) << 16)
         | (static_cast<uint32_t>(data_[address + 3]) << 24);
}

void Memory::write8(uint32_t address, uint8_t value) {
    checkRange(address, 1);
    data_[address] = value;
}

void Memory::write16(uint32_t address, uint16_t value) {
    checkRange(address, 2);

    data_[address] = static_cast<uint8_t>(value & 0xFF);
    data_[address + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void Memory::write32(uint32_t address, uint32_t value) {
    checkRange(address, 4);

    data_[address] = static_cast<uint8_t>(value & 0xFF);
    data_[address + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    data_[address + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    data_[address + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}
