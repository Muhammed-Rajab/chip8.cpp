#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../include/assembler/assembler.hpp"

// ====== Stages ======
void Assembler::run_first_pass() {
  uint16_t PC = 0x200;
  for (const auto &line : tkzr.get_token_lines()) {

    if (line.empty())
      continue;

    size_t start = 0;

    if (line.front().type == TokenType::LabelDef) {
      const std::string label =
          line.front().text.substr(0, line.front().text.size() - 1);

      if (label_table.find(label) != label_table.end()) {
        std::cerr << "error: duplicate label: " << label << "\n";
      } else {
        label_table[label] = PC;
      }

      start = 1;
      if (line.size() == 1)
        continue; // only label
    }

    if (line[start].type == TokenType::ByteDirective) {
      size_t byte_count = 0;
      for (const auto &tk : line) {
        if (tk.type == TokenType::Immediate)
          byte_count += 1;
      }

      PC += byte_count;
    } else {

      PC += 2;
    }
  }
}

// ====== Checkers ======
bool Assembler::is_immediate_or_label(const Token &tk) {
  return tk.type == TokenType::Immediate || tk.type == TokenType::LabelRef;
}

// ====== Parsing helpers ======
void print_token_line(const std::vector<Token> &line) {
  for (const auto &token : line) {
    std::cout << token.as_string();
  }
  std::cout << std::endl;
}

void Assembler::throw_invalid_instruction(
    const char *MNEMONIC, const std::vector<Token> &line) const {
  std::ostringstream oss;
  oss << "invalid " << MNEMONIC << " instruction: ";
  for (const auto &tk : line)
    oss << tk.text << ' ';
  throw std::runtime_error(oss.str());
}

uint16_t Assembler::parse_immediate(const std::string &s) {
  // hex
  if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0)
    return std::stoul(s, nullptr, 16);
  // decimal
  return std::stoul(s, nullptr, 10);
}

uint8_t Assembler::parse_register(const std::string &s) {
  if (s.size() != 2 || s[0] != 'V')
    throw std::runtime_error("invalid register name: " + s);

  char reg = std::toupper(s[1]);

  if (reg >= '0' && reg <= '9')
    return reg - '0';
  if (reg >= 'A' && reg <= 'F')
    return 10 + (reg - 'A');

  throw std::runtime_error("invalid register name: " + s);
}

uint16_t Assembler::resolve_immediate_and_label(const Token &tk) {
  if (tk.type == TokenType::Immediate)
    return parse_immediate(tk.text);
  if (tk.type == TokenType::LabelRef) {
    if (label_table.count(tk.text))
      return label_table[tk.text];
    else {
      std::ostringstream oss;
      oss << "error: unknown label: " << tk.text;
      throw std::runtime_error(oss.str());
    }
  }
  return 0;
}

// ====== Instruction parsers ======

uint16_t Assembler::parse_JP(const std::vector<Token> &line) {
  // ! nnn could also be a label

  // 1nnn - JP addr
  if (line.size() == 2 && is_immediate_or_label(line[1])) {
    uint16_t addr = resolve_immediate_and_label(line[1]);
    return 0x1000 | (addr & 0x0FFF);
  }

  // Bnnn - JP V0, addr
  if (line.size() == 4 && line[1].type == TokenType::Register &&
      line[1].text == "V0" && line[2].type == TokenType::Comma &&
      is_immediate_or_label(line[3])) {
    uint16_t addr = resolve_immediate_and_label(line[3]);
    return 0xB000 | (addr & 0x0FFF);
  }

  throw_invalid_instruction("JP", line);
  return 0x0;
}

uint16_t Assembler::parse_CALL(const std::vector<Token> &line) {
  // ! nnn could also be a label

  // 2nnn - CALL addr
  if (line.size() == 2 && is_immediate_or_label(line[1])) {
    uint16_t addr = resolve_immediate_and_label(line[1]);
    return 0x2000 | (addr & 0x0FFF);
  }

  throw_invalid_instruction("CALL", line);
  return 0x0;
}

