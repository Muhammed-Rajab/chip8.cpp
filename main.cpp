#include "./include/chip8.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {

  // 1nnn
  std::vector<uint8_t> rom = {0x12, 0xFF};

  // 6xkk
  // std::vector<uint8_t> rom = {0x60, 0xFF};

  // 7xkk
  // std::vector<uint8_t> rom = {0x71, 0x22};

  Chip8 cpu;
  cpu.LoadFromArray(rom.data(), rom.size());

  cpu.Cycle();

  // std::cout << cpu.DumpRegisters();
  std::cout << cpu.DumpCPU();

  return EXIT_SUCCESS;
}
