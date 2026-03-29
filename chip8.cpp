#include "chip8.hpp"
#include <ranges>
#include <fstream>
#include <stdexcept>

using namespace Chip;
const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

const std::array<Byte, FONTSET_SIZE> fontset
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
 
Chip8::Chip8(): randGen(std::random_device{}()), randByte(0, 255U){
    PC = START_ADDRESS;
    // we need to load fonts
    for (unsigned int i: std::views::iota(static_cast<unsigned int>(0), FONTSET_SIZE)){
        main_memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

void Chip8::LoadROM(const std::string filename){

	// set both flags of binary as well as to have file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()){
		std::uint32_t size = file.tellg();
		file.seekg(0);
		file.read(reinterpret_cast<char *>(&main_memory[START_ADDRESS]), size);
		file.close();
	} else {
		throw std::runtime_error("the file does not exist, or is unable to be opened");
	}
}

void Chip8::Decode(const std::uint16_t instruction){
	Byte nibble = static_cast<Byte>(instruction >> 12);

	switch (nibble) {
		case 0x0: {
			std::uint16_t sub = instruction & 0x00FF;
			if (sub == 0xE0) {
				display.fill(0);
			} else if (sub == 0xEE) {
				SP--;
				PC = level_stack[SP];
			}
			break;
		}
		case 0x1: {
			PC = instruction & 0xFFF;
			break;
		}
		case 0x2: {
			level_stack[SP] = PC;
			SP++;
			PC = instruction & 0xFFF;
			break;
		}
		case 0x3: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t xx = instruction & 0x0FF;

			if (registers[vr] == xx){
				PC += 2;
			}
			
			break;
		}
		case 0x4: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t xx = instruction & 0x0FF;

			if (registers[vr] != xx){
				PC += 2;
			}
			
			break;
		}

		case 0x5: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t vy = (instruction & 0x0F0) >> 4;

			if (registers[vr] == registers[vy]){
				PC += 2;
			}
			break;
		}

		case 0x6: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t xx = instruction & 0x0FF;

			registers[vr] = xx;
			break;
		}
		
		case 0x7: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t xx = instruction & 0x0FF;

			registers[vr] += xx;
			break;
		}
		case 0x8: {
			std::uint16_t rx = (instruction & 0xF00) >> 8;
			std::uint16_t ry = (instruction & 0x0F0) >> 4;
			std::uint16_t opperation = (instruction & 0x00F);

			switch (opperation) {
				case 0: {
					registers[rx] = registers[ry];
					break;
				}
				case 1: {
					registers[rx] |= registers[ry];
					break;
				}
				case 2: {
					registers[rx] &= registers[ry];
					break;
				}
				case 3: {
					registers[rx] ^= registers[ry];
					break;
				}
				case 4: {
					std::uint16_t sum  = registers[rx] + registers[ry];
					registers[rx] += registers[ry];
					if (sum > 0xFF){
						registers[0xF] = 1;
					} else {
						registers[0xF] = 0;
					}
					break;
				}
				case 5: {
					if (registers[rx] >= registers[ry]) {
						registers[0xF] = 1;
					} else {
						registers[0xF] = 0;
					}

					registers[rx] -= registers[ry];
					break;
				}
				case 6: {
					if (registers[rx] & 0x01){
						registers[0xF] = 1;
					} else {
						registers[0xF] = 0;
					}

					registers[rx] >>= 1;
					break;
				}
				case 7: {
					if (registers[ry] >= registers[rx]) {
						registers[0xF] = 1;
					} else {
						registers[0xF] = 0;
					}
					registers[rx] =  registers[ry] - registers[rx];
					break;
				}
				case 0xE: {
					if (registers[rx] & 0x80){
						registers[0xF] = 1;
					} else {
						registers[0xF] = 0;
					}

					registers[rx] <<= 1;
					break;
				}
			}
			break;
		}
		case 0x9: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t vy = (instruction & 0x0F0) >> 4;

			if (registers[vr] != registers[vy]){
				PC += 2;
			}
			break;
		}
		case 0xA: {
			IR = (instruction & 0x0FFF);
			break;
		}
		case 0xB: {
			PC =  (instruction & 0x0FFF) + registers[0];
			break;
		}
		case 0xC: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			registers[vr] = (instruction & 0x00FF) & randByte(randGen);
			break;
		}
		case 0xD: {
			Byte rx = registers[(instruction & 0xF00) >> 8];
			Byte ry = registers[(instruction & 0x0F0) >> 4];
			Byte s  = (instruction & 0x00F);

			rx %= 64;
			ry %= 32;

			registers[0xF] = 0;

			for (Byte i = 0; i < s; i++) {
				if (ry + i >= 32) break;

				Byte spriteByte = main_memory[IR + i];

				std::uint64_t row = static_cast<std::uint64_t>(spriteByte) << 56;
				std::uint64_t spriteRow = (row >> rx) | (row << (64 - rx));

				if (display[ry + i] & spriteRow) {
					registers[0xF] = 1;
				}

				display[ry + i] ^= spriteRow;
			}
			break;
		}
		case 0xE: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t sub = instruction & 0x00FF;

			switch (sub) {
				case 0x9E: {
					if (key[registers[vr]]) PC += 2;
					break;
				}
				case 0xA1: {
					if (!key[registers[vr]]) PC += 2;
					break;
				}
			}
			break;
		}

		case 0xF: {
			std::uint16_t vr = (instruction & 0xF00) >> 8;
			std::uint16_t sub = instruction & 0x00FF;

			switch (sub) {
				case 0x07: {
					registers[vr] = Timer;
					break;
				}
				case 0x0A: {
					bool keyPressed = false;
					for (Byte i = 0; i < 16; i++) {
						if (key[i]) {
							registers[vr] = i;
							keyPressed = true;
							break;
						}
					}
					if (!keyPressed) PC -= 2;
					break;
				}
				case 0x15: {
					Timer = registers[vr];
					break;
				}
				case 0x18: {
					Sound_Timer = registers[vr];
					break;
				}
				case 0x1E: {
					IR += registers[vr];
					break;
				}
				case 0x29: {
					IR = FONTSET_START_ADDRESS + (registers[vr] * 5);
					break;
				}
				case 0x33: {
					Byte val = registers[vr];
					main_memory[IR + 2] = val % 10;
					val /= 10;
					main_memory[IR + 1] = val % 10;
					val /= 10;
					main_memory[IR] = val % 10;
					break;
				}
				case 0x55: {
					for (std::uint16_t i = 0; i <= vr; i++) {
						main_memory[IR + i] = registers[i];
					}
					IR += vr + 1;
					break;
				}
				case 0x65: {
					for (std::uint16_t i = 0; i <= vr; i++) {
						registers[i] = main_memory[IR + i];
					}
					IR += vr + 1;
					break;
				}
			}
			break;
		}
		}
	}


