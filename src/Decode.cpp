#include "Decode.hpp"

int32_t signExtend(uint32_t value, int bits) {
    uint32_t signBit = 1u << (bits - 1);
    return static_cast<int32_t>((value ^ signBit) - signBit);
}

DecodedInstruction decodeInstruction(uint32_t raw) {
    DecodedInstruction d;

    d.raw = raw;
    d.opcode = raw & 0x7F;
    d.rd = (raw >> 7) & 0x1F;
    d.funct3 = (raw >> 12) & 0x07;
    d.rs1 = (raw >> 15) & 0x1F;
    d.rs2 = (raw >> 20) & 0x1F;
    d.funct7 = (raw >> 25) & 0x7F;

    uint32_t immI = (raw >> 20) & 0xFFF;
    d.immI = signExtend(immI, 12);

    uint32_t immS = ((raw >> 7) & 0x1F)
                  | (((raw >> 25) & 0x7F) << 5);
    d.immS = signExtend(immS, 12);

    uint32_t immB = (((raw >> 31) & 0x1) << 12)
                  | (((raw >> 7) & 0x1) << 11)
                  | (((raw >> 25) & 0x3F) << 5)
                  | (((raw >> 8) & 0xF) << 1);
    d.immB = signExtend(immB, 13);

    d.immU = static_cast<int32_t>(raw & 0xFFFFF000);

    uint32_t immJ = (((raw >> 31) & 0x1) << 20)
                  | (((raw >> 12) & 0xFF) << 12)
                  | (((raw >> 20) & 0x1) << 11)
                  | (((raw >> 21) & 0x3FF) << 1);
    d.immJ = signExtend(immJ, 21);

    return d;
}
