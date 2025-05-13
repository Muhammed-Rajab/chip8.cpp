#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <cstddef>
#include <cstdint>
#include <string>

class Chip8 {
public:
  // state
  bool halted = false;

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

  // member function pointer
  typedef void (Chip8::*Chip8OP)();

  // decode tables
  Chip8OP table[0xF + 1];
  Chip8OP table8[0xF + 1]; // only requires 0xE + 1, but going OxF + 1 cause
                           // it's safer and future proof
  Chip8OP tableF[0xFF + 1];

  // ====== Constructor ======
  Chip8();

  // ====== Loaders ======
  void LoadFromArray(const uint8_t *rom, size_t size);

  // ====== Reset CPU State ======
  void Reset();

  // ====== Cycle ======
  void Fetch();
  void DecodeAndExecute();
  void Cycle();

  // ====== Debugging ======
  bool RunTillHalt();
  std::string DumpCPU() const;
  std::string DumpVideo() const;
  std::string DumpRegisters() const;

  // ====== Opcode tables ======
  void TableF() { (this->*(tableF[(opcode & 0x00FFu)]))(); }
  void Table8() { (this->*(table8[(opcode & 0x000Fu)]))(); }

  // ====== Opcodes ======
  void OP_NULL();
  void OP_FxFF(); // special halt instruction

  void OP_1nnn();
  void OP_2nnn();
  void OP_3xkk();
  void OP_4xkk();
  void OP_5xy0();
  void OP_6xkk();
  void OP_7xkk();

  void OP_8xy0();
  void OP_8xy1();
  void OP_8xy2();
  void OP_8xy3();
  void OP_8xy4();
  void OP_8xy5();
  void OP_8xy6();
  void OP_8xy7();
  void OP_8xyE();

  void OP_9xy0();

  void OP_Annn();
  void OP_Bnnn();
  void OP_Cxkk();
  void OP_Dxyn();
};

#endif
