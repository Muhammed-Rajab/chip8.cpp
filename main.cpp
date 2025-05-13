#include "./include/chip8.hpp"
#include "include/chip8_disassembler.hpp"
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

  // const std::string filename = "./roms/test/ibm.ch8";
  // auto rom = LoadRomFromFile(filename);
  //
  // Chip8 cpu;
  // cpu.LoadFromArray(rom.data(), rom.size());
  //
  // while (true) {
  //   std::cout << "\033[2J\033[H";
  //   std::cout << cpu.DumpVideo();
  //   cpu.Cycle();
  //   std::this_thread::sleep_for(std::chrono::milliseconds(16));
  // }

  // std::vector<uint16_t> testOpcodes = {
  //     0x00E0, // CLS
  //     0x00EE, // RET
  //     0x1234, // JP 0x234
  //     0x2345, // CALL 0x345
  //     0x3A7F, // SE V[A], 0x7F
  //     0x4B20, // SNE V[B], 0x20
  //     0x5AB0, // SE V[A], V[B]
  //     0x6C0F, // LD V[C], 0x0F
  //     0x7D01, // ADD V[D], 0x01
  //     0x8AB0, // LD V[A], V[B]
  //     0x8AC1, // OR V[A], V[C]
  //     0x8AC2, // AND V[A], V[C]
  //     0x8AC3, // XOR V[A], V[C]
  //     0x8AC4, // ADD V[A], V[C]
  //     0x8AC5, // SUB V[A], V[C]
  //     0x8AC6, // SHR V[A]
  //     0x8AC7, // SUBN V[A], V[C]
  //     0x8ACE, // SHL V[A]
  //     0x9AB0, // SNE V[A], V[B]
  //     0xA123, // LD I, 0x123
  //     0xB456, // JP V0, 0x456
  //     0xC7FF, // RND V[7], 0xFF
  //     0xDAB5, // DRW V[A], V[B], 5
  //     0xEA9E, // SKP V[A]
  //     0xEAA1, // SKNP V[A]
  //     0xFA07, // LD V[A], DT
  //     0xFA0A, // LD V[A], K
  //     0xFA15, // LD DT, V[A]
  //     0xFA18, // LD ST, V[A]
  //     0xFA1E, // ADD I, V[A]
  //     0xFA29, // LD F, V[A]
  //     0xFA33, // LD B, V[A]
  //     0xFA55, // LD [I], V[A]
  //     0xFA65  // LD V[A], [I]
  // };
  //
  // for (auto opcode : testOpcodes) {
  //   std::string out = Disassembler::Decode(opcode);
  //   if (!out.empty())
  //     std::cout << out << std::endl;
  // }

  const std::string filename = "./roms/test/ibm.ch8";
  auto rom = LoadRomFromFile(filename);

  Disassembler dasm;
  std::cout << dasm.DecodeRomFromArray(rom, true);
  return EXIT_SUCCESS;
}
