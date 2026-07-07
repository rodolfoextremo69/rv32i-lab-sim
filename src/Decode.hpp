#pragma once

#include <cstdint>

struct DecodedInstruction {
    uint32_t raw = 0;

    uint32_t opcode = 0;
    uint32_t rd = 0;
    uint32_t funct3 = 0;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    uint32_t funct7 = 0;

    int32_t immI = 0;
    int32_t immS = 0;
    int32_t immB = 0;
    int32_t immU = 0;
    int32_t immJ = 0;
};

int32_t signExtend(uint32_t value, int bits);
DecodedInstruction decodeInstruction(uint32_t raw);
