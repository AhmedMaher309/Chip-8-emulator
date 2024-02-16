#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>

#include "chip.hpp"

using namespace std;

const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_SIZE = 80;

// list of all the fonts with their hexa code
uint8_t fontset[FONTSET_SIZE] = {
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
  for (unsigned int i = 0; i < FONTSET_SIZE; i++) {
    memory[FONTSET_START_ADDRESS + i] = fontset[i];
  }

  // Initialize RNG
  randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

  // Set up function pointer table
  table[0x0] = &Chip8::Table0;
  table[0x1] = &Chip8::OP_1nnn;
  table[0x2] = &Chip8::OP_2nnn;
  table[0x3] = &Chip8::OP_3xkk;
  table[0x4] = &Chip8::OP_4xkk;
  table[0x5] = &Chip8::OP_5xy0;
  table[0x6] = &Chip8::OP_6xkk;
  table[0x7] = &Chip8::OP_7xkk;
  table[0x8] =
      &Chip8::Table8; // doesn't excute commands but go to the Table8 function
  table[0x9] = &Chip8::OP_9xy0;
  table[0xA] = &Chip8::OP_Annn;
  table[0xB] = &Chip8::OP_Bnnn;
  table[0xC] = &Chip8::OP_Cxkk;
  table[0xD] = &Chip8::OP_Dxyn;
  table[0xE] = &Chip8::TableE;
  table[0xF] = &Chip8::TableF;

  // make sure that the indeces that doesn't represent any instructions
  // are all assigned to the NULL instruction
  for (size_t i = 0; i <= 0xE; i++) {
    table0[i] = &Chip8::OP_NULL;
    table8[i] = &Chip8::OP_NULL;
    tableE[i] = &Chip8::OP_NULL;
  }

  table0[0x0] = &Chip8::OP_00E0;
  table0[0xE] = &Chip8::OP_00EE;

  table8[0x0] = &Chip8::OP_8xy0;
  table8[0x1] = &Chip8::OP_8xy1;
  table8[0x2] = &Chip8::OP_8xy2;
  table8[0x3] = &Chip8::OP_8xy3;
  table8[0x4] = &Chip8::OP_8xy4;
  table8[0x5] = &Chip8::OP_8xy5;
  table8[0x6] = &Chip8::OP_8xy6;
  table8[0x7] = &Chip8::OP_8xy7;
  table8[0xE] = &Chip8::OP_8xyE;

  tableE[0x1] = &Chip8::OP_ExA1;
  tableE[0xE] = &Chip8::OP_Ex9E;

  // make sure that the indeces that doesn't represent any instructions
  // are all assigned to the NULL instruction
  for (size_t i = 0; i <= 0x65; i++) {
    tableF[i] = &Chip8::OP_NULL;
  }

  tableF[0x07] = &Chip8::OP_Fx07;
  tableF[0x0A] = &Chip8::OP_Fx0A;
  tableF[0x15] = &Chip8::OP_Fx15;
  tableF[0x18] = &Chip8::OP_Fx18;
  tableF[0x1E] = &Chip8::OP_Fx1E;
  tableF[0x29] = &Chip8::OP_Fx29;
  tableF[0x33] = &Chip8::OP_Fx33;
  tableF[0x55] = &Chip8::OP_Fx55;
  tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::LoadROM(char const *filename) {
  // open file
  ifstream file(filename, ios::binary | ios::ate);

  if (file.is_open()) {
    // get the size of the file and allocate a buffer with the same size
    streampos size = file.tellg();
    char *buffer = new char[size];

    // start from the beggining of the file and fill the buffer
    file.seekg(0, ios::beg);
    file.read(buffer, size);
    file.close();

    // load the ROM contents into the chip-8 memory
    for (long i = 0; i < size; i++) {
      memory[START_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;
  }
}

// Fetch, Decode, and Excute
// - Fetch the next instruction in the form of an opcode
// - Decode the instruction to determine what operation needs to occur
// - Execute the instruction
void Chip8::Cycle() {
  // Fetch
  opcode = (memory[pc] << 8u) | memory[pc + 1];

  // Increment the PC before we execute anything
  pc += 2;

  // Decode and Execute
  // extract the significant four bits to determine
  // the index in the table to the opcode's instruction type
  ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

  // Decrement the delay timer if it's been set
  if (delayTimer > 0) {
    --delayTimer;
  }

  // Decrement the sound timer if it's been set
  if (soundTimer > 0) {
    --soundTimer;
  }
}

void Chip8::Table0() { ((*this).*(table0[opcode & 0x000Fu]))(); }

void Chip8::Table8() { ((*this).*(table8[opcode & 0x000Fu]))(); }

void Chip8::TableE() { ((*this).*(tableE[opcode & 0x000Fu]))(); }

void Chip8::TableF() { ((*this).*(tableF[opcode & 0x00FFu]))(); }

//////////////////////////////////////// The instructions
/////////////////////////////////////////

// NULL instruction
void Chip8::OP_NULL() {}

// CLS instruction
void Chip8::OP_00E0() { memset(video, 0, sizeof(video)); }

// RET instruction
// return from a subroutine
void Chip8::OP_00EE() {
  sp--;
  pc = stack[sp];
}

// JP address instruction
// jump to location nnn
void Chip8::OP_1nnn() {
  uint16_t address = opcode & 0x0FFFu;

  pc = address;
}

// CALL address
// call subroutine at nnn
void Chip8::OP_2nnn() {
  uint16_t address = opcode & 0x0FFFu;

  stack[sp] = pc;
  sp++;
  pc = address;
}

// SE VX, byte
// skip next instruction if Vx = kk
void Chip8::OP_3xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  if (registers[Vx] == byte)
    pc += 2;
}

// SNE Vx, byte
// skip next instruction if Vx != kk
void Chip8::OP_4xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  if (registers[Vx] != byte)
    pc += 2;
}

// SE Vx, Vy
// skip next instruction if Vx = Vy
void Chip8::OP_5xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] == registers[Vy])
    pc += 2;
}

