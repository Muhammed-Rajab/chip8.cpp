# basics
cmake_minimum_required(VERSION 3.28)
project(chip8)

# set c++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# === Fetch Raylib ===
include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG        5.0
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

include_directories(include)

# 1. chip8emu: the emulator
add_executable(ch8emu
  ch8emu.cpp
  src/chip8.cpp
  src/opcodes.cpp
  src/disassembler/disassembler.cpp
)

# target_link_libraries(ch8emu PRIVATE
#   ${CMAKE_SOURCE_DIR}/raylib/libraylib.a
#   m
#   dl
#   pthread
#   GL
#   rt
#   X11
# )
#


# target_include_directories(ch8emu PRIVATE 
#   include
#   raylib/
# )

target_link_libraries(ch8emu PRIVATE raylib)




# Extra libraries for Linux
if(UNIX)
  target_link_libraries(ch8emu PRIVATE m dl pthread GL X11 rt)
endif()

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
