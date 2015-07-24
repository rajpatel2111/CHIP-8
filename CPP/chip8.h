
// CHIP8 Emulator

#ifndef _CHIP8_H_
#define _CHIP8_H_

class chip8 {
	
	public:
		chip8();
		~chip8();
		
		bool drawFlag;
		
		unsigned char gfx[64 * 32];	// Screen has 2048 (64 * 32) pixels
		unsigned char key[16];	// HEX Keypad (0x0 to 0xF)
		
		void emulateCycle();
		void debugRender();
		bool loadApplication(const char* filename);
	
	private:
		unsigned short I;	// Index Register
		unsigned short pc;	// Program Counter
		unsigned short sp;	// Stack Pointer
		unsigned short opcode;	// Current Opcode
		
		unsigned short stack[16];	// Stack (16 levels)
		
		unsigned char V[16];	// General Purpose Registers (V0 - VE) and VF (carry flag)
		unsigned char memory[4096];	// Total 4K Memory
		
		// Timers countdown at 60 Hz
		unsigned char delay_timer;
		unsigned char sound_timer;
		
		void init();
};

inline chip8::chip8() {}

inline chip8::~chip8() {}

#endif

