#include "../include/chip8.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>

// DEFAULT HANDLER
void Chip8::OP_NULL() {
  std::cerr << "TF AM I SUPPOSED TO DO? IDK NO 0x" << std::hex << opcode
            << std::dec << "!\n";
}

// HALT (Stops execution)
void Chip8::OP_FxFF() { halted = true; }

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

// SE Vx, kk (Skip next instruction if Vx == kk)
void Chip8::OP_3xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  if (V[x] == kk) {
    pc += 2;
  }
}

// SNE Vx, kk (Skip next instruction if Vx != kk)
void Chip8::OP_4xkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  if (V[x] != kk) {
    pc += 2;
  }
}

// SE Vx, Vy (Skip next instruction if Vx = Vy)
void Chip8::OP_5xy0() {

  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  if (V[x] == V[y]) {
    pc += 2;
  }
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

// SNE Vx, Vy (Skip next instruction if Vx != Vy)
void Chip8::OP_9xy0() {

  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  if (V[x] != V[y]) {
    pc += 2;
  }
}

// LD I, nnn
void Chip8::OP_Annn() {
  uint16_t nnn = opcode & 0x0FFFu;
  index = nnn;
}

// JMP V0, nnn (Jump to location nnn + V0)
void Chip8::OP_Bnnn() {
  uint16_t nnn = opcode & 0x0FFFu;
  pc = nnn + V[0];
}

// RND Vx, byte (Set Vx = random byte & kk)
void Chip8::OP_Cxkk() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t kk = (opcode & 0x00FFu);

  V[x] = (rand() % 256) & kk;
}

// DRW Vx, Vy, nibble
void Chip8::OP_Dxyn() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  uint8_t n = (opcode & 0x000Fu);

  uint8_t x_pos = V[x] % VIDEO_WIDTH;
  uint8_t y_pos = V[y] % VIDEO_HEIGHT;

  V[0xF] = 0;

  for (int j = 0; j < n; j += 1) {

    uint8_t byte = memory[index + j];

    for (int i = 0; i < 8; i += 1) {

      bool collision = byte & (0x80 >> i);

      if (collision != 0) {
        uint8_t x_coord = (x_pos + i) % VIDEO_WIDTH;
        uint8_t y_coord = (y_pos + j) % VIDEO_HEIGHT;

        uint16_t v_index = y_coord * VIDEO_WIDTH + x_coord;

        if (video[v_index] == 1) {
          V[0xF] = 1;
        }

        video[v_index] ^= 1;
      }
    }
  }
}
