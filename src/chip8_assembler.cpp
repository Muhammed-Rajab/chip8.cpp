#include "../include/chip8_assembler.hpp"

Token create_token(TokenType tt, std::string text) { return {tt, text}; }

// uint16_t Assembler::Encode(std::string line) {
//
//   auto comment_pos = line.find(';');
//
//   // strip away the comment
//   if (comment_pos != std::string::npos)
//     line = line.substr(0, comment_pos);
//
//   return 0x0;
// }
