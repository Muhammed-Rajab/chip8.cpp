#ifndef TOKENIZER_4_CHIP8_HPP
#define TOKENIZER_4_CHIP8_HPP

#include <cctype>
#include <cstddef>
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
  SpecialRegister,
  SpecialMnemonic,
  MemoryDereference,
  ByteDirective,
  Unknown
};

struct Token {
  TokenType type;
  std::string text;

  std::string as_string() const;
};

Token create_token(TokenType tt, std::string text);

const std::unordered_set<std::string> CHIP8_MNEMONICS = {
    "CLS", "RET", "JP",   "CALL", "SE",  "SNE", "LD",  "ADD", "OR",  "AND",
    "XOR", "SUB", "SUBN", "SHR",  "SHL", "RND", "DRW", "SKP", "SKNP"};

const std::unordered_set<std::string> SPECIAL_REGISTERS = {"I", "DT", "ST"};

const std::unordered_set<std::string> SPECIAL_MNEMONICS = {"F", "B", "K"};

class Tokenizer {

private:
  std::string source_code = {};
  std::vector<std::string> source_code_lines = {};
  std::vector<std::vector<Token>> token_lines = {};

  // ====== Checkers ======
  bool is_mnemonic(const std::string &s);
  bool is_comma(const std::string &s);
  bool is_immediate(const std::string &s);
  bool is_register(const std::string &s);
  bool is_labeldef(const std::string &s, size_t tokens_so_far);
  bool is_special_register(const std::string &s);
  bool is_memory_dereference(const std::string &s);
  bool is_special_mnemonic(const std::string &s);
  bool is_byte_directive(const std::string &s);

  // strips away comment
  std::string strip_comments(const std::string &line);

  // adds gap between ','
  std::string normalize(const std::string &line);

  // splits up the line to individual string tokens
  std::vector<std::string> generate_string_tokens(const std::string &line);
  std::vector<Token> tokenize(const std::vector<std::string> &string_tokens);

  // handles source code
  void split_source_to_lines();

  // generates token lines
  void generate_token_lines();

public:
  Tokenizer(std::string source);

  const std::vector<std::vector<Token>> &get_token_lines() const;

  std::string get_tokens_lines_as_string() const;
};

#endif
