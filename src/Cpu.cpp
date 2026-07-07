#include "Cpu.hpp"

#include "Decode.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    uint32_t arithmeticShiftRight(uint32_t value, uint32_t shamt) {
        shamt &= 0x1F;

        if (shamt == 0) {
            return value;
        }

        if ((value & 0x80000000u) == 0) {
            return value >> shamt;
        }

        return (value >> shamt) | (~0u << (32 - shamt));
    }

    [[noreturn]] void invalidInstruction(uint32_t pc, uint32_t raw, const std::string& reason) {
        std::ostringstream out;
        out << "Invalid instruction at PC 0x"
            << std::hex << std::setw(8) << std::setfill('0') << pc
            << ", raw = 0x"
            << std::setw(8) << raw
            << ". " << reason;
        throw std::runtime_error(out.str());
    }
}

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
    executeOne(true);
}

void Cpu::run(uint64_t maxSteps) {
    uint64_t executed = 0;

    for (; executed < maxSteps && !halted_; ++executed) {
        uint32_t oldPc = pc_;
        executeOne(false);

        if (!halted_ && pc_ == oldPc) {
            std::cout << "Stopped: PC did not change. Possible infinite self-loop.\n";
            break;
        }
    }

    std::cout << "Run finished. Executed " << executed
              << " instruction(s). PC = 0x"
              << std::hex << std::setw(8) << std::setfill('0') << pc_
              << std::dec << std::setfill(' ') << "\n";
}

