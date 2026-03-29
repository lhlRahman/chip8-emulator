#include <array>
#include <cstdint>
#include <iostream>
#include <chrono>
#include "chip8.hpp"
#include "platform.hpp"

// building a chip 8 emulator, each register will be a unsigned char or maybe a
// std::byte 4 memory will be just an array of size 4096 bytes since the IR and
// PC are 16 bits it will have 8 bit stack pointer, 8 bit delay timer, 8 bit
// sound timer, and 16 input keys a display of 64 x 32
//
//
// it will have 16 registers

int main(int argc, char **argv) {
    if (argc < 2){
        throw std::runtime_error("no ROM filename provided");
    }
    Chip::Chip8 chip;
    chip.LoadROM(argv[1]);

    Platform screen("CHIP 8", 1024, 512, 64, 32);

    int cycleDelay = 1;
    int videoPitch = 64 * sizeof(uint32_t);
    bool quit = false;
    auto lastCycleTime = std::chrono::high_resolution_clock::now();

    std::array<uint32_t, 64 * 32> videoBuffer = {0};

    while (!quit)
    {
        quit = screen.ProcessInput(chip.key.data());

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;
            chip.Cycle();

            for (int row = 0; row < 32; row++) {
                for (int col = 0; col < 64; col++) {
                    bool pixelOn = (chip.display[row] >> (63 - col)) & 1;
                    videoBuffer[row * 64 + col] = pixelOn ? 0xFFFFFFFF : 0x00000000;
                }
            }

            screen.Update(videoBuffer.data(), videoPitch);
        }
    }
}

