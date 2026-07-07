#pragma once

#include "Memory.hpp"

#include <array>
#include <cstdint>
#include <string>

class Cpu {
public:
    static constexpr std::size_t RegisterCount = 32;
    static constexpr std::size_t DefaultMemorySize = 64 * 1024;

    Cpu();

    void loadProgram(const std::string& path);

    void step();
    void run(uint64_t maxSteps);

    uint32_t pc() const;
    uint32_t reg(std::size_t index) const;

    const Memory& memory() const;
    Memory& memory();

    bool halted() const;

    void printState() const;
    void printRegisters() const;
    void printRegister(const std::string& name) const;
    void printMemory(uint32_t start, std::size_t count) const;

private:
    uint32_t pc_;
    std::array<uint32_t, RegisterCount> regs_;
    Memory memory_;
    bool halted_;

    void forceZeroRegister();
    static int registerIndexFromName(const std::string& name);
};
