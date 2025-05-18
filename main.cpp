#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "./include/assembler/assembler.hpp"
#include "./include/chip8.hpp"
#include "./include/disassembler/disassembler.hpp"

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

// ====== ROM display ======
void DisplayRomAsOpcode(const std::vector<uint8_t> &rom,
                        uint16_t start_address = 0x200) {
  if (rom.size() % 2 != 0) {
    std::cerr << "warning: ROM size is not aligned to 2 bytes. last byte will "
                 "be ignored.\n";
  }

  for (size_t i = 0; i + 1 < rom.size(); i += 2) {
    uint16_t opcode = (rom[i] << 8) | rom[i + 1];
    std::cout << "0x" << std::hex << std::uppercase << std::setw(4)
              << std::setfill('0') << opcode << "\n";
  }
}

int main(int argc, char *argv[]) {

  auto ibm = LoadRomFromFile("./roms/test/ibm.ch8");

  Chip8 cpu;

  // cpu.LoadFromArray(rom.data(), rom.size());
  cpu.LoadFromArray(ibm.data(), ibm.size());

  while (true) {
    std::cout << "\033[2J";
    std::cout << "\033[H";
    std::cout << cpu.DumpVideo();
    cpu.Cycle();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  return EXIT_SUCCESS;
}
