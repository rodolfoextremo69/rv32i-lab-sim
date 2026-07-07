#include "Cpu.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>

Cpu::Cpu()
    : pc_(0),
      regs_{},
      memory_(DefaultMemorySize),
      halted_(false) {
    regs_.fill(0);
}

void Cpu::loadProgram(const std::string& path) {
    memory_.loadBinary(path, 0);
    pc_ = 0;
    halted_ = false;
    regs_.fill(0);
}

uint32_t Cpu::pc() const {
    return pc_;
}

uint32_t Cpu::reg(std::size_t index) const {
    if (index >= RegisterCount) {
        throw std::out_of_range("Invalid register index");
    }

    return regs_[index];
}

const Memory& Cpu::memory() const {
    return memory_;
}

Memory& Cpu::memory() {
    return memory_;
}

bool Cpu::halted() const {
    return halted_;
}

void Cpu::forceZeroRegister() {
    regs_[0] = 0;
}

void Cpu::step() {
    if (halted_) {
        std::cout << "CPU is halted.\n";
        return;
    }

    uint32_t instruction = memory_.read32(pc_);

    std::cout << "PC 0x"
              << std::hex << std::setw(8) << std::setfill('0') << pc_
              << " -> instruction 0x"
              << std::setw(8) << instruction
              << std::dec << std::setfill(' ') << "\n";

    pc_ += 4;

    forceZeroRegister();
}

void Cpu::run(uint64_t maxSteps) {
    for (uint64_t i = 0; i < maxSteps && !halted_; ++i) {
        step();
    }
}

void Cpu::printState() const {
    std::cout << "PC = 0x"
              << std::hex << std::setw(8) << std::setfill('0') << pc_
              << std::dec << std::setfill(' ') << "\n";

    std::cout << "Status = "
              << (halted_ ? "halted" : "running")
              << "\n";
}

void Cpu::printRegisters() const {
    for (std::size_t i = 0; i < RegisterCount; ++i) {
        std::cout << "x" << std::dec << i
                  << " = 0x"
                  << std::hex << std::setw(8) << std::setfill('0')
                  << regs_[i]
                  << std::dec << std::setfill(' ')
                  << "\n";
    }
}

int Cpu::registerIndexFromName(const std::string& name) {
    if (name.size() >= 2 && name[0] == 'x') {
        int index = std::stoi(name.substr(1));
        if (index >= 0 && index < 32) {
            return index;
        }
    }

    if (name == "zero") return 0;
    if (name == "ra") return 1;
    if (name == "sp") return 2;
    if (name == "gp") return 3;
    if (name == "tp") return 4;

    if (name == "t0") return 5;
    if (name == "t1") return 6;
    if (name == "t2") return 7;

    if (name == "s0" || name == "fp") return 8;
    if (name == "s1") return 9;

    if (name == "a0") return 10;
    if (name == "a1") return 11;
    if (name == "a2") return 12;
    if (name == "a3") return 13;
    if (name == "a4") return 14;
    if (name == "a5") return 15;
    if (name == "a6") return 16;
    if (name == "a7") return 17;

    if (name == "s2") return 18;
    if (name == "s3") return 19;
    if (name == "s4") return 20;
    if (name == "s5") return 21;
    if (name == "s6") return 22;
    if (name == "s7") return 23;
    if (name == "s8") return 24;
    if (name == "s9") return 25;
    if (name == "s10") return 26;
    if (name == "s11") return 27;

    if (name == "t3") return 28;
    if (name == "t4") return 29;
    if (name == "t5") return 30;
    if (name == "t6") return 31;

    return -1;
}

void Cpu::printRegister(const std::string& name) const {
    int index = registerIndexFromName(name);

    if (index < 0) {
        std::cout << "Unknown register: " << name << "\n";
        return;
    }

    std::cout << name << "/x" << index
              << " = 0x"
              << std::hex << std::setw(8) << std::setfill('0')
              << regs_[index]
              << std::dec << std::setfill(' ')
              << "\n";
}

void Cpu::printMemory(uint32_t start, std::size_t count) const {
    for (std::size_t i = 0; i < count; ++i) {
        if (i % 16 == 0) {
            std::cout << "\n0x"
                      << std::hex << std::setw(8) << std::setfill('0')
                      << (start + static_cast<uint32_t>(i))
                      << ": ";
        }

        uint8_t value = memory_.read8(start + static_cast<uint32_t>(i));

        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(value)
                  << " ";
    }

    std::cout << std::dec << std::setfill(' ') << "\n";
}