void Cpu::executeOne(bool trace) {
    if (halted_) {
        std::cout << "CPU is halted.\n";
        return;
    }

    try {
        uint32_t oldPc = pc_;
        uint32_t raw = memory_.read32(oldPc);
        DecodedInstruction d = decodeInstruction(raw);

        uint32_t nextPc = oldPc + 4;

        auto readReg = [&](uint32_t index) -> uint32_t {
            return regs_[index];
        };

        auto writeReg = [&](uint32_t index, uint32_t value) {
            if (index != 0) {
                regs_[index] = value;
            }
        };

        uint32_t rs1u = readReg(d.rs1);
        uint32_t rs2u = readReg(d.rs2);
        int32_t rs1s = static_cast<int32_t>(rs1u);
        int32_t rs2s = static_cast<int32_t>(rs2u);

        switch (d.opcode) {
            case 0x03: { // LOAD
                uint32_t address = rs1u + static_cast<uint32_t>(d.immI);

                switch (d.funct3) {
                    case 0x0: writeReg(d.rd, static_cast<uint32_t>(signExtend(memory_.read8(address), 8))); break;  // lb
                    case 0x1: writeReg(d.rd, static_cast<uint32_t>(signExtend(memory_.read16(address), 16))); break; // lh
                    case 0x2: writeReg(d.rd, memory_.read32(address)); break;                                       // lw
                    case 0x4: writeReg(d.rd, memory_.read8(address)); break;                                        // lbu
                    case 0x5: writeReg(d.rd, memory_.read16(address)); break;                                       // lhu
                    default: invalidInstruction(oldPc, raw, "Unknown LOAD funct3");
                }

                break;
            }

            case 0x13: { // OP-IMM
                uint32_t shamt = (raw >> 20) & 0x1F;

                switch (d.funct3) {
                    case 0x0: writeReg(d.rd, rs1u + static_cast<uint32_t>(d.immI)); break;            // addi
                    case 0x2: writeReg(d.rd, rs1s < d.immI ? 1u : 0u); break;                         // slti
                    case 0x3: writeReg(d.rd, rs1u < static_cast<uint32_t>(d.immI) ? 1u : 0u); break;  // sltiu
                    case 0x4: writeReg(d.rd, rs1u ^ static_cast<uint32_t>(d.immI)); break;            // xori
                    case 0x6: writeReg(d.rd, rs1u | static_cast<uint32_t>(d.immI)); break;            // ori
                    case 0x7: writeReg(d.rd, rs1u & static_cast<uint32_t>(d.immI)); break;            // andi

                    case 0x1: { // slli
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid SLLI funct7");
                        }

                        writeReg(d.rd, rs1u << shamt);
                        break;
                    }

                    case 0x5: { // srli / srai
                        if (d.funct7 == 0x00) {
                            writeReg(d.rd, rs1u >> shamt);
                        } else if (d.funct7 == 0x20) {
                            writeReg(d.rd, arithmeticShiftRight(rs1u, shamt));
                        } else {
                            invalidInstruction(oldPc, raw, "Invalid shift-immediate funct7");
                        }

                        break;
                    }

                    default:
                        invalidInstruction(oldPc, raw, "Unknown OP-IMM funct3");
                }

                break;
            }

            case 0x17: { // auipc
                writeReg(d.rd, oldPc + static_cast<uint32_t>(d.immU));
                break;
            }

            case 0x23: { // STORE
                uint32_t address = rs1u + static_cast<uint32_t>(d.immS);

                switch (d.funct3) {
                    case 0x0: memory_.write8(address, static_cast<uint8_t>(rs2u & 0xFF)); break;       // sb
                    case 0x1: memory_.write16(address, static_cast<uint16_t>(rs2u & 0xFFFF)); break;  // sh
                    case 0x2: memory_.write32(address, rs2u); break;                                  // sw
                    default: invalidInstruction(oldPc, raw, "Unknown STORE funct3");
                }

                break;
            }

            case 0x33: { // OP
                uint32_t shamt = rs2u & 0x1F;

                switch (d.funct3) {
                    case 0x0:
                        if (d.funct7 == 0x00) {
                            writeReg(d.rd, rs1u + rs2u); // add
                        } else if (d.funct7 == 0x20) {
                            writeReg(d.rd, rs1u - rs2u); // sub
                        } else {
                            invalidInstruction(oldPc, raw, "Invalid ADD/SUB funct7");
                        }
                        break;

                    case 0x1:
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid SLL funct7");
                        }
                        writeReg(d.rd, rs1u << shamt); // sll
                        break;

                    case 0x2:
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid SLT funct7");
                        }
                        writeReg(d.rd, rs1s < rs2s ? 1u : 0u); // slt
                        break;

                    case 0x3:
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid SLTU funct7");
                        }
                        writeReg(d.rd, rs1u < rs2u ? 1u : 0u); // sltu
                        break;

                    case 0x4:
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid XOR funct7");
                        }
                        writeReg(d.rd, rs1u ^ rs2u); // xor
                        break;

                    case 0x5:
                        if (d.funct7 == 0x00) {
                            writeReg(d.rd, rs1u >> shamt); // srl
                        } else if (d.funct7 == 0x20) {
                            writeReg(d.rd, arithmeticShiftRight(rs1u, shamt)); // sra
                        } else {
                            invalidInstruction(oldPc, raw, "Invalid SRL/SRA funct7");
                        }
                        break;

                    case 0x6:
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid OR funct7");
                        }
                        writeReg(d.rd, rs1u | rs2u); // or
                        break;

                    case 0x7:
                        if (d.funct7 != 0x00) {
                            invalidInstruction(oldPc, raw, "Invalid AND funct7");
                        }
                        writeReg(d.rd, rs1u & rs2u); // and
                        break;

                    default:
                        invalidInstruction(oldPc, raw, "Unknown OP funct3");
                }

                break;
            }

            case 0x37: { // lui
                writeReg(d.rd, static_cast<uint32_t>(d.immU));
                break;
            }

            case 0x63: { // BRANCH
                bool takeBranch = false;

                switch (d.funct3) {
                    case 0x0: takeBranch = (rs1u == rs2u); break; // beq
                    case 0x1: takeBranch = (rs1u != rs2u); break; // bne
                    case 0x4: takeBranch = (rs1s < rs2s); break;  // blt
                    case 0x5: takeBranch = (rs1s >= rs2s); break; // bge
                    case 0x6: takeBranch = (rs1u < rs2u); break;  // bltu
                    case 0x7: takeBranch = (rs1u >= rs2u); break; // bgeu
                    default: invalidInstruction(oldPc, raw, "Unknown BRANCH funct3");
                }

                if (takeBranch) {
                    nextPc = oldPc + static_cast<uint32_t>(d.immB);
                }

                break;
            }

            case 0x67: { // jalr
                if (d.funct3 != 0x0) {
                    invalidInstruction(oldPc, raw, "Invalid JALR funct3");
                }

                uint32_t target = (rs1u + static_cast<uint32_t>(d.immI)) & ~1u;
                writeReg(d.rd, oldPc + 4);
                nextPc = target;
                break;
            }

            case 0x6F: { // jal
                writeReg(d.rd, oldPc + 4);
                nextPc = oldPc + static_cast<uint32_t>(d.immJ);
                break;
            }

            case 0x73: { // SYSTEM / ECALL
                if (raw != 0x00000073) {
                    invalidInstruction(oldPc, raw, "Only ECALL is supported for SYSTEM opcode");
                }

                uint32_t syscall = regs_[17]; // a7

                switch (syscall) {
                    case 1: // print integer
                        std::cout << static_cast<int32_t>(regs_[10]);
                        break;

                    case 4: { // print string
                        uint32_t address = regs_[10];
                        uint32_t safetyCounter = 0;

                        while (safetyCounter < 4096) {
                            uint8_t c = memory_.read8(address++);

                            if (c == 0) {
                                break;
                            }

                            std::cout << static_cast<char>(c);
                            ++safetyCounter;
                        }

                        break;
                    }

                    case 5: { // read integer
                        int32_t value = 0;
                        std::cin >> value;
                        writeReg(10, static_cast<uint32_t>(value));
                        break;
                    }

                    case 10: // exit
                        halted_ = true;
                        break;

                    case 11: // print character
                        std::cout << static_cast<char>(regs_[10] & 0xFF);
                        break;

                    case 93: // Linux-like exit
                        halted_ = true;
                        break;

                    default:
                        std::cout << "[Unsupported ecall a7=" << syscall << "]";
                        halted_ = true;
                        break;
                }

                break;
            }

            default:
                invalidInstruction(oldPc, raw, "Unknown opcode");
        }

        pc_ = nextPc;
        forceZeroRegister();

        if (trace) {
            std::cout << "Executed PC 0x"
                      << std::hex << std::setw(8) << std::setfill('0') << oldPc
                      << " | instruction 0x"
                      << std::setw(8) << raw
                      << " | next PC 0x"
                      << std::setw(8) << pc_
                      << std::dec << std::setfill(' ')
                      << "\n";
        }
    } catch (const std::exception& error) {
        std::cout << "Runtime error: " << error.what() << "\n";
        halted_ = true;
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
