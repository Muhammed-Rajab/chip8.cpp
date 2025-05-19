#include <cstdlib>
#include <iostream>
#include <string>

#include "include/assembler/assembler.hpp"

constexpr auto VERSION = 0.1;

void print_help() {
  std::cout << "usage: ch8asm <input_file> [-o <output_file>] [--verbose] "
               "[--help] [--version]\n";
  std::cout << "options:\n"
            << "  -o <file>       specify output file (default: out.ch8)\n"
            << "  --verbose       enable verbose output\n"
            << "  --help          show this help message\n"
            << "  --version       show version info\n";
}

void print_version() { std::cout << "ch8asm version " << VERSION << "\n"; }

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input file provided.\n";
    print_help();
    return 1;
  }

  std::string input_file;
  std::string output_file = "out.ch8";
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
    } else if (arg == "-o") {
      if (i + 1 < argc) {
        output_file = argv[++i];
      } else {
        std::cerr << "error: -o requires an argument.\n";
        return 1;
      }
    } else if (input_file.empty()) {
      input_file = arg;
    } else {
      std::cerr << "unknown argument: " << arg << "\n";
      return 1;
    }
  }

  if (input_file.empty()) {
    std::cerr << "error: no input file specified.\n";
    return 1;
  }

  if (verbose) {
    std::cout << "[verbose] assembling " << input_file << " to " << output_file
              << "\n";
  }

  try {
    Assembler assembler = Assembler::FromFile(input_file);
    assembler.WriteToFile(output_file);

    if (verbose) {
      std::cout << "[verbose] wrote " << output_file << " ("
                << assembler.GetBytes().size() << " bytes)\n";
    } else {
      std::cout << "assembled successfully to " << output_file << "\n";
    }
  } catch (const std::exception &e) {
    std::cerr << "assembler error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
