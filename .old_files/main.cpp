#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <random>
#include <sys/types.h>
#include <thread>

#define LOG std::cout
#define NL "\n"

const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8 {

public:
  uint8_t registers[16]{};
  uint8_t memory[4096]{};
  uint16_t index{};
  uint16_t pc{};

  uint16_t stack[16]{};
  uint8_t sp{};

  uint8_t delayTimer{};
  uint8_t soundTimer{};

  uint8_t keypad[16]{};

  static constexpr uint8_t VIDEO_WIDTH = 64;
  static constexpr uint8_t VIDEO_HEIGHT = 32;
  uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};

  uint16_t opcode;

  std::default_random_engine randGen;
  std::uniform_int_distribution<uint8_t> randByte;

  typedef void (Chip8::*Chip8Func)();

  Chip8Func table[0xF + 1];
  Chip8Func table0[0xE + 1];
  Chip8Func table8[0xE + 1];
  Chip8Func tableE[0xE + 1];
  Chip8Func tableF[0x65 + 1];

  Chip8()
      : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    pc = START_ADDRESS;

    for (unsigned int i = 0; i < FONTSET_SIZE; i += 1) {
      memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    //*--------------------------------------------------------
    //* SETUP TABLES FOR OPCODES
    //*--------------------------------------------------------

    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    for (int i = 0; i <= 0xE; ++i) {
      table0[i] = &Chip8::OP_NULL;
      table8[i] = &Chip8::OP_NULL;
      tableE[i] = &Chip8::OP_NULL;
    }

    // TABLE 0
    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    // TABLE 8
    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    // TABLE E
    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    for (int i = 0; i <= 0x65; ++i) {
      tableF[i] = &Chip8::OP_NULL;
    }

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
  }

  void Table0() { ((*this).*(table0[opcode & 0x000Fu]))(); }

  void Table8() { ((*this).*(table8[opcode & 0x000Fu]))(); }

  void TableE() { ((*this).*(tableE[opcode & 0x000Fu]))(); }

  void TableF() { ((*this).*(tableF[opcode & 0x00FFu]))(); }

  void OP_NULL() { std::cout << "NULL IS CALLED WTF?" << std::endl; }

  void LoadROM(char const *filename) {

    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {

      std::streampos size = file.tellg();
      char *buffer = new char[size];

      file.seekg(0, std::ios::beg);
      file.read(buffer, size);
      file.close();

      for (long i = 0; i < size; i += 1) {
        memory[START_ADDRESS + i] = buffer[i];
      }

      delete[] buffer;
    }
  }

  // INSTRUCTIONS

  // CLS
  void OP_00E0() { memset(video, 0, sizeof(video)); }

  // RET
  void OP_00EE() {
    sp--;
    pc = stack[sp];
  }

  // JMP addr
  void OP_1nnn() {
    uint16_t address = opcode & 0x0FFFu;
    pc = address;
  }

  // CALL addr
  void OP_2nnn() {
    uint16_t address = opcode & 0x0FFFu;
    stack[sp] = pc;
    ++sp;
    pc = address;
  }

  // SKIP next instruction if Vx == kk
  void OP_3xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    if (registers[Vx] == byte) {
      pc += 2;
    }
  }

  // SKIP next instruction if Vx != kk
  void OP_4xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    if (registers[Vx] != byte) {
      pc += 2;
    }
  }

  // SKIP next instruction if Vx == Vy
  void OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy]) {
      pc += 2;
    }
  }

  // LD Vx, kk (Set Vx = kk)
  void OP_6xkk() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] = byte;
  }

  // ADD Vx, kk (Set Vx = Vx + kk)
  void OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] += byte;
  }

  // LD Vx, Vy (Set Vx = Vy)
  void OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
  }

  // OR Vx, Vy (Set Vx = Vx OR Vy)
  void OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
  }

  // AND Vx, Vy (Set Vx = Vx AND Vy)
  void OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
  }

  // XOR Vx, Vy (Set Vx = Vx XOR Vy)
  void OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
  }

  // ADD Vx, Vy (Set Vx = Vx + Vy, and set Vf = 1 if carry, and lowest 8 bits
  // are kept)
  void OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if (sum > 255u) {
      registers[0xF] = 1;
    } else {
      registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu;
  }

  // SUB Vx, Vy (Set Vx = Vx - Vy, if Vx > Vy then Vf = 1 else Vf = 0)
  void OP_8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] > registers[Vy]) {
      registers[0xF] = 1;
    } else {
      registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
  }

  // SHR Vx (right shift Vx by 1, put the LSB in Vf, and divide Vx by 2)
  void OP_8xy6() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >>= 1;
  }

  // SUBN Vx, Vy (Set Vx = Vy - Vx, if Vy > Vx then Vf = 1 else Vf = 0)
  void OP_8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vy] > registers[Vx]) {
      registers[0xF] = 1;
    } else {
      registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
  }

  // SHL Vx (left shift Vx by 1, put the MSB in Vf, and multiply Vx by 2)
  void OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
    registers[Vx] <<= 1;
  }

  // SNE Vx, Vy (Skip next instruction if Vx != Vy)
  void OP_9xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy]) {
      pc += 2;
    }
  }

  // LD I, addr (Set I = nnn)
  void OP_Annn() {
    uint16_t address = opcode & 0x0FFFu;
    index = address;
  }

  // JMP V0, addr (jump to location V0 + addr)
  void OP_Bnnn() {
    uint16_t address = opcode & 0x0FFFu;
    pc = registers[0] + address;
  }

  // RND Vx, byte (Set Vx = random byte AND KK)
  void OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] = randByte(randGen) & byte;
  }

  // DRW Vx, Vy, nibble (Display n-byte sprite, starting at memory location I at
  // (Vx, Vy), set Vf = collision)
  void OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = (opcode & 0x000Fu);

    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row) {

      uint8_t spriteByte = memory[index + row];

      for (unsigned int col = 0; col < 8; ++col) {

        uint8_t spritePixel = spriteByte & (0x80u >> col);
        uint32_t *screenPixel =
            &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

        if (spritePixel) {
          if (*screenPixel == 0xFFFFFFFF) {
            registers[0xF] = 1;
          }

          *screenPixel ^= 0xFFFFFFFF;
        }
      }
    }
  }

  // SKP Vx (Skip next instruction if key with the value of Vx is pressed)
  void OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (keypad[key]) {
      pc += 2;
    }
  }

  // SKPN Vx (Skip next instruction if key with the value of Vx is not pressed)
  void OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (!keypad[key]) {
      pc += 2;
    }
  }

  // LD Vx, DT (Set Vx = delay timer value)
  void OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
  }

  // LD Vx, K (Wait for a key press, store the value of the key in Vx)
  // easiest way -> decrement pc by 2, so the same command is run again,
  // basically "waiting"
  void OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[0]) {
      registers[Vx] = 0;
    } else if (keypad[1]) {
      registers[Vx] = 1;
    } else if (keypad[2]) {
      registers[Vx] = 2;
    } else if (keypad[3]) {
      registers[Vx] = 3;
    } else if (keypad[4]) {
      registers[Vx] = 4;
    } else if (keypad[5]) {
      registers[Vx] = 5;
    } else if (keypad[6]) {
      registers[Vx] = 6;
    } else if (keypad[7]) {
      registers[Vx] = 7;
    } else if (keypad[8]) {
      registers[Vx] = 8;
    } else if (keypad[9]) {
      registers[Vx] = 9;
    } else if (keypad[10]) {
      registers[Vx] = 10;
    } else if (keypad[11]) {
      registers[Vx] = 11;
    } else if (keypad[12]) {
      registers[Vx] = 12;
    } else if (keypad[13]) {
      registers[Vx] = 13;
    } else if (keypad[14]) {
      registers[Vx] = 14;
    } else if (keypad[15]) {
      registers[Vx] = 15;
    } else {
      pc -= 2;
    }
  }

  // LD DT, Vx
  void OP_Fx15() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delayTimer = registers[Vx];
  }

  // LD ST, Vx
  void OP_Fx18() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    soundTimer = registers[Vx];
  }

  // ADD I, Vx (Set I = I + Vx)
  void OP_Fx1E() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    index += registers[Vx];
  }

  // LD I, Vx
  void OP_Fx29() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    index = FONTSET_START_ADDRESS + (5 * digit);
  }

  // LD B, Vx (stores the BCD representation of Vx in memory location I, I+1,
  // and I+2)
  void OP_Fx33() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    // ONES
    memory[index + 2] = value % 10;
    value /= 10;

    // TENS
    memory[index + 1] = value % 10;
    value /= 10;

    // HUNDREDS
    memory[index] = value % 10;
  }

  // LD [I], Vx (Store register V0 through Vx in memory starting from location
  // I)
  void OP_Fx55() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i) {
      memory[index + i] = registers[i];
    }
  }

  // LD Vx, [I] (Read registers V0 through Vx from the memory starting at
  // location I)
  void OP_Fx65() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i) {
      registers[i] = memory[index + i];
    }
  }

  void Cycle() {

    // fetch
    opcode = (memory[pc] << 8u) | memory[pc + 1];

    pc += 2;

    // decode and execute
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    if (delayTimer > 0) {
      --delayTimer;
    }

    if (soundTimer > 0) {
      --soundTimer;
    }
  }

  void DrawDisplay() {
    std::system("clear");

    for (int y = 0; y < VIDEO_HEIGHT; ++y) {
      for (int x = 0; x < VIDEO_WIDTH; ++x) {
        std::cout << (video[y * VIDEO_WIDTH + x] ? "█" : " ");
      }
      std::cout << "\n";
    }
  }
};

int main() {

  Chip8 chip;

  chip.LoadROM("./test_opcode.ch8");

  while (true) {
    chip.Cycle();
    chip.DrawDisplay();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return EXIT_SUCCESS;
}
