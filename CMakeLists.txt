# basics
cmake_minimum_required(VERSION 3.28)
project(chip8)

# set c++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# include custom headers
include_directories(include)

# 1. chip8emu: the emulator
add_executable(ch8emu
  ch8emu.cpp
  src/chip8.cpp
  src/opcodes.cpp
)
target_link_libraries(ch8emu ${SDL2_LIBRARIES})
target_include_directories(ch8emu PRIVATE include)

# 2. ch8asm: the assembler
add_executable(ch8asm
  # cli
  ch8asm.cpp
  # assembler source code
  src/assembler/assembler.cpp
  src/assembler/tokenizer.cpp
)
target_include_directories(ch8asm PRIVATE include)

# 3. ch8dis: the disassembler
add_executable(ch8dis
  # cli
  ch8dis.cpp
  # disassembler source code
  src/disassembler/disassembler.cpp
)
target_include_directories(ch8dis PRIVATE include)