uint16_t Assembler::parse_SE(std::vector<Token> &line) {

  // 3xkk - SE Vx, byte
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Immediate) {

    uint8_t x = parse_register(line[1].text);
    uint16_t kk = parse_immediate(line[3].text);
    if (kk > 0xFF)
      throw std::runtime_error("immediate value out of range for a byte");

    return (0x3000 | (x << 8u)) | kk;
  }

  // 5xy0 - SE Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {

    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x5000 | (x << 8u)) | (y << 4u);
  }

  throw_invalid_instruction("SE", line);
  return 0x0;
}

uint16_t Assembler::parse_SNE(std::vector<Token> &line) {

  // 4xkk - SNE Vx, byte
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Immediate) {

    uint8_t x = parse_register(line[1].text);
    uint16_t kk = parse_immediate(line[3].text);
    if (kk > 0xFF)
      throw std::runtime_error("immediate value out of range for a byte");

    return (0x4000 | (x << 8u)) | kk;
  }

  // 9xy0 - SNE Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {

    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x9000 | (x << 8u)) | (y << 4u);
  }

  throw_invalid_instruction("SNE", line);
  return 0x0;
}

uint16_t Assembler::parse_ADD(std::vector<Token> &line) {

  // 7xkk - ADD Vx, byte
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Immediate) {

    uint8_t x = parse_register(line[1].text);
    uint16_t kk = parse_immediate(line[3].text);
    if (kk > 0xFF)
      throw std::runtime_error("immediate value out of range for a byte");

    return (0x7000 | (x << 8u)) | kk;
  }

  // 8xy4 - ADD Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {

    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8004 | (x << 8u)) | (y << 4u);
  }

  // Fx1E - ADD I, Vx
  if (line[1].type == TokenType::SpecialRegister && line[1].text == "I" &&
      line[2].type == TokenType::Comma && line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[3].text);

    return (0xF01E | (x << 8u));
  }

  throw_invalid_instruction("ADD", line);
  return 0x0;
}

uint16_t Assembler::parse_AND(std::vector<Token> &line) {
  if (line.size() != 4)
    throw_invalid_instruction("AND", line);

  // 8xy2 - AND Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8002) | (x << 8u) | (y << 4u);
  }

  throw_invalid_instruction("AND", line);
  return 0x0000;
}

uint16_t Assembler::parse_OR(std::vector<Token> &line) {
  if (line.size() != 4)
    throw_invalid_instruction("OR", line);

  // 8xy1 - OR Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8001) | (x << 8u) | (y << 4u);
  }

  throw_invalid_instruction("OR", line);
  return 0x0000;
}

uint16_t Assembler::parse_XOR(std::vector<Token> &line) {
  if (line.size() != 4)
    throw_invalid_instruction("XOR", line);

  // 8xy3 - XOR Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8003) | (x << 8u) | (y << 4u);
  }

  throw_invalid_instruction("XOR", line);
  return 0x0000;
}

uint16_t Assembler::parse_SUB(std::vector<Token> &line) {
  if (line.size() != 4)
    throw_invalid_instruction("SUB", line);

  // 8xy5 - SUB Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8005) | (x << 8u) | (y << 4u);
  }

  throw_invalid_instruction("SUB", line);
  return 0x0000;
}

uint16_t Assembler::parse_SUBN(std::vector<Token> &line) {
  if (line.size() != 4)
    throw_invalid_instruction("SUBN", line);

  // 8xy7 - SUBN Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8007) | (x << 8u) | (y << 4u);
  }

  throw_invalid_instruction("SUBN", line);
  return 0x0000;
}

