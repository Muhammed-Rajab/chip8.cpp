#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "include/disassembler/disassembler.hpp"

constexpr auto VERSION = 0.1;

void print_help() {
  std::cout << "usage: ch8dis <input_file> [--verbose] [--help] [--version]\n";
  std::cout << "options:\n"
            << "  --verbose       enable verbose disassembly\n"
            << "  --help          show this help message\n"
            << "  --version       show version info\n";
}

void print_version() { std::cout << "ch8dis nuts version " << VERSION << "\n"; }

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input file provided.\n";
    print_help();
    return 1;
  }

  std::string input_file;
  bool verbose = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help") {
      print_help();
      return 0;
    } else if (arg == "--version") {
      print_version();
      return 0;
    } else if (arg == "--verbose") {
      verbose = true;
    } else if (input_file.empty()) {
      input_file = arg;
    } else {
      std::cerr << "unknown argument: " << arg << "\n";
      return 1;
    }
  }

  try {
    std::ifstream file(input_file, std::ios::binary);
    if (!file) {
      throw std::runtime_error("failed to open input file: " + input_file);
    }

    std::vector<uint8_t> rom((std::istreambuf_iterator<char>(file)), {});
    Disassembler dis;
    std::string output = dis.DecodeRomFromArray(rom, verbose);
    std::cout << output;

  } catch (const std::exception &e) {
    std::cerr << "disassembler error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
