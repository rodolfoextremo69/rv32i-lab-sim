#include "Shell.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

Shell::Shell(Cpu& cpu)
    : cpu_(cpu) {}

uint32_t Shell::parseNumber(const std::string& text) {
    int base = 10;

    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
    }

    return static_cast<uint32_t>(std::stoul(text, nullptr, base));
}

void Shell::printHelp() {
    std::cout << "Commands:\n"
              << "  help              Show commands\n"
              << "  state             Show PC and CPU status\n"
              << "  step              Execute one instruction\n"
              << "  run N             Execute N instructions\n"
              << "  regs              Show all registers\n"
              << "  reg NAME          Show one register, example: reg a0 or reg x10\n"
              << "  mem ADDR COUNT    Show COUNT bytes from memory\n"
              << "  quit              Exit simulator\n";
}

void Shell::loop() {
    printHelp();

    std::string line;

    while (true) {
        std::cout << "rv32i> ";

        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.empty()) {
            continue;
        }

        executeCommand(line);
    }
}

void Shell::executeCommand(const std::string& line) {
    std::istringstream input(line);

    std::string cmd;
    input >> cmd;

    if (cmd == "help") {
        printHelp();
    } else if (cmd == "state") {
        cpu_.printState();
    } else if (cmd == "step") {
        cpu_.step();
    } else if (cmd == "run") {
        std::string amountText;
        input >> amountText;

        if (amountText.empty()) {
            std::cout << "Usage: run N\n";
            return;
        }

        cpu_.run(parseNumber(amountText));
    } else if (cmd == "regs") {
        cpu_.printRegisters();
    } else if (cmd == "reg") {
        std::string name;
        input >> name;

        if (name.empty()) {
            std::cout << "Usage: reg NAME\n";
            return;
        }

        cpu_.printRegister(name);
    } else if (cmd == "mem") {
        std::string addressText;
        std::string countText;

        input >> addressText >> countText;

        if (addressText.empty() || countText.empty()) {
            std::cout << "Usage: mem ADDR COUNT\n";
            return;
        }

        uint32_t address = parseNumber(addressText);
        uint32_t count = parseNumber(countText);

        cpu_.printMemory(address, count);
    } else if (cmd == "quit" || cmd == "exit") {
        std::cout << "Bye.\n";
        std::exit(0);
    } else {
        std::cout << "Unknown command. Type help.\n";
    }
}