uint16_t Assembler::parse_LD(std::vector<Token> &line) {
  if (line.size() != 4)
    throw_invalid_instruction("LD", line);

  // 6xkk - LD Vx, byte
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Immediate) {

    uint8_t x = parse_register(line[1].text);
    uint16_t kk = parse_immediate(line[3].text);
    if (kk > 0xFF)
      throw std::runtime_error("immediate value out of range for a byte");

    return (0x6000 | (x << 8u)) | kk;
  }

  // 8xy0 - LD Vx, Vy
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register) {

    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);

    return (0x8000 | (x << 8u)) | (y << 4u);
  }

  // Annn - LD I, addr
  if (line[1].type == TokenType::SpecialRegister && line[1].text == "I" &&
      line[2].type == TokenType::Comma && is_immediate_or_label(line[3])) {
    uint16_t addr = resolve_immediate_and_label(line[3]);

    if (addr > 0xFFF)
      throw std::runtime_error("immediate value out of range ( <= 0xFFF)");

    return (0xA000 | addr);
  }

  // Fx55 - LD [I], Vx
  if (line[1].type == TokenType::MemoryDereference &&
      line[2].type == TokenType::Comma && line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[3].text);
    return (0xF055 | (x << 8u));
  }

  // Fx65 - LD Vx, [I]
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::MemoryDereference) {
    uint8_t x = parse_register(line[1].text);
    return (0xF065 | (x << 8u));
  }

  // Fx07 - LD Vx, DT
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::SpecialRegister && line[3].text == "DT") {
    uint8_t x = parse_register(line[1].text);
    return (0xF007 | (x << 8u));
  }

  // Fx15 - LD DT, Vx
  if (line[1].type == TokenType::SpecialRegister && line[1].text == "DT" &&
      line[2].type == TokenType::Comma && line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[3].text);
    return (0xF015 | (x << 8u));
  }

  // Fx18 - LD ST, Vx
  if (line[1].type == TokenType::SpecialRegister && line[1].text == "ST" &&
      line[2].type == TokenType::Comma && line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[3].text);
    return (0xF018 | (x << 8u));
  }

  // Fx29 - LD F, Vx
  if (line[1].type == TokenType::SpecialMnemonic && line[1].text == "F" &&
      line[2].type == TokenType::Comma && line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[3].text);
    return (0xF029 | (x << 8u));
  }

  // Fx33 - LD B, Vx
  if (line[1].type == TokenType::SpecialMnemonic && line[1].text == "B" &&
      line[2].type == TokenType::Comma && line[3].type == TokenType::Register) {
    uint8_t x = parse_register(line[3].text);
    return (0xF033 | (x << 8u));
  }

  // Fx0A - LD Vx, K
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::SpecialMnemonic && line[3].text == "K") {
    uint8_t x = parse_register(line[1].text);
    return (0xF00A | (x << 8u));
  }

  throw_invalid_instruction("LD", line);
  return 0x0;
}

uint16_t Assembler::parse_RND(std::vector<Token> &line) {

  // Cxkk - RND Vx, byte
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Immediate) {
    uint8_t x = parse_register(line[1].text);
    uint16_t kk = parse_immediate(line[3].text);
    if (kk > 0xFF)
      throw std::runtime_error("immediate value out of range for a byte");

    return (0xC000 | (x << 8u)) | kk;
  }
  throw_invalid_instruction("RND", line);
  return 0x0;
}

uint16_t Assembler::parse_SKP(std::vector<Token> &line) {
  // Ex9E - SKP Vx
  if (line[1].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    return (0xE09E | (x << 8u));
  }

  throw_invalid_instruction("SKP", line);
  return 0x0;
}

uint16_t Assembler::parse_SKNP(std::vector<Token> &line) {
  // ExA1 - SKNP Vx
  if (line[1].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    return (0xE0A1 | (x << 8u));
  }

  throw_invalid_instruction("SKNP", line);
  return 0x0;
}

uint16_t Assembler::parse_DRW(std::vector<Token> &line) {
  // Dxyn - DRW Vx, Vy, nibble
  if (line[1].type == TokenType::Register && line[2].type == TokenType::Comma &&
      line[3].type == TokenType::Register && line[4].type == TokenType::Comma &&
      line[5].type == TokenType::Immediate) {
    uint8_t x = parse_register(line[1].text);
    uint8_t y = parse_register(line[3].text);
    uint16_t n = parse_immediate(line[5].text);

    if (n > 0xF)
      throw std::runtime_error("immediate value out of range for a nibble");

    return (0xD000 | (x << 8u) | (y << 4u) | (n));
  }
  throw_invalid_instruction("DRW", line);
  return 0x0;
}

uint16_t Assembler::parse_SHR(std::vector<Token> &line) {
  // 8xy6 - SHR Vx
  if (line[1].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    return (0x8006 | (x << 8u));
  }
  throw_invalid_instruction("SHR", line);
  return 0x0;
}

uint16_t Assembler::parse_SHL(std::vector<Token> &line) {
  // 8xyE - SHL Vx {, Vy}
  if (line[1].type == TokenType::Register) {
    uint8_t x = parse_register(line[1].text);
    return (0x800E | (x << 8u));
  }
  throw_invalid_instruction("SHL", line);
  return 0x0;
}

