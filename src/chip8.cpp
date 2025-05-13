#include "../include/chip8.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>

// ====== Constructor ======
Chip8::Chip8() {
  // seeding
  srand(time(nullptr));

  // copy font to memory - (done once, and preserved)
  std::copy(fontset, fontset + FONTSET_SIZE, memory + FONTSET_START_ADDRESS);

  // setup decode tables
  for (size_t i = 0; i < 16; i += 1) {
    table[i] = &Chip8::OP_NULL;
  }

  table[0x0] = &Chip8::Table0;
  table[0x1] = &Chip8::OP_1nnn;
  table[0x2] = &Chip8::OP_2nnn;
  table[0x3] = &Chip8::OP_3xkk;
  table[0x4] = &Chip8::OP_4xkk;
  table[0x5] = &Chip8::OP_5xy0;
  table[0x6] = &Chip8::OP_6xkk;
  table[0x7] = &Chip8::OP_7xkk;
  table[0x8] = &Chip8::Table8;
  table[0x9] = &Chip8::OP_9xy0;
  table[0xA] = &Chip8::OP_Annn;
  table[0xB] = &Chip8::OP_Bnnn;
  table[0xC] = &Chip8::OP_Cxkk;
  table[0xD] = &Chip8::OP_Dxyn;
  table[0xE] = &Chip8::TableE;
  table[0xF] = &Chip8::TableF;

  // 0
  for (size_t i = 0; i < 0xF + 1; i += 1) {
    table0[i] = &Chip8::OP_NULL;
  }
  table0[0x0] = &Chip8::OP_00E0;
  table0[0xE] = &Chip8::OP_00EE;

  // 8
  for (size_t i = 0; i < 0xF + 1; i += 1) {
    table8[i] = &Chip8::OP_NULL;
  }

  table8[0x0] = &Chip8::OP_8xy0;
  table8[0x1] = &Chip8::OP_8xy1;
  table8[0x2] = &Chip8::OP_8xy2;
  table8[0x3] = &Chip8::OP_8xy3;
  table8[0x4] = &Chip8::OP_8xy4;
  table8[0x5] = &Chip8::OP_8xy5;
  table8[0x6] = &Chip8::OP_8xy6;
  table8[0x7] = &Chip8::OP_8xy7;
  table8[0xE] = &Chip8::OP_8xyE;

  // E
  for (size_t i = 0; i < 0xF + 1; i += 1) {
    tableE[i] = &Chip8::OP_NULL;
  }
  tableE[0xE] = &Chip8::OP_Ex9E;
  tableE[0x1] = &Chip8::OP_ExA1;

  // F
  for (size_t i = 0; i < 0xFF + 1; i += 1) {
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
  tableF[0xFF] = &Chip8::OP_FxFF; // HALT
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

  if (halted)
    return;

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

bool Chip8::RunTillHalt() {

  do {
    Cycle();
  } while (!halted);

  return true;
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

std::string Chip8::DumpVideo() const {
  std::ostringstream dump;

  for (size_t y = 0; y < VIDEO_HEIGHT; y += 1) {
    for (size_t x = 0; x < VIDEO_WIDTH; x += 1) {
      size_t index = y * VIDEO_WIDTH + x;
      dump << (video[index] ? "#" : ".");
    }
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

std::string Chip8::DumpMemoryTableHex(uint16_t start, uint16_t count) const {
  std::ostringstream dump;
  const int columns = 16;
  const uint16_t memSize = sizeof(memory);

  if (count == 0) {
    throw std::invalid_argument(
        "Memory dump count is zero. Nothing to display.");
  }

  if (start >= memSize) {
    throw std::invalid_argument("Start address is out of memory bounds.");
  }

  // Prevent overflow: cap count to fit within memory
  if (start + count > memSize || start + count < start) {
    count = memSize - start;
  }

  // Header row
  dump << "       ";
  for (int col = 0; col < columns; ++col) {
    dump << " " << std::setw(2) << std::setfill('0') << std::hex
         << std::uppercase << col;
  }
  dump << "\n";

  // Dump memory in 16-column layout
  for (uint16_t i = 0; i < count; i += columns) {
    dump << "0x" << std::hex << std::uppercase << std::setw(3)
         << std::setfill('0') << (start + i) << "  ";
    for (int j = 0; j < columns; ++j) {
      if (i + j < count) {
        dump << " " << std::setw(2) << std::setfill('0')
             << static_cast<int>(memory[start + i + j]);
      } else {
        dump << "   ";
      }
    }
    dump << "\n";
  }

  return dump.str();
}
