#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
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

  // CONSTRUCTOR
  Chip8() {

    // copy font to memory - (done once, and preserved)
    std::copy(fontset, fontset + FONTSET_SIZE, memory + FONTSET_START_ADDRESS);
  }

  // LOADERS
  void LoadFromArray(const uint8_t *rom, size_t size) {
    Reset();

    if (STARTING_ADDRESS + size > 4096) {
      throw std::runtime_error("rom too big");
    }

    for (size_t i = 0; i < size; i += 1) {
      memory[STARTING_ADDRESS + i] = rom[i];
    }
  }

  // RESET CPU STATE
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
};

#endif
