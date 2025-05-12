#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>

class Chip8 {
public:
  // RAM
  uint8_t memory[4096];

  // meta
  static constexpr uint16_t STARTING_ADDRESS = 0x200;

  // register (Vf is flag)
  uint8_t V[16];

  // pseudo registers
  uint8_t sp{};
  uint16_t pc = STARTING_ADDRESS;
  uint16_t index{};

  // stack
  uint16_t stack[16]{};

  // misc. registers
  uint8_t delay{};
  uint8_t sound{};

  // video
  static constexpr uint8_t VIDEO_WIDTH = 64;
  static constexpr uint8_t VIDEO_HEIGHT = 32;
  uint8_t video[VIDEO_WIDTH * VIDEO_HEIGHT];

  // opcode - current
  uint16_t opcode;

  // keypad
  uint8_t keypad[16]{};

  // fonts
  static constexpr uint8_t FONTSET_SIZE = 80;
  static constexpr uint16_t FONTSET_START_ADDRESS = 0x50;
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

  // ====== Constructor ======
  Chip8() {

    // copy font to memory - (done once, and preserved)
    std::copy(fontset, fontset + FONTSET_SIZE, memory + FONTSET_START_ADDRESS);
  }

  // ====== Loaders ======
  void LoadFromArray(const uint8_t *rom, size_t size) {
    Reset();

    if (STARTING_ADDRESS + size > 4096) {
      throw std::runtime_error("rom too big");
    }

    for (size_t i = 0; i < size; i += 1) {
      memory[STARTING_ADDRESS + i] = rom[i];
    }
  }

  // ====== Reset CPU State ======
  void Reset() {

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
  void Fetch() { opcode = (memory[pc] << 8u) | memory[pc + 1]; }

  void DecodeAndExecute() {
    // decode the opcode and execute the right instruction from table (future)
  }

  void Cycle() {

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
  std::string DumpCPU() const {
    std::ostringstream dump;

    dump << "==== CPU STATE ====\n";
    dump << "PC: 0x" << std::hex << pc << "\n";
    dump << "Index: 0x" << std::hex << index << "\n";
    dump << "SP: 0x" << std::hex << static_cast<int>(sp) << "\n";
    dump << "Delay: " << std::dec << static_cast<int>(delay) << "\n";
    dump << "Sound: " << std::dec << static_cast<int>(sound) << "\n";
    dump << "Opcode: 0x" << std::hex << opcode << "\n";

    dump << "\n:: Registers ::\n";
    for (size_t i = 0; i < 16; i++) {
      dump << "V[" << std::hex << i << "]: 0x" << std::setw(2)
           << std::setfill('0') << static_cast<int>(V[i]) << "  ";
      if ((i + 1) % 4 == 0)
        dump << "\n";
    }

    dump << "\n:: Stack ::\n";
    for (size_t i = 0; i < 16; i += 1) {
      dump << "[" << i << "]: 0x" << std::hex << stack[i] << "\n";
    }

    return dump.str();
  }
};

#endif
