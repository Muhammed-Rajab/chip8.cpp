#include "./include/chip8.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

// ====== ROM Loader ======
std::vector<uint8_t> LoadRomFromFile(const std::string &filename) {
  std::ifstream file(filename,
                     std::ios::binary | std::ios::ate); // ate = seek to end

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open ROM file: " + filename);
  }

  std::streamsize size = file.tellg(); // get size of file
  file.seekg(0, std::ios::beg);        // rewind to beginning

  std::vector<uint8_t> rom(size);
  if (!file.read(reinterpret_cast<char *>(rom.data()), size)) {
    throw std::runtime_error("Failed to read ROM file: " + filename);
  }

  return rom;
}

int main(int argc, char *argv[]) {

  const std::string filename = "./roms/test/ibm.ch8";
  auto rom = LoadRomFromFile(filename);

  Chip8 cpu;
  cpu.LoadFromArray(rom.data(), rom.size());

  while (true) {
    std::cout << "\033[2J\033[H";
    std::cout << cpu.DumpVideo();
    cpu.Cycle();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  return EXIT_SUCCESS;
}
