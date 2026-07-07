#pragma once

#include "Cpu.hpp"

#include <cstdint>
#include <string>

class Shell {
public:
    explicit Shell(Cpu& cpu);

    void loop();

private:
    Cpu& cpu_;

    void executeCommand(const std::string& line);
    static uint32_t parseNumber(const std::string& text);
    static void printHelp();
};
