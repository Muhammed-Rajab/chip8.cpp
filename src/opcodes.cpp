#include "../include/chip8.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/types.h>

// DEFAULT HANDLER
void Chip8::OP_NULL() {
  std::ostringstream msg;
  msg << "Unhandled opcode: 0x" << std::hex << std::uppercase << opcode
      << " at PC: 0x" << std::setw(4) << std::setfill('0') << pc;
  throw std::runtime_error(msg.str());
}

// HALT (Stops execution)
void Chip8::OP_FxFF() { halted = true; }

// CLS (Clears screen)
void Chip8::OP_00E0() { memset(video, 0, sizeof(video)); }

// RET (Return from a subroutine)
void Chip8::OP_00EE() {
  sp -= 1;
  pc = stack[sp];
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

// LD Vx, Vy (Set Vx = Vy)
void Chip8::OP_8xy0() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  V[x] = V[y];
}

// OR Vx, Vy
void Chip8::OP_8xy1() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  V[x] |= V[y];
}

// AND Vx, Vy
void Chip8::OP_8xy2() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  V[x] &= V[y];
}

// XOR Vx, Vy
void Chip8::OP_8xy3() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  V[x] ^= V[y];
}

// ADD Vx, Vy (set Vx = Vx + Vy, set Vf = 1 if carry else 0)
void Chip8::OP_8xy4() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  uint16_t sum = V[x] + V[y];

  V[0xF] = sum > 0xFF ? 1 : 0;
  V[x] = sum & 0xFFu;
}

// SUB Vx, Vy (set Vx = Vx - Vy, set Vf = 1 if not borrow else 0)
void Chip8::OP_8xy5() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  V[0xF] = V[x] > V[y] ? 1 : 0;

  V[x] -= V[y];
}

// SHR Vx (Right shift Vx by 1, and set Vf = 1 if LSB of Vx is 1 else 0)
void Chip8::OP_8xy6() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;

  V[0xF] = V[x] & 0x1;
  V[x] >>= 1;
}

// SUBN Vx, Vy (Set Vx = Vy - Vx, and set Vf = 1 if Vy > Vx, else 0)
void Chip8::OP_8xy7() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;

  V[0xF] = V[y] > V[x] ? 1 : 0;
  V[x] = V[y] - V[x];
}

// SHL Vx (Left shift Vx by 1, and set Vf = 1 if MSB of Vx is 1 else 0)
void Chip8::OP_8xyE() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;

  V[0xF] = (V[x] & 0x80) >> 7u;
  V[x] <<= 1;
}

// SKP Vx (Skip next instruction if key with the value of Vx is pressed)
void Chip8::OP_Ex9E() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;

  if (keypad[V[x]]) {
    pc += 2;
  }
}

// SKPN Vx (Skip next instruction if key with the value of Vx is not pressed)
void Chip8::OP_ExA1() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;

  if (!keypad[V[x]]) {
    pc += 2;
  }
}

// LD Vx, DT
void Chip8::OP_Fx07() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;

  V[x] = delay;
}

// LD Vx, K (Wait for keypress, store the value of the key in Vx)
void Chip8::OP_Fx0A() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Wait for a key press, store the value of the key in Vx

  for (uint8_t i = 0; i < 16; i += 1) {
    if (keypad[i]) {
      V[x] = i;
      return;
    }
  }

  pc -= 2; // Simulates waiting by executing the same instruction again
}

// LD DT, Vx
void Chip8::OP_Fx15() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Set delay timer = Vx
  delay = V[x];
}

// LD ST, Vx
void Chip8::OP_Fx18() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Set sound timer = Vx
  sound = V[x];
}

// ADD I, Vx
void Chip8::OP_Fx1E() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Set I = I + Vx

  // overflow flag
  V[0xF] = (index + V[x] > 0xFFF) ? 1 : 0;
  index += V[x];
}

// LD F, Vx
void Chip8::OP_Fx29() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Set I = location of sprite for digit Vx
  index = FONTSET_START_ADDRESS + (5 * V[x]);
}

// LD B, Vx
void Chip8::OP_Fx33() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Store BCD representation of Vx in memory locations I, I+1, and I+2

  uint8_t val = V[x];

  /// ones
  memory[index + 2] = val % 10;
  val /= 10;

  // tens
  memory[index + 1] = val % 10;
  val /= 10;

  // hundreds
  memory[index] = val % 10;
}

// LD [I], Vx
void Chip8::OP_Fx55() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Store registers V0 through Vx in memory starting at location I.
  for (uint8_t i = 0; i <= x; i += 1) {
    memory[index + i] = V[i];
  }
}

// LD Vx, [I]
void Chip8::OP_Fx65() {
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  // Read registers V0 through Vx from memory starting at location I
  for (uint8_t i = 0; i <= x; i += 1) {
    V[i] = memory[index + i];
  }
}
