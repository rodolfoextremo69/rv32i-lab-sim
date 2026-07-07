#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

class Memory {
public:
    explicit Memory(std::size_t sizeBytes);

    void loadBinary(const std::string& path, uint32_t baseAddress = 0);

    uint8_t read8(uint32_t address) const;
    uint16_t read16(uint32_t address) const;
    uint32_t read32(uint32_t address) const;

    void write8(uint32_t address, uint8_t value);
    void write16(uint32_t address, uint16_t value);
    void write32(uint32_t address, uint32_t value);

    std::size_t size() const;

private:
    std::vector<uint8_t> data_;

    void checkRange(uint32_t address, std::size_t bytes) const;
};
