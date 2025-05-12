#include "../include/chip8.hpp"

#include <cstdint>
#include <iostream>

// DEFAULT HANDLER
void Chip8::OP_NULL() {
  std::cerr << "TF AM I SUPPOSED TO DO? IDK NO 0x" << std::hex << opcode
            << std::dec << "!\n";
}

// JMP nnn
void Chip8::OP_1nnn() {
  uint16_t nnn = opcode & 0x0FFFu;
  pc = nnn;
}

// CALL nnn
void Chip8::OP_2nnn() {
  uint16_t nnn = opcode & 0x0FFFu;
  stack[sp] = pc;
  sp += 1;
  pc = nnn;
}

// SE Vx, kk (Skip next construction if Vx == kk)
void Chip8::OP_3xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  if (V[x] == kk) {
    pc += 2;
  }
}

void Chip8::OP_4xkk() {
  std::cout << "under construction" << std::endl; //
}

void Chip8::OP_5xy0() {
  std::cout << "under construction" << std::endl; //
}

// LD Vx, kk (Set Vx = kk)
void Chip8::OP_6xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  V[x] = kk;
}

// ADD Vx, kk (Set Vx = Vx + kk)
void Chip8::OP_7xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  V[x] = V[x] + kk;
}
