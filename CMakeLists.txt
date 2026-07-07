cmake_minimum_required(VERSION 3.20)
project(rv32i_lab_sim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rv32i_lab_sim
        src/main.cpp
        src/Memory.cpp
        src/Decode.cpp
        src/Cpu.cpp
        src/Shell.cpp
)
