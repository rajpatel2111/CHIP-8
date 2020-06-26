
// CHIP8 Emulator

// Include
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "chip8.h"

unsigned char chip8_fontset[80] = {
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

void chip8::init() {
	pc = 0x200;	// Program counter starts at 0x200
	opcode = 0;	// Reset current opcode
	I = 0;	// Reset Index Register
	sp = 0;	// Reset Stack Pointer
	
	// Clear Display
	for (int i = 0; i < 2048; ++i) {
		gfx[i] = 0;
	}
	
	// Clear Stack
	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
	}
	
	// Clear Keys & Registers V0 to VF
	for (int i = 0; i < 16; ++i) {
		key[i] = V[i] = 0;
	}
	
	// Clear Memory
	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}
	
	// Load Fontset
	for (int i = 0; i < 80; ++i) {
		memory[i] = chip8_fontset[i];
	}
	
	// Reset Timers
	delay_timer = 0;
	sound_timer = 0;
	
	// Clear screen once
	drawFlag = true;
	srand(time(NULL));
}

void chip8::emulateCycle() {
	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
	
	// Decode opcode
	switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode & 0x000F) {
				case 0x0000:	// 0x00E0: Clears the screen
					for (int i = 0; i < 2048; ++i) {
						gfx[i] = 0x0;
					}
					drawFlag = true;
					pc += 2;
					break;
				
				case 0x000E:	// 0x00EE: Returns from subroutine
					--sp;
					pc = stack[sp];
					pc += 2;
					break;
				
				default:
					printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
			}
			break;
		
		case 0x1000:	// 0x1NNN: Jumps to address NNN
			pc = opcode & 0x0FFF;
			break;
		
		case 0x2000:	// 0x2NNN: Calls subroutine at NNN
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
			break;
		
		case 0x3000:	// 0x3XNN: Skips the next instruction if VX equals NN
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case 0x4000:	// 0x4XNN: Skips the next instruction if VX doesn't equal NN
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case 0x5000:	// 0x5XY0: Skips the next instruction if VX equals VY
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case 0x6000:	// 0x6XNN: Sets VX to NN
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
			break;
		
		case 0x7000:	// 0x7XNN: Adds NN to VX
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
			break;
		
		case 0x8000:
			switch (opcode & 0x000F) {
				case 0x0000:	// 0x8XY0: Sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case 0x0001:	// 0x8XY1: Sets VX to VX or VY
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case 0x0002:	// 0x8XY2: Sets VX to VX and VY
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case 0x0003:	// 0x8XY3: Sets VX to VX xor VY
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case 0x0004:	// 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
					if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
						V[0xF] = 1;	// Carry
					} else {
						V[0xF] = 0;
					}
					
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case 0x0005:	// 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
						V[0xF] = 0;	// Borrow
					} else {
						V[0xF] = 1;
					}
					
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				case 0x0006:	// 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
					break;
				
				case 0x0007:	// 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
						V[0xF] = 0;	// Borrow
					} else {
						V[0xF] = 1;
					}
					
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				
				case 0x000E:	// 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
					break;
				
				default:
					printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
			}
			break;
		
		case 0x9000:	// 0x9XY0: Skips the next instruction if VX doesn't equal VY
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		
		case 0xA000:	// ANNN: Sets I to the address NNN
			// Execute opcode
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		
		case 0xB000:	// 0xBNNN: Jumps to the address NNN plus V0
			pc = (opcode & 0x0FFF) + V[0];
			break;
		
		case 0xC000:	// 0xCXNN: Sets VX to a random number, masked by NN
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
			break;
		
		case 0xD000:	// 0xDXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
						// Each row of 8 pixels is read as bit-coded starting from memory location I;
						// I value doesn’t change after the execution of this instruction.
						// As described above, VF is set to 1 if any screen pixels are flipped from set to unset
						// when the sprite is drawn, and to 0 if that doesn’t happen
			{
				unsigned short x = V[(opcode & 0x0F00) >> 8];
				unsigned short y = V[(opcode & 0x00F0) >> 4];
				unsigned short height = opcode & 0x000F;
				unsigned short pixel;
				
				V[0xF] = 0;
				
				for (int yline = 0; yline < height; yline++) {
					pixel = memory[I + yline];
					
					for (int xline = 0; xline < 8; xline++) {
						if ((pixel & (0x80 >> xline)) != 0) {
							if (gfx[x + xline + ((y + yline) * 64)] == 1) {
								V[0xF] = 1;
							}
							gfx[x + xline + ((y + yline) * 64)] ^= 1;
						}
					}
				}
				
				drawFlag = true;
				pc += 2;
			}
			break;
		
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x009E:	// EX9E: Skips the next instruction if the key stored in VX is pressed
					if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
						pc += 4;
					} else {
						pc += 2;
					}
					break;
				
				case 0x00A1:	// 0xEXA1: Skips the next instruction if the key stored in VX isn't pressed
					if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
						pc += 4;
					} else {
						pc += 2;
					}
					break;
				
				default:
					printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
			}
			break;
		
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007:	// 0xFX07: Sets VX to the value of the delay timer
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
					break;
				
				case 0x000A:	// 0xFX0A: A key press is awaited, and then stored in VX
					{
						bool keyPress = false;
						
						for (int i = 0; i < 16; ++i) {
							if (key[i] != 0) {
								V[(opcode & 0x0F00) >> 8] = i;
								keyPress = true;
							}
						}
						
						// If there was no key press received then skip this cycle and try again
						if (!keyPress) {
							return;
						}
						
						pc += 2;
					}
					break;
				
				case 0x0015:	// 0xFX15: Sets the delay timer to VX
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				
				case 0x0018:	// 0xFX18: Sets the sound timer to VX
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				
				case 0x001E:	// 0xFX1E: Adds VX to I.
								// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
								// This is undocumented feature of the CHIP-8 and used by Spacefight 2091! game.
					if ((I + V[(opcode & 0x0F00) >> 8]) > 0xFFF) {
						V[0xF] = 1;
					} else {
						V[0xF] = 0;
					}
					
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
					break;
				
				case 0x0029:	// 0xFX29: Sets I to the location of the sprite for the character in VX.
								// Characters 0-F (in hexadecimal) are represented by a 4x5 font
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
					break;
				
				case 0x0033:	// FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
					memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
					pc += 2;
					break;
				
				case 0x0055:	// 0xFX55: Stores V0 to VX in memory starting at address I
								// On the original interpreter, when the operation is done,
								// I=I+X+1. On current implementations, I is left unchanged.
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
						memory[I + i] = V[i];
					}
					
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
					break;
				
				case 0x0065:	// 0xFX65: Fills V0 to VX with values from memory starting at address I
								// On the original interpreter, when the operation is done,
								// I=I+X+1. On current implementations, I is left unchanged.
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
						V[i] = memory[I + i];
					}
					
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
					break;
				
				default:
					printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
			}
			break;
		
		default:
			printf("Unknown opcode: 0x%X\n", opcode);
	}
	
	// Update Timers
	if (delay_timer > 0) {
		--delay_timer;
	}
	
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			printf("BEEP!\n");
		}
		--sound_timer;
	}
}

void chip8::debugRender() {
	// Draw
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			if (gfx[(y * 64) + x] == 0) {
				printf("O");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
	printf("\n");
}

bool chip8::loadApplication(const char* filename) {
	init();
	printf("Loading: %s\n", filename);
	
	// Open file
	FILE* pFile = fopen(filename, "rb");
	if (pFile == NULL) {
		fputs("File error", stderr);
		return false;
	}
	
	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int) lSize);
	
	// Allocate memory to contain the whole file
	char* buffer = (char*) malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) {
		fputs("Reading error", stderr);
		return false;
	}

	// Copy buffer to CHIP8 memory
	if ((4096 - 512) > lSize) {
		for (int i = 0; i < lSize; ++i) {
			memory[i + 512] = buffer[i];
		}
	} else {
		printf("Error: ROM too big for memory");
	}

	// Close the file and free the buffer
	fclose(pFile);
	free(buffer);

	return true;
}

