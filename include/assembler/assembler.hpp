#ifndef ASSEMBLER_4_CHIP8_HPP
#define ASSEMBLER_4_CHIP8_HPP

#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "tokenizer.hpp"

class Assembler {

private:
  Tokenizer tkzr;
  std::unordered_map<std::string, uint16_t> label_table;

  // ====== Stages ======
  void run_first_pass();

  // ====== Checkers ======
  bool is_immediate_or_label(const Token &tk);

  // ====== Parsing helpers ======
  uint16_t parse_immediate(const std::string &s);
  uint8_t parse_register(const std::string &s);
  uint16_t resolve_immediate_and_label(const Token &tk);
  void throw_invalid_instruction(const char *MNEMONIC,
                                 const std::vector<Token> &line) const;

  // ====== Instruction parsers ======
  uint16_t parse_JP(const std::vector<Token> &line);
  uint16_t parse_CALL(const std::vector<Token> &line);
  uint16_t parse_SE(std::vector<Token> &line);
  uint16_t parse_SNE(std::vector<Token> &line);
  uint16_t parse_ADD(std::vector<Token> &line);

  uint16_t parse_AND(std::vector<Token> &line);
  uint16_t parse_OR(std::vector<Token> &line);
  uint16_t parse_XOR(std::vector<Token> &line);

  uint16_t parse_SUB(std::vector<Token> &line);
  uint16_t parse_SUBN(std::vector<Token> &line);

  uint16_t parse_LD(std::vector<Token> &line);

  uint16_t assemble_instruction(std::vector<Token> line);

public:
  Assembler(std::string source_code);
};

#endif
