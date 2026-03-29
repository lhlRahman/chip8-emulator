#pragma once
#include <array>
#include <cstdint>
#include <random>

using Byte = unsigned char;

namespace Chip {

class Chip8 {
    public:
        Chip8();

        std::array<Byte, 16> registers = {0};
        std::array<Byte, 1 << 12> main_memory = {0};

        
        std::uint16_t IR = 0;
        std::uint16_t PC = 0;

        std::array<std::uint16_t, 16> level_stack = {0};
        Byte SP = 0;
        Byte Timer = 0;
        Byte Sound_Timer = 0;

        std::array<Byte, 16> key = {0};
        std::array<std::uint32_t, 64> display = {0};
        std::uint16_t opcode;

        void LoadROM(const std::string filename);
        void Decode(const std::uint16_t instruction);


    private:
        std::mt19937 randGen;
        std::uniform_int_distribution<std::uint16_t> randByte;

};

}