#ifndef CHIP8_DISASSEMBLER_HPP
#define CHIP8_DISASSEMBLER_HPP

#include <cstdint>
#include <string>
#include <vector>

class Disassembler {

public:
  static std::string Decode(uint16_t opcode);
  static std::string DecodeRomFromArray(std::vector<uint8_t> rom,
                                        bool verbose = false);
  static std::vector<std::string>
  DecodeRomFromArrayAsVector(std::vector<uint8_t> rom, bool verbose = false);
};

#endif
