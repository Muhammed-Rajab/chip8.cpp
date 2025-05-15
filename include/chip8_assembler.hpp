#ifndef CHIP8_ASSEMBLER_HPP
#define CHIP8_ASSEMBLER_HPP

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class TokenType {
  LabelDef,
  Mnemonic,
  Register,
  Immediate,
  LabelRef,
  Comma,
  SpecialRegister,
  MemoryDereference,
  Unknown
};

struct Token {
  TokenType type;
  std::string text;

  std::string as_string() {

    std::string type_as_string;

    switch (type) {
    case TokenType::LabelDef:
      type_as_string = "LabelDef";
      break;
    case TokenType::Mnemonic:
      type_as_string = "Mnemonic";
      break;
    case TokenType::Register:
      type_as_string = "Register";
      break;
    case TokenType::SpecialRegister:
      type_as_string = "SpecialRegister";
      break;
    case TokenType::MemoryDereference:
      type_as_string = "MemoryDereference";
      break;
    case TokenType::Immediate:
      type_as_string = "Immediate";
      break;
    case TokenType::LabelRef:
      type_as_string = "LabelRef";
      break;
    case TokenType::Comma:
      type_as_string = "Comma";
      break;
    case TokenType::Unknown:
      type_as_string = "Unknown";
      break;
    default:
      type_as_string = "SHIT";
      break;
    }

    std::ostringstream oss;
    oss << type_as_string << "[" << text << "]";
    return oss.str();
  }
};

Token create_token(TokenType tt, std::string text);

const std::unordered_set<std::string> CHIP8_MNEMONICS = {
    "CLS", "RET", "JP",   "CALL", "SE",  "SNE", "LD",  "ADD", "OR",  "AND",
    "XOR", "SUB", "SUBN", "SHR",  "SHL", "RND", "DRW", "SKP", "SKNP"};

const std::unordered_set<std::string> SPECIAL_REGISTERS = {"I", "DT", "ST"};

class Tokenizer {

private:
  // ====== Checkers ======
  bool is_mnemonic(const std::string &s) {
    return CHIP8_MNEMONICS.count(s); //
  }
  bool is_comma(const std::string &s) {
    return s == ","; //
  }
  bool is_immediate(const std::string &s) {
    if (s.rfind("0x", 0) == 0)
      return true;
    return std::all_of(s.begin(), s.end(), ::isdigit);
  }
  bool is_register(const std::string &s) {
    return s.length() == 2 && s[0] == 'V' && std::isxdigit(s[1]); //
  }
  bool is_labeldef(const std::string &s, size_t tokens_so_far) {
    return tokens_so_far == 0 && !s.empty() && s.back() == ':'; //
  }
  bool is_special_register(const std::string &s) {
    return SPECIAL_REGISTERS.count(s);
  }
  bool is_memory_dereference(const std::string &s) {
    return s == "[I]"; //
  }

  // strips away comment
  std::string strip_comments(const std::string &line) {
    auto comment_pos = line.find(';');

    if (comment_pos != std::string::npos)
      return line.substr(0, comment_pos);
    return line;
  }

  // adds gap between ','
  std::string normalize(const std::string &line) {
    std::string out;

    for (char c : line) {
      if (c == ',') {
        out += ' ';
        out += c;
        out += ' ';
      } else {
        out += c;
      }
    }
    return out;
  }

  // splits up the line to individual string tokens
  std::vector<std::string> generate_string_tokens(const std::string &line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string word;

    while (iss >> word) {
      tokens.push_back(word);
    }
    return tokens;
  }

