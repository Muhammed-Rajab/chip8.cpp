#include "../../include/disassembler/disassembler.hpp"
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <vector>

std::string Disassembler::Decode(uint16_t opcode) {

  std::ostringstream out;

  uint8_t first_nibble = (opcode & 0xF000u) >> 12u;
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  uint8_t n = (opcode & 0x000Fu);
  uint8_t kk = (opcode & 0x00FFu);
  uint16_t nnn = (opcode & 0x0FFFu);

  switch (first_nibble) {
  case 0x0:
    if (opcode == 0x00E0)
      out << "CLS";
    else if (opcode == 0x00EE)
      out << "RET";
    else
      out << "SYS 0x" << std::hex << nnn;
    break;

  case 0x1:
    out << "JP 0x" << std::hex << nnn;
    break;

  case 0x2:
    out << "CALL 0x" << std::hex << nnn;
    break;

  case 0x3:
    out << "SE V" << std::hex << +x << ", 0x" << +kk;
    break;

  case 0x4:
    out << "SNE V" << std::hex << +x << ", 0x" << +kk;
    break;

  case 0x5:
    if (n == 0x0)
      out << "SE V" << std::hex << +x << ", V" << +y;
    else
      out << "??? (" << std::hex << std::uppercase << opcode << ")";
    break;

  case 0x6:
    out << "LD V" << std::hex << +x << ", 0x" << +kk;
    break;

  case 0x7:
    out << "ADD V" << std::hex << +x << ", 0x" << +kk;
    break;

  case 0x8:
    switch (n) {
    case 0x0:
      out << "LD V" << std::hex << +x << ", V" << +y;
      break;
    case 0x1:
      out << "OR V" << std::hex << +x << ", V" << +y;
      break;
    case 0x2:
      out << "AND V" << std::hex << +x << ", V" << +y;
      break;
    case 0x3:
      out << "XOR V" << std::hex << +x << ", V" << +y;
      break;
    case 0x4:
      out << "ADD V" << std::hex << +x << ", V" << +y;
      break;
    case 0x5:
      out << "SUB V" << std::hex << +x << ", V" << +y;
      break;
    case 0x6:
      out << "SHR V" << std::hex << +x;
      break;
    case 0x7:
      out << "SUBN V" << std::hex << +x << ", V" << +y;
      break;
    case 0xE:
      out << "SHL V" << std::hex << +x;
      break;
    default:
      out << "??? (" << std::hex << std::uppercase << opcode << ")";
      break;
    }
    break;

  case 0x9:
    if (n == 0x0)
      out << "SNE V" << std::hex << +x << ", V" << +y;
    else
      out << "??? (" << std::hex << std::uppercase << opcode << ")";
    break;

  case 0xA:
    out << "LD I, 0x" << std::hex << nnn;
    break;

  case 0xB:
    out << "JP V0, 0x" << std::hex << nnn;
    break;

  case 0xC:
    out << "RND V" << std::hex << +x << ", 0x" << +kk;
    break;

  case 0xD:
    out << "DRW V" << std::hex << +x << ", V" << +y << ", 0x" << +n;
    break;

  case 0xE:
    if (kk == 0x9E)
      out << "SKP V" << std::hex << +x;
    else if (kk == 0xA1)
      out << "SKNP V" << std::hex << +x;
    else
      out << "??? (" << std::hex << std::uppercase << opcode << ")";
    break;

  case 0xF:
    switch (kk) {
    case 0x07:
      out << "LD V" << std::hex << +x << ", DT";
      break;
    case 0x0A:
      out << "LD V" << std::hex << +x << ", K";
      break;
    case 0x15:
      out << "LD DT, V" << std::hex << +x;
      break;
    case 0x18:
      out << "LD ST, V" << std::hex << +x;
      break;
    case 0x1E:
      out << "ADD I, V" << std::hex << +x;
      break;
    case 0x29:
      out << "LD F, V" << std::hex << +x;
      break;
    case 0x33:
      out << "LD B, V" << std::hex << +x;
      break;
    case 0x55:
      out << "LD [I], V" << std::hex << +x;
      break;
    case 0x65:
      out << "LD V" << std::hex << +x << ", [I]";
      break;
    default:
      out << "??? (" << std::hex << std::uppercase << opcode << ")";
      break;
    }
    break;

  default:
    out << "??? (" << std::hex << std::uppercase << opcode << ")";
    break;
  }
  return out.str();
}

std::string Disassembler::DecodeRomFromArray(std::vector<uint8_t> rom,
                                             bool verbose) {
  std::ostringstream out;

  for (size_t i = 0; i + 1 < rom.size(); i += 2) {
    uint16_t opcode = (rom[i] << 8u) | rom[i + 1];

    if (verbose)
      out << std::setw(4) << std::setfill('0') << std::hex << i + 0x200 << ": "
          << std::setw(4) << opcode << "  " << Disassembler::Decode(opcode)
          << "\n";
    else
      out << Disassembler::Decode(opcode) << "\n";
  }

  return out.str();
}

std::vector<std::string>
Disassembler::DecodeRomFromArrayAsVector(std::vector<uint8_t> rom,
                                         bool verbose) {

  std::vector<std::string> lines;

  for (size_t i = 0; i + 1 < rom.size(); i += 2) {
    uint16_t opcode = (rom[i] << 8u) | rom[i + 1];

    std::ostringstream out;

    if (verbose)
      out << std::setw(4) << std::setfill('0') << std::hex << i + 0x200 << ": "
          << std::setw(4) << opcode << "  " << Disassembler::Decode(opcode)
          << "\n";
    else
      out << Disassembler::Decode(opcode) << "\n";

    lines.push_back(out.str());
  }

  return lines;
}
