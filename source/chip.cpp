#include <iostream>
#include <fstream>

using namespace std;


const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_SIZE = 80;


// list of all the fonts with their hexa code
uint8_t fontset[FONTSET_SIZE] =
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

const unsigned int FONTSET_START_ADDRESS = 0x50;


Chip8::Chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())

{
	// Initialize the program counter
	pc = START_ADDRESS;

	// Load the fonts to the memory
	for (unsigned int i = 0; i < FONTSET_SIZE: i++)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	// Initialize RNG
	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

	default_random_engine randGen;
	uniform_int_distribution<uint8_t> randByte;
}

void Chip8::LoadROM(char const *filename)
{
	// open file 
	ifstream file(filename, ios::binary | ios::ate);
	
	if(file.is_open()){
		// get the size of the file and allocate a buffer with the same size
		streampos size = file.tellg();
		char* buffer = new char[size];

		// start from the beggining of the file and fill the buffer
		file.seekg(0, ios::beg);
		file.read(buffer, size);
		file.close();

		//load the ROM contents into the chip-8 memory
		for( long i = 0; i < size; i++){
		       memory[START_ADDRESS +i] = buffer[i];
		}

		delete[] buffer;
	}
}


//////////////////////////////////////// The instructions //////////////////////////////////////

// CLS instruction
void Chip8::OP_00E0()
{
	memset(video, 0, sizeof(video));
}

// RET instruction
// return from a subroutine
void Chip8::OP_00EE()
{
	--sp;
	pc = stack[sp];
}

// JP address instruction
// jump to location nnn
void Chip8::OP_1nnn()
{
	uint16_t address = opcode & 0x0FFFu;

	pc = address;
}

// CALL address
//call subroutine at nnn 
void Chip8::OP_2nnn()
{
	uint16_t address = opcode & 0x0FFFu;

	stack[sp] = pc;
	sp++;
	pc = address;
}

// SE VX, byte
// skip next instruction if Vx = kk
void Chip8::OP_3xkk()
{
	uint8_t Vx = (opcode &0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if(registers[Vx] == byte)
	{
		pc += 2;
	}
}

// SNE Vx, byte
// skip next instruction if Vx != kk
void Chip8::OP_4xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}

//SE Vx, Vy
//skip next instruction if Vx = Vy
void Chip::OP_5xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if(registers[Vx] == registers[Vy])
	{
		pc += 2;
	}

}

//LD Vx, byte
// set Vx = kk
void Chip8::OP_6xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

// ADD Vx, byte
// set Vx = Vx + kk
void Chip8::OP_7xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}


// LD Vx, Vy
// set Vx = Vy
void Chip8::OP_8xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

// OR Vx, Vy
// set Vx = Vx OR Vy
void Chip8::OP_8xy1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

// AND Vx, Vy
// set Vx = Vx AND Vy
Chip8::OP_8xy2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	
	registers[Vx] &= registers[Vy];
}

// ADD Vx, Vy
// set Vx = Vx + Vy
// The values of Vx and Vy are added together. 
// If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, 
// otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
void Chip8::OP_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];
	Vf = 0xF;
	if (sum > 255U)
	{
		registers[Vf] = 1;
	}
	else
	{
		registers[Vf] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}

// SUB Vx, Vy
// set Vx = Vx - Vy, set Vf = NOt borrow
// If Vx > Vy, then VF is set to 1, otherwise 0. 
// Then Vy is subtracted from Vx, and the results stored in Vx.
void Chip8::OP_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	
	Vf = 0xF;

	if (registers[Vx] > registers[Vy])
	{
		registers[Vf] = 1;
	}
	else
	{
		registers[Vf] = 0;
	}

	registers[Vx] -= registers[Vy];
}

// SHR Vx
// set Vx = Vx SHR 1
// If the least-significant bit of Vx is 1, then VF is set to 1, 
// otherwise 0. Then Vx is divided by 2.
void Chip8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0f00u) >> 8u;

	Vf = 0xF;
	// Save least significant bit (LSB) in Vf
	registers[Vf] = (registers[Vx] & 0x1u);
	
	// shift right one bit
	registers[Vx] >>= 1;
}

// SUBN Vx, Vy
// set Vx = Vy - Vx, set Vf = Not borrow
// If Vy > Vx, then VF is set to 1, otherwise 0. 
// Then Vx is subtracted from Vy, and the results stored in Vx.
void Chip8::OP_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	Vf = 0xF;

	if (registers[Vy] > registers[Vx])
	{
		registers[Vf] = 1;
	}
	else
	{
		registers[Vf] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

// SHL Vx
// set Vx = Vx SHL 1
// If the most-significant bit of Vx is 1, then VF is set to 1, 
// otherwise to 0. Then Vx is multiplied by 2.
void Chip8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

// SNE Vx, Vy
// skip next instruction if Vx != Vy
void Chip8::OP_9xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

// LD I, address
// set I = nnn
void Chip::OP_Annn()
{
	uint16_t address = opcode & 0x0FFFu;

	index = address;
}































