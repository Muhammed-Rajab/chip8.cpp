#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "./include/assembler/assembler.hpp"
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

int main(int argc, char *argv[]) {

  // auto rom = LoadRomFromFile("./roms/test/ibm.ch8");
  // Disassembler dasm;
  // std::cout << dasm.DecodeRomFromArray(rom, true);

  std::string source = R"(; Test program for tokenizer coverage

Start:
    CLS
    RET
    JP Start
    JP V0, Start
    JP Start
    JP V0, 0x222
    JP 0x333
    CALL Start
    CALL 0x200
    SE V0, 0x22
    SE V0, V1
    SNE V0, 0x22
    SNE V0, V1
    ADD V0, 0x22
    ADD V0, V1
    ADD I, VF
    AND V1, V2
    OR V2, V3
    XOR V3, V4
    SUB V3, V4
    SUBN V3, V4
    LD V3, 0x22
    LD V0, V1
)";
  Assembler chip8_asm(source);

  return EXIT_SUCCESS;
}