uint16_t Assembler::parse_CLS(std::vector<Token> &line) { return 0x00E0; }

uint16_t Assembler::parse_RET(std::vector<Token> &line) { return 0x00EE; }

uint16_t Assembler::assemble_instruction(std::vector<Token> line) {

  if (line.empty())
    return 0x0000;

  std::string mnemonic = line[0].text;
  std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

  // CLS
  if (mnemonic == "CLS")
    return parse_CLS(line);

  // RET
  if (mnemonic == "RET")
    return parse_RET(line);

  // JP
  if (mnemonic == "JP") {
    return parse_JP(line);
  }

  // CALL
  if (mnemonic == "CALL") {
    return parse_CALL(line);
  }

  // SE
  if (mnemonic == "SE") {
    return parse_SE(line);
  }

  // SNE
  if (mnemonic == "SNE") {
    return parse_SNE(line);
  }

  // ADD
  if (mnemonic == "ADD") {
    return parse_ADD(line);
  }

  // AND
  if (mnemonic == "AND") {
    return parse_AND(line);
  }

  // OR
  if (mnemonic == "OR") {
    return parse_OR(line);
  }

  // XOR
  if (mnemonic == "XOR") {
    return parse_XOR(line);
  }

  // SUB
  if (mnemonic == "SUB") {
    return parse_SUB(line);
  }

  // SUBN
  if (mnemonic == "SUBN") {
    return parse_SUBN(line);
  }

  // LD
  if (mnemonic == "LD") {
    return parse_LD(line);
  }

  // RND
  if (mnemonic == "RND") {
    return parse_RND(line);
  }

  // SKP
  if (mnemonic == "SKP") {
    return parse_SKP(line);
  }

  // SKPN
  if (mnemonic == "SKNP") {
    return parse_SKNP(line);
  }

  // DRW
  if (mnemonic == "DRW") {
    return parse_DRW(line);
  }

  // SHR
  if (mnemonic == "SHR") {
    return parse_SHR(line);
  }

  // SHL
  if (mnemonic == "SHL") {
    return parse_SHL(line);
  }

  throw_invalid_instruction(mnemonic.c_str(), line);
  return 0x0000;
}

Assembler::Assembler(std::string source_code) : tkzr(source_code) {

  // ====== Tokenizer Tester ======

  // uint16_t PC = 0x200;
  // for (const auto &line : tkzr.get_token_lines()) {
  //
  //   if (line.empty())
  //     continue;
  //
  //   std::cout << "0x" << std::hex << PC << ": ";
  //   for (auto &tk : line) {
  //     std::cout << tk.as_string() << " ";
  //   }
  //   std::cout << std::endl;
  //   PC += 2;
  // }

  run_first_pass(); //

  uint16_t PC = 0x200;

  for (const auto &line : tkzr.get_token_lines()) {
    if (line.empty())
      continue;

    size_t start = 0;
    if (line.front().type == TokenType::LabelDef) {
      start = 1;
      if (line.size() == 1)
        continue;
    }

    if (line[start].type == TokenType::ByteDirective) {
      for (size_t i = start + 1; i < line.size(); i += 1) {
        if (line[i].type == TokenType::Immediate) {
          uint16_t val = parse_immediate(line[i].text);

          if (val > 0xFF)
            throw std::runtime_error("immediate value out of range for a byte");
          bytes.push_back(val);
          PC += 1;
        }
      }
    } else {
      uint16_t opcode = assemble_instruction(line);

      bytes.push_back(((opcode & 0xFF00u) >> 8u));
      bytes.push_back((opcode & 0x00FFu));

      // std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0')
      //           << std::uppercase << opcode << std::endl;

      PC += 2;
    }
  }
}

std::vector<uint8_t> Assembler::GetBytes() const { return bytes; }

void Assembler::WriteToFile(std::string path) const {
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    throw std::runtime_error("failed to open file for writing: " + path);
  }

  file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
  if (!file) {
    throw std::runtime_error("failed to write data to file: " + path);
  }
}

Assembler Assembler::FromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file) {
    throw std::runtime_error("failed to open source file: " + filename);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return Assembler(buffer.str());
}
