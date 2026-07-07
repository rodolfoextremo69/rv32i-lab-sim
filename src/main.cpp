#include "Cpu.hpp"
#include "Shell.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage:\n"
                  << "  rv32i_lab_sim program.bin\n";
        return 1;
    }

    try {
        Cpu cpu;
        cpu.loadProgram(argv[1]);

        Shell shell(cpu);
        shell.loop();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
