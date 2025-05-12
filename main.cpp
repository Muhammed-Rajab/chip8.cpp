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
  std::vector<uint8_t> rom = {
      0xA2, 0xF0, // LD I, 0x2F0
      0xFF, 0xFF  // HALT (FxFF — your custom instruction)
  };

  Chip8 cpu;
  cpu.LoadFromArray(rom.data(), rom.size());

  cpu.RunTillHalt();

  // std::cout << cpu.DumpRegisters();
  std::cout << cpu.DumpCPU();

  return EXIT_SUCCESS;
}
