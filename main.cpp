#include "./include/chip8.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {

  // 1nnn
  // std::vector<uint8_t> rom = {0x12, 0xFF};

  // 2nnn
  // std::vector<uint8_t> rom = {0x22, 0xFF};

  // 3xkk
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0A, //
  //     0x30, 0x0A, //
  //     0x60, 0xFF  //
  // };

  // 4xkk
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0A, // 600A: Set V0 = 0x0A
  //     0x40, 0x0B, // 400B: Skip next if V0 != 0x0B (true, so skip)
  //     0x60, 0xFF  // 60FF: Would set V0 = 0xFF, but should be skipped
  // };

  // 5xy0
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0A, // V0 = 0x0A
  //     0x61, 0x0A, // V1 = 0x0A
  //     0x50, 0x10, // Skip if V0 == V1
  //     0x60, 0xFF  // Should be skipped
  // };
  //
  // 6xkk
  // std::vector<uint8_t> rom = {0x60, 0xFF};

  // 7xkk
  // std::vector<uint8_t> rom = {0x71, 0x22};

  // 9xy0
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0A, // V0 = 0x0A
  //     0x61, 0x0B, // V1 = 0x0B
  //     0x90, 0x10, // SNE V0, V1 (skip next if V0 != V1)
  //     0x62, 0xFF, // V2 = 0xFF (should be skipped)
  //     0x63, 0x33  // V3 = 0x33
  // };
  //

  // 0xFxFF
  // std::vector<uint8_t> rom = {
  //     0x60, 0x22, //
  //     0xFF, 0xFF, //
  //     0x60, 0x33, // this shouldn't execute, V[0] should be 0x22
  // };

  // 0xAnnn
  // std::vector<uint8_t> rom = {
  //     0xA2, 0xF0, // LD I, 0x2F0
  //     0xFF, 0xFF  // HALT (FxFF — your custom instruction)
  // };

  // 0xBnnn
  // std::vector<uint8_t> rom = {
  //     0x60, 0x05, // LD V0, 0x05
  //     0xB2, 0x01, // JP V0, 0x200 → should jump to 0x205
  //     0x60, 0xFF, // [0x204] should be skipped
  //     0x60, 0x0A, // [0x205] V0 = 0x0A
  //     0xFF, 0xFF  // HALT
  // };

  // 0xCxkk
  // std::vector<uint8_t> rom = {
  //     0xC0, 0x0F, // RND V0, 0x0F (V0 = rand() & 0x0F)
  //     0xFF, 0xFF  // HALT
  // };

  // 0xDxyn
  // std::vector<uint8_t> rom = {
  //     0x60, 0x00,                   //
  //     0x61, 0x00,                   //
  //     0xA2, 0x0A,                   //
  //     0xD0, 0x15,                   //
  //     0xFF, 0xFF,                   //
  //     0xF0, 0x10, 0xF0, 0x80, 0xF0, //
  // };

  // std::vector<uint8_t> rom = {};

  // 0x8xy0
  // std::vector<uint8_t> rom = {
  //     0x61, 0x0A, // LD V1, 0x0A
  //     0x80, 0x10, // LD V0, V1
  //     0xFF, 0xFF  // HALT
  // };

  // 0x8xy1
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0F, // LD V0, 0x0F
  //     0x61, 0xF0, // LD V1, 0xF0
  //     0x80, 0x11, // OR V0, V1 (V0 = 0xFF)
  //     0xFF, 0xFF  // HALT
  // };

  // 0x8xy2
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0F, // LD V0, 0x0F
  //     0x61, 0xF0, // LD V1, 0xF0
  //     0x80, 0x12, // AND V0, V1 (V0 = 0x00)
  //     0xFF, 0xFF  // HALT
  // };
  //
  // 0x8xy3
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0F, // LD V0, 0x0F
  //     0x61, 0xF0, // LD V1, 0xF0
  //     0x80, 0x13, // XOR V0, V1 (V0 = 0xFF)
  //     0xFF, 0xFF  // HALT
  // };
  //
  // 0x8xy4
  // std::vector<uint8_t> rom = {
  //     0x60, 0xF0, // LD V0, 0xF0
  //     0x61, 0x20, // LD V1, 0x20
  //     0x80, 0x14, // ADD V0, V1 (V0 = 0x10, VF = 1)
  //     0xFF, 0xFF  // HALT
  // };
  //
  // 0x8xy5
  // std::vector<uint8_t> rom = {
  //     0x60, 0x30, // LD V0, 0x30
  //     0x61, 0x10, // LD V1, 0x10
  //     0x80, 0x15, // SUB V0, V1 (V0 = 0x20, VF = 1)
  //     0xFF, 0xFF  // HALT
  // };
  //
  // 0x8xy6
  // std::vector<uint8_t> rom = {
  //     0x60, 0x05, // LD V0, 0x05
  //     0x80, 0x16, // SHR V0 (V0 = 0x02, VF = 1)
  //     0xFF, 0xFF  // HALT
  // };
  //
  // 0x8xy7
  // std::vector<uint8_t> rom = {
  //     0x60, 0x10, // LD V0, 0x10
  //     0x61, 0x30, // LD V1, 0x30
  //     0x80, 0x17, // SUBN V0, V1 (V0 = 0x20, VF = 1)
  //     0xFF, 0xFF  // HALT
  // };
  //
  // 0x8xyE
  // std::vector<uint8_t> rom = {
  //     0x60, 0x81, // LD V0, 0x81
  //     0x80, 0x1E, // SHL V0 (V0 = 0x02, VF = 1)
  //     0xFF, 0xFF  // HALT
  // };
  //
  //

  // E Family
  // std::vector<uint8_t> rom = {
  //     0x60, 0x05, // LD V0, 0x05
  //     0xE0, 0x9E, // SKP V0 (should skip next if key 5 is pressed)
  //     0x61, 0x01, // LD V1, 0x01
  //     0xE0, 0xA1, // SKPN V0 (should skip next if key 5 is not pressed)
  //     0x61, 0x02, // LD V1, 0x02 (this should run if skipped)
  //     0xFF, 0xFF  // HALT
  // };

  // F Family

  // 0xFx07 and 0xFx15
  // std::vector<uint8_t> rom = {
  //     0x60, 0x22, // V0 = 0x22
  //     0xF0, 0x15, // DT = V0; DT = 0X21?
  //     0xF1, 0x07, // V1 = DT; V1 = 0x20?
  //     0xFF, 0xFF  // HALT
  // };

  // 0xFx0A
  // std::vector<uint8_t> rom = {
  //     0xF0, 0x0A, //
  //     0xFF, 0xFF  // HALT
  // };
  //

  // 0xFx18
  // std::vector<uint8_t> rom = {
  //     0x60, 0x22, //
  //     0xF0, 0x18, //
  //     0xFF, 0xFF  // HALT
  // };

  // 0xFx1E
  // std::vector<uint8_t> rom = {
  //     0x60, 0x21, // V0 = 0x21
  //     0xA0, 0x10, // I = 0x010
  //     0xF0, 0x1E, // I += V0 => I = 0x31
  //     0xFF, 0xFF  // HALT
  // };

  // 0xFx29
  // std::vector<uint8_t> rom = {
  //     0x60,
  //     0x03, // V0 = 0x03 (digit 3)
  //     0xF0,
  //     0x29, // I = sprite address for 3 → I = 0x5F (if font starts at 0x50)
  //     0xFF,
  //     0xFF // HALT
  // };

  // 0xFx33
  // std::vector<uint8_t> rom = {
  //     0x60, 0x7B, // V0 = 123
  //     0xA3, 0x00, // I = 0x300
  //     0xF0, 0x33, // Store BCD of 123 at I
  //     0xFF, 0xFF  // HALT
  // };

  // 0xFx55
  // std::vector<uint8_t> rom = {
  //     0x60, 0x0A, // V0 = 10
  //     0x61, 0x0B, // V1 = 11
  //     0xA3, 0x00, // I = 0x300
  //     0xF1, 0x55, // Store V0 and V1 into M[0x300], M[0x301]
  //     0xFF, 0xFF  // HALT
  // };

  // 0xFx65
  // Assume memory at 0x300 has: 0x0C, 0x0D
  std::vector<uint8_t> rom = {
      0xA3, 0x00, // I = 0x300
      0xF1, 0x65, // Load V0, V1 from M[0x300], M[0x301]
      0xFF, 0xFF  // HALT
  };

  Chip8 cpu;
  cpu.LoadFromArray(rom.data(), rom.size());

  cpu.memory[0x300] = 0x0C;
  cpu.memory[0x301] = 0x0D;

  cpu.RunTillHalt();

  // std::cout << cpu.DumpRegisters();
  std::cout << cpu.DumpCPU();
  // std::cout << cpu.DumpVideo();
  std::cout << cpu.DumpMemoryTableHex(0x300, 0x20);

  return EXIT_SUCCESS;
}
