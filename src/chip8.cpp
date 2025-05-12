#include "../include/chip8.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>

// ====== Constructor ======
Chip8::Chip8() {
  // copy font to memory - (done once, and preserved)
  std::copy(fontset, fontset + FONTSET_SIZE, memory + FONTSET_START_ADDRESS);

  // setup decode tables
  for (size_t i = 0; i < 16; i += 1) {
    table[i] = &Chip8::OP_NULL;
  }

  table[0x1] = &Chip8::OP_1nnn;
  table[0x2] = &Chip8::OP_2nnn;
  table[0x3] = &Chip8::OP_3xkk;
  table[0x4] = &Chip8::OP_4xkk;
  table[0x5] = &Chip8::OP_5xy0;
  table[0x6] = &Chip8::OP_6xkk;
  table[0x7] = &Chip8::OP_7xkk;
}

// ====== Loaders ======
void Chip8::LoadFromArray(const uint8_t *rom, size_t size) {
  Reset();

  if (STARTING_ADDRESS + size > 4096) {
    throw std::runtime_error("rom too big");
  }

  for (size_t i = 0; i < size; i += 1) {
    memory[STARTING_ADDRESS + i] = rom[i];
  }
}

// ====== Reset CPU State ======
void Chip8::Reset() {

  std::fill(memory + STARTING_ADDRESS, memory + 4096, 0);

  std::fill(V, V + 16, 0);

  sp = {};
  pc = STARTING_ADDRESS;
  index = {};

  std::fill(stack, stack + 16, 0);

  delay = {};
  sound = {};

  std::fill(video, video + VIDEO_WIDTH * VIDEO_HEIGHT, 0);

  opcode = {};

  std::fill(keypad, keypad + 16, 0);
}

// ====== Cycle ======
void Chip8::Fetch() { opcode = (memory[pc] << 8u) | memory[pc + 1]; }

void Chip8::DecodeAndExecute() {
  // decode the opcode and execute the right instruction from table (future)
  (this->*(table[(opcode & 0xF000u) >> 12u]))();
}

void Chip8::Cycle() {

  // fetch opcode from memory
  Fetch();

  // increment program counter
  uint16_t prev_pc =
      pc; // might be useful for debugging or opcode behaviour tracking
  pc += 2;

  // decode execute current opcode
  DecodeAndExecute();

  // update delay register
  if (delay > 0) {
    delay -= 1;
  }

  // update sound register
  if (sound > 0) {
    sound -= 1;
  }
}

// ====== Debugging ======
std::string Chip8::DumpRegisters() const {

  std::ostringstream dump;

  dump << ":: Registers ::\n";
  for (size_t i = 0; i < 16; i++) {
    dump << "V[" << std::hex << i << "]: 0x" << std::setw(2)
         << std::setfill('0') << static_cast<int>(V[i]) << "  ";
    if ((i + 1) % 4 == 0)
      dump << "\n";
  }

  return dump.str();
}

std::string Chip8::DumpCPU() const {
  std::ostringstream dump;

  dump << "==== CPU STATE ====\n";
  dump << "PC: 0x" << std::hex << pc << "\n";
  dump << "Index: 0x" << std::hex << index << "\n";
  dump << "SP: 0x" << std::hex << static_cast<int>(sp) << "\n";
  dump << "Delay: " << std::dec << static_cast<int>(delay) << "\n";
  dump << "Sound: " << std::dec << static_cast<int>(sound) << "\n";
  dump << "Opcode: 0x" << std::hex << opcode << "\n";

  dump << "\n";
  dump << this->DumpRegisters();

  dump << "\n:: Stack ::\n";
  for (size_t i = 0; i < 16; i += 1) {
    dump << "[" << i << "]: 0x" << std::hex << stack[i] << "\n";
  }

  return dump.str();
}
