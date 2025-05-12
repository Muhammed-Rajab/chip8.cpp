#include "../include/chip8.hpp"
#include <cstdint>

// LD Vx, kk (Set Vx = kk)
void Chip8::OP_6xnn() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  V[x] = kk;
}
