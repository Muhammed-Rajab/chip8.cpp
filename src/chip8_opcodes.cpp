#include "../include/chip8.hpp"

#include <cstdint>
#include <iostream>

// DEFAULT HANDLER
void Chip8::OP_NULL() {
  std::cerr << "TF AM I SUPPOSED TO DO? IDK NO 0x" << std::hex << opcode
            << std::dec << "!\n";
}

// LD Vx, kk (Set Vx = kk)
void Chip8::OP_6xnn() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  V[x] = kk;
}