  std::vector<Token> tokenize(const std::vector<std::string> &string_tokens) {

    std::vector<Token> tokens;

    for (size_t i = 0; i < string_tokens.size(); i += 1) {
      std::string st = string_tokens.at(i);
      if (is_labeldef(st, tokens.size())) {
        tokens.push_back(create_token(TokenType::LabelDef, st));
      } else if (is_mnemonic(st)) {
        tokens.push_back(create_token(TokenType::Mnemonic, st));
      } else if (is_comma(st)) {
        tokens.push_back(create_token(TokenType::Comma, st));
      } else if (is_register(st)) {
        tokens.push_back(create_token(TokenType::Register, st));
      } else if (is_special_register(st)) {
        tokens.push_back(create_token(TokenType::SpecialRegister, st));
      } else if (is_memory_dereference(st)) {
        tokens.push_back(create_token(TokenType::MemoryDereference, st));
      } else if (is_immediate(st)) {
        tokens.push_back(create_token(TokenType::Immediate, st));
      } else {
        tokens.push_back(create_token(TokenType::LabelRef, st));
      }
    }

    return tokens;
  }

  std::string source_code = {};
  std::vector<std::string> source_code_lines = {};
  std::vector<std::vector<Token>> token_lines = {};

  void split_source_to_lines() {
    std::istringstream iss(source_code);
    std::string line;

    while (std::getline(iss, line)) {
      source_code_lines.push_back(line);
    }
  }

  void generate_token_lines() {
    for (const auto &line : source_code_lines) {
      std::string stripped = strip_comments(line);
      std::string normalized = normalize(stripped);
      std::vector<std::string> string_tokens =
          generate_string_tokens(normalized);
      token_lines.push_back(tokenize(string_tokens));
    }
  }

public:
  Tokenizer(std::string source) : source_code(source) {
    // split up the source code by new line
    split_source_to_lines();

    // generates token lines for the source code lines
    generate_token_lines();
  }

  const std::vector<std::vector<Token>> &get_token_lines() const {
    return token_lines;
  }

  std::string get_tokens_lines_as_string() const {
    std::ostringstream oss;

    size_t index = 1;
    for (auto line : get_token_lines()) {
      oss << "LINE " << index << ": ";
      for (auto token : line) {
        oss << token.as_string() << " ";
      }
      oss << "\n";
      index += 1;
    }

    return oss.str();
  }
};

class Assembler {

private:
  Tokenizer tkzr;
  std::unordered_map<std::string, uint16_t> label_table;

  void run_first_pass() {
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
  bool is_immediate_or_label(const Token &tk) {
    return tk.type == TokenType::Immediate || tk.type == TokenType::LabelRef;
  }

  // ====== Parsing helpers ======
  uint16_t parse_immediate(const std::string &s) {
    // hex
    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0)
      return std::stoul(s, nullptr, 16);
    // decimal
    return std::stoul(s, nullptr, 10);
  }

  uint16_t resolve_value(const Token &tk) {
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
  uint16_t parse_JP(const std::vector<Token> &line) {
    // nnn could also be a label

    // JP nnn -> 1nnn
    if (line.size() == 2 && is_immediate_or_label(line[1])) {
      uint16_t addr = resolve_value(line[1]);
      return 0x1000 | (addr & 0x0FFF);
    }

    // JP V0, addr -> Bnnn
    if (line.size() == 4 && line[1].type == TokenType::Register &&
        line[1].text == "V0" && line[2].type == TokenType::Comma &&
        is_immediate_or_label(line[3])) {
      uint16_t addr = resolve_value(line[3]);
      return 0xB000 | (addr & 0x0FFF);
    }

    std::ostringstream oss;
    oss << "invalid JP instruction: ";
    for (const auto &tk : line)
      oss << tk.text << ' ';
    throw std::runtime_error(oss.str());
  }

  uint16_t parse_CALL(const std::vector<Token> &line) {

    // CALL nnn
    // or
    // CALL labelref
    if (line.size() == 2 && is_immediate_or_label(line[1])) {
      uint16_t addr = resolve_value(line[1]);
      return 0x2000 | (addr & 0x0FFF);
    }

    std::ostringstream oss;
    oss << "invalid CALL instruction: ";
    for (const auto &tk : line)
      oss << tk.text << ' ';
    throw std::runtime_error(oss.str());
  }

  uint16_t assemble_instruction(std::vector<Token> line) {

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

    return 0x0000;
  }

public:
  Assembler(std::string source_code) : tkzr(source_code) {
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
};

#endif
