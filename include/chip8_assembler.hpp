#ifndef CHIP8_ASSEMBLER_HPP
#define CHIP8_ASSEMBLER_HPP

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

enum class TokenType {
  LabelDef,
  Mnemonic,
  Register,
  Immediate,
  LabelRef,
  Comma,
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

class Assembler {

private:
  std::string strip_comments(const std::string &line) {

    auto comment_pos = line.find(';');

    if (comment_pos != std::string::npos)
      return line.substr(0, comment_pos);

    return line;
  }

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

  std::vector<std::string> tokenize(const std::string &line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string word;

    while (iss >> word) {
      tokens.push_back(word);
    }

    return tokens;
  }

  bool is_mnemonic(const std::string &s) { return CHIP8_MNEMONICS.count(s); }
  bool is_comma(const std::string &s) { return s == ","; }
  bool is_immediate(const std::string &s) { return s.rfind("0x", 0) == 0; }
  bool is_register(const std::string &s) {
    return s.length() == 2 && s[0] == 'V' && std::isxdigit(s[1]);
  }
  bool is_labeldef(const std::string &s, size_t tokens_so_far) {
    return tokens_so_far == 0 && !s.empty() && s.back() == ':';
  }

  std::vector<Token> classify(const std::vector<std::string> &string_tokens) {

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
      } else if (is_immediate(st)) {
        tokens.push_back(create_token(TokenType::Immediate, st));
      } else {
        tokens.push_back(create_token(TokenType::LabelRef, st));
      }
    }

    return tokens;
  }

public:
  uint16_t Encode(std::string line) {

    std::cout << "original: '" << line << "'" << std::endl;

    std::string stripped_line = strip_comments(line);
    std::cout << "stripped: '" << stripped_line << "'" << std::endl;

    std::string normalized_line = normalize(stripped_line);
    std::cout << "normalized: '" << normalized_line << "'" << std::endl;

    std::vector<std::string> tokens = tokenize(normalized_line);

    std::cout << "tokens: " << std::endl;
    for (auto t : tokens) {
      std::cout << "'" << t << "'" << std::endl;
    }

    std::vector<Token> classified_tokens = classify(tokens);
    std::cout << "classified tokens: " << std::endl;
    for (auto t : classified_tokens) {
      std::cout << t.as_string() << std::endl;
    }

    return 0x0;
  }
};

#endif
