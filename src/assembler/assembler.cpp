#include <cctype>
#include <cstdint>
#include <iomanip>
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
  for (const auto &tl : tkzr.get_token_lines()) {

    if (tl.empty())
      continue;

    if (tl.front().type == TokenType::LabelDef) {
      const std::string label =
          tl.front().text.substr(0, tl.front().text.size() - 1);

      if (label_table.find(label) != label_table.end()) {
        std::cerr << "error: duplicate label: " << label << "\n";
      } else {
        label_table[label] = PC;
      }
    }

    PC += 2;
  }
}

// ====== Checkers ======
bool Assembler::is_immediate_or_label(const Token &tk) {
  return tk.type == TokenType::Immediate || tk.type == TokenType::LabelRef;
}

// ====== Parsing helpers ======
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

  throw_invalid_instruction("LD", line);
  return 0x0;
}

uint16_t Assembler::assemble_instruction(std::vector<Token> line) {

  if (line.empty())
    return 0x0000;

  const std::string &mnemonic = line[0].text;

  // CLS
  if (mnemonic == "CLS")
    return 0x00E0;

  // RET
  if (mnemonic == "RET")
    return 0x00EE;

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

  throw_invalid_instruction(mnemonic.c_str(), line);
  return 0x0000;
}

Assembler::Assembler(std::string source_code) : tkzr(source_code) {
  run_first_pass(); //

  uint16_t PC = 0x200;
  std::vector<uint8_t> bytes;

  for (const auto &line : tkzr.get_token_lines()) {
    if (line.empty() || line.front().type == TokenType::LabelDef)
      continue;

    uint16_t opcode = assemble_instruction(line);

    bytes.push_back(((opcode & 0xFF00u) >> 8u));
    bytes.push_back((opcode & 0x00FFu));

    std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0')
              << std::uppercase << opcode << std::endl;

    PC += 2;
  }
}