// LD Vx, byte
//  set Vx = kk
void Chip8::OP_6xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  registers[Vx] = byte;
}

// ADD Vx, byte
// set Vx = Vx + kk
void Chip8::OP_7xkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  registers[Vx] += byte;
}

// LD Vx, Vy
// set Vx = Vy
void Chip8::OP_8xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] = registers[Vy];
}

// OR Vx, Vy
// set Vx = Vx OR Vy
void Chip8::OP_8xy1() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] |= registers[Vy];
}

// AND Vx, Vy
// set Vx = Vx AND Vy
void Chip8::OP_8xy2() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] &= registers[Vy];
}

// XOR Vx, Vy
// set Vx = Vx XOR Vy
void Chip8::OP_8xy3() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  registers[Vx] ^= registers[Vy];
}

// ADD Vx, Vy
// set Vx = Vx + Vy
// The values of Vx and Vy are added together.
// If the result is greater than 8 bits (i.e., > 255,) VF is set to 1,
// otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
void Chip8::OP_8xy4() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  uint16_t sum = registers[Vx] + registers[Vy];
  uint8_t Vf = 0xF;

  if (sum > 255U)
    registers[Vf] = 1;
  else
    registers[Vf] = 0;

  registers[Vx] = sum & 0xFFu;
}

// SUB Vx, Vy
// set Vx = Vx - Vy, set Vf = NOt borrow
// If Vx > Vy, then VF is set to 1, otherwise 0.
// Then Vy is subtracted from Vx, and the results stored in Vx.
void Chip8::OP_8xy5() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  uint8_t Vf = 0xF;

  if (registers[Vx] > registers[Vy])
    registers[Vf] = 1;
  else
    registers[Vf] = 0;

  registers[Vx] -= registers[Vy];
}

// SHR Vx
// set Vx = Vx SHR 1
// If the least-significant bit of Vx is 1, then VF is set to 1,
// otherwise 0. Then Vx is divided by 2.
void Chip8::OP_8xy6() {
  uint8_t Vx = (opcode & 0x0f00u) >> 8u;

  uint8_t Vf = 0xF;
  // Save least significant bit (LSB) in Vf
  registers[Vf] = (registers[Vx] & 0x1u);

  // shift right one bit
  registers[Vx] >>= 1;
}

// SUBN Vx, Vy
// set Vx = Vy - Vx, set Vf = Not borrow
// If Vy > Vx, then VF is set to 1, otherwise 0.
// Then Vx is subtracted from Vy, and the results stored in Vx.
void Chip8::OP_8xy7() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  uint8_t Vf = 0xF;

  if (registers[Vy] > registers[Vx])
    registers[Vf] = 1;
  else
    registers[Vf] = 0;

  registers[Vx] = registers[Vy] - registers[Vx];
}

// SHL Vx
// set Vx = Vx SHL 1
// If the most-significant bit of Vx is 1, then VF is set to 1,
// otherwise to 0. Then Vx is multiplied by 2.
void Chip8::OP_8xyE() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  // Save MSB in VF
  uint8_t Vf = 0xf;
  registers[Vf] = (registers[Vx] & 0x80u) >> 7u;

  registers[Vx] <<= 1;
}

