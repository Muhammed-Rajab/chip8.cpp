#include "./include/chip8.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {

  std::vector<uint8_t> rom = {0x60, 0xFF};

  Chip8 cpu;
  cpu.LoadFromArray(rom.data(), rom.size());

  std::cout << cpu.DumpCPU();

  return EXIT_SUCCESS;
}
