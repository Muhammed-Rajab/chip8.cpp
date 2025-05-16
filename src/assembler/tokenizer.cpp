#include "../../include/assembler/tokenizer.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

// ====== Token ======
std::string Token::as_string() {

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
  case TokenType::SpecialMnemonic:
    type_as_string = "SpecialMnemonic";
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

Token create_token(TokenType tt, std::string text) { return {tt, text}; }

// =======================
// ====== Tokenizer ======
// =======================

// ====== Checkers ======
bool Tokenizer::is_mnemonic(const std::string &s) {
  return CHIP8_MNEMONICS.count(s);
}

bool Tokenizer::is_comma(const std::string &s) {
  return s == ","; //
}
bool Tokenizer::is_immediate(const std::string &s) {
  if (s.rfind("0x", 0) == 0)
    return true;
  return std::all_of(s.begin(), s.end(), ::isdigit);
}
bool Tokenizer::is_register(const std::string &s) {
  return s.length() == 2 && s[0] == 'V' && std::isxdigit(s[1]); //
}
bool Tokenizer::is_labeldef(const std::string &s, size_t tokens_so_far) {
  return tokens_so_far == 0 && !s.empty() && s.back() == ':'; //
}
bool Tokenizer::is_special_register(const std::string &s) {
  return SPECIAL_REGISTERS.count(s);
}
bool Tokenizer::is_memory_dereference(const std::string &s) {
  return s == "[I]"; //
}
bool Tokenizer::is_special_mnemonic(const std::string &s) {
  return SPECIAL_MNEMONICS.count(s); //
}

// strips away comment
std::string Tokenizer::strip_comments(const std::string &line) {
  auto comment_pos = line.find(';');

  if (comment_pos != std::string::npos)
    return line.substr(0, comment_pos);
  return line;
}

// adds gap between ','
std::string Tokenizer::normalize(const std::string &line) {
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
std::vector<std::string>
Tokenizer::generate_string_tokens(const std::string &line) {
  std::istringstream iss(line);
  std::vector<std::string> tokens;
  std::string word;

  while (iss >> word) {
    tokens.push_back(word);
  }
  return tokens;
}

std::vector<Token>
Tokenizer::tokenize(const std::vector<std::string> &string_tokens) {
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
    } else if (is_special_mnemonic(st)) {
      tokens.push_back(create_token(TokenType::SpecialMnemonic, st));
    } else {
      tokens.push_back(create_token(TokenType::LabelRef, st));
    }
  }

  return tokens;
}

void Tokenizer::split_source_to_lines() {
  std::istringstream iss(source_code);
  std::string line;

  while (std::getline(iss, line)) {
    source_code_lines.push_back(line);
  }
}

void Tokenizer::generate_token_lines() {
  for (const auto &line : source_code_lines) {
    std::string stripped = strip_comments(line);
    std::string normalized = normalize(stripped);
    std::vector<std::string> string_tokens = generate_string_tokens(normalized);
    token_lines.push_back(tokenize(string_tokens));
  }
}

Tokenizer::Tokenizer(std::string source) : source_code(source) {
  // split up the source code by new line
  split_source_to_lines();

  // generates token lines for the source code lines
  generate_token_lines();
}

const std::vector<std::vector<Token>> &Tokenizer::get_token_lines() const {
  return token_lines;
}

std::string Tokenizer::get_tokens_lines_as_string() const {
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