// SNE Vx, Vy
// skip next instruction if Vx != Vy
void Chip8::OP_9xy0() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  if (registers[Vx] != registers[Vy])
    pc += 2;
}

// LD I, address
// set I = nnn
void Chip8::OP_Annn() {
  uint16_t address = opcode & 0x0FFFu;
  index = address;
}

// JP V0, address
// jump to location nnn + V0
void Chip8::OP_Bnnn() {
  uint16_t address = opcode & 0x0FFFu;
  pc = registers[0] + address;
}

// RND Vx, byte
// set Vx = random byte AND kk
void Chip8::OP_Cxkk() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  registers[Vx] = randByte(randGen) & byte;
}

// DRW Vx, Vy, nibble
void Chip8::OP_Dxyn() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;
  uint8_t height = opcode & 0x000Fu;

  // Wrap if going beyond screen boundaries
  uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
  uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

  registers[0xF] = 0;

  for (unsigned int row = 0; row < height; ++row) {
    uint8_t spriteByte = memory[index + row];

    for (unsigned int col = 0; col < 8; ++col) {
      uint8_t spritePixel = spriteByte & (0x80u >> col);
      uint32_t *screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

      // Sprite pixel is on
      if (spritePixel) {
        // Screen pixel also on - collision
        if (*screenPixel == 0xFFFFFFFF) {
          registers[0xF] = 1;
        }

        // Effectively XOR with the sprite pixel
        *screenPixel ^= 0xFFFFFFFF;
      }
    }
  }
}

// SKP Vx
// skip next instruction if the key with the value of Vx is pressed
void Chip8::OP_Ex9E() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  uint8_t key = registers[Vx];

  if (keypad[key])
    pc += 2;
}

// SKNP
// skip next instruction if the key with the value of Vx is not pressed
void Chip8::OP_ExA1() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  uint8_t key = registers[Vx];

  if (!keypad[key])
    pc += 2;
}

// LD Vx, DT
// set Vx = delat timer value
void Chip8::OP_Fx07() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  registers[Vx] = delayTimer;
}

// LD Vx, K
// wait for a key press. store the value of the key in Vx
void Chip8::OP_Fx0A() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  if (keypad[0])
    registers[Vx] = 0;
  else if (keypad[1])
    registers[Vx] = 1;
  else if (keypad[2])
    registers[Vx] = 2;
  else if (keypad[3])
    registers[Vx] = 3;
  else if (keypad[4])
    registers[Vx] = 4;
  else if (keypad[5])
    registers[Vx] = 5;
  else if (keypad[6])
    registers[Vx] = 6;
  else if (keypad[7])
    registers[Vx] = 7;
  else if (keypad[8])
    registers[Vx] = 8;
  else if (keypad[9])
    registers[Vx] = 9;
  else if (keypad[10])
    registers[Vx] = 10;
  else if (keypad[11])
    registers[Vx] = 11;
  else if (keypad[12])
    registers[Vx] = 12;
  else if (keypad[13])
    registers[Vx] = 13;
  else if (keypad[14])
    registers[Vx] = 14;
  else if (keypad[15])
    registers[Vx] = 15;
  else
    pc -= 2;
}

// LD DT, Vx
// set delay timer = Vx
void Chip8::OP_Fx15() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  delayTimer = registers[Vx];
}

// LD ST, Vx
// set sound timer = Vx
void Chip8::OP_Fx18() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  soundTimer = registers[Vx];
}

// ADD I, Vx
// set I = I + Vx
void Chip8::OP_Fx1E() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  index += registers[Vx];
}

// LD I, Vx
// set I = I + Vx
void Chip8::OP_Fx29() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t digit = registers[Vx];
  index = FONTSET_START_ADDRESS + (5 * digit);
}

// LD B, Vx
// store the BCD representation of Vx in memory locations I, I+1, and I+2
//  I   --> hundreds
//  I+1 --> tens
//  I+2 --> units
void Chip8::OP_Fx33() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t value = registers[Vx];

  // Ones-place
  memory[index + 2] = value % 10;
  value /= 10;

  // Tens-place
  memory[index + 1] = value % 10;
  value /= 10;

  // Hundreds-place
  memory[index] = value % 10;
}

// LD [I], Vx
// store registers V0 through Vx in the memory starting from location I
void Chip8::OP_Fx55() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  for (uint8_t i = 0; i <= Vx; ++i)
    memory[index + i] = registers[i];
}

// LD Vx,[I]
// Read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;

  for (uint8_t i = 0; i <= Vx; ++i)
    registers[i] = memory[index + i];
}
