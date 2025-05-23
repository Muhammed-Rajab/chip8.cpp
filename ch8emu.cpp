#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "./include/chip8.hpp"
#include "./include/disassembler/disassembler.hpp"

#include "raylib/raylib.h"

// ====== ROM Loader ======
std::vector<uint8_t> LoadRomFromFile(const std::string &filename) {
  std::ifstream file(filename,
                     std::ios::binary | std::ios::ate); // ate = seek to end

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open ROM file: " + filename);
  }

  std::streamsize size = file.tellg(); // get size of file
  file.seekg(0, std::ios::beg);        // rewind to beginning

  std::vector<uint8_t> rom(size);
  if (!file.read(reinterpret_cast<char *>(rom.data()), size)) {
    throw std::runtime_error("Failed to read ROM file: " + filename);
  }

  return rom;
}

void DrawRectangleLinesBetter(Rectangle rec, float thickness, Color c) {
  DrawRectangle(rec.x, rec.y, rec.width, thickness, c);
  DrawRectangle(rec.x, rec.y + rec.height - thickness, rec.width, thickness, c);
  DrawRectangle(rec.x, rec.y, thickness, rec.height, c);
  DrawRectangle(rec.x + rec.width - thickness, rec.y, thickness, rec.height, c);
}

enum class EmulatorModes { Normal, Debug };

class Emulator {
private:
  // Chip8
  Chip8 &cpu;

  EmulatorModes mode = EmulatorModes::Debug;
  int normal_cycles_per_frame = 12;
  int debug_cycles_per_frame = 1;

  // video
  int VIDEO_SCREEN_WIDTH = {};
  int VIDEO_X_COUNT = {};
  int VIDEO_Y_COUNT = {};

  int VIDEO_GRID_SIZE = {};
  int VIDEO_SCREEN_HEIGHT = {};

  // raylib
  constexpr static int WINDOW_WIDTH = 955;
  constexpr static int WINDOW_HEIGHT = 500;
  Font fontTTF;

  // disassembled code
  std::vector<std::string> disassembled_rom = {};

  void handle_cpu_input() {
    const int chip8_keymap[16] = {
        KEY_X,     // 0
        KEY_ONE,   // 1
        KEY_TWO,   // 2
        KEY_THREE, // 3
        KEY_Q,     // 4
        KEY_W,     // 5
        KEY_E,     // 6
        KEY_A,     // 7
        KEY_S,     // 8
        KEY_D,     // 9
        KEY_Z,     // A
        KEY_C,     // B
        KEY_FOUR,  // C
        KEY_R,     // D
        KEY_F,     // E
        KEY_V      // F
    };

    for (int i = 0; i < 16; i++) {
      cpu.keypad[i] = IsKeyDown(chip8_keymap[i]) ? 1 : 0;
    }
  }

  void render_video(float px, float py) {

    for (int row = 0; row < VIDEO_Y_COUNT; row += 1) {
      for (int col = 0; col < VIDEO_X_COUNT; col += 1) {
        const int index = row * VIDEO_X_COUNT + col;
        const uint8_t pixel = cpu.video[index];

        const int x = px + VIDEO_GRID_SIZE * col;
        const int y = py + VIDEO_GRID_SIZE * row;

        if (pixel) {
          const Rectangle rec = {(float)x, (float)y, (float)VIDEO_GRID_SIZE,
                                 (float)VIDEO_GRID_SIZE};
          DrawRectangleRec(rec, WHITE);
          DrawRectangleLinesBetter(rec, 1, BLACK);
        }
      }
    }

    // draw border
    const Rectangle border = {(float)px, (float)py, (float)VIDEO_SCREEN_WIDTH,
                              (float)VIDEO_SCREEN_HEIGHT};
    DrawRectangleLinesBetter(border, 1, GRAY);
  }

  void render_memory(float px, float py) {

    //====== A note to my future self ======
    // There are lots of magic numbers here, and it can be frustrating.
    // All these numbers are there to render the things to screen properly, in a
    // tight layout. Tweak around, make breaking changes and findout how
    // they work.

    const float s = 207.0f / 64.0f;

    for (size_t i = 0; i < 4096; i++) {
      const float x = (i % 64) * s;
      const float y = static_cast<int>(i / 64) * s;

      const uint8_t byte = cpu.memory[i];
      Color c = {byte, byte, byte, 255};

      if (i < cpu.FONTSET_START_ADDRESS) {
        c = {byte, 0, 0, 255};
      } else if (i < cpu.FONTSET_START_ADDRESS + cpu.FONTSET_SIZE) {
        c = {byte, byte, 0, 255};
      } else if (i < cpu.STARTING_ADDRESS) {
        c = {255, 0, 0, 255};
      }

      if (i == cpu.pc) {
        c = {0, 255, 0, 255};
      }

      const Rectangle rec = {px + x, py + y, s, s};
      DrawRectangleRec(rec, c);
    }

    const Rectangle border = {px - 1, py - 1, 207.0f + 1, 207.0f + 1};
    DrawRectangleLinesBetter(border, 1, GRAY);
  }

  void render_registers(float px, float py) {

    const int line_height = 25;
    const int column_spacing = 110;
    const int text_size = 20;

    px += 10;
    py += 10;

    DrawTextEx(fontTTF, "Registers", {(float)px, (float)py}, text_size, 0,
               WHITE);

    int fy = py + line_height;

    for (int i = 0; i < 16; ++i) {
      const uint8_t val = cpu.V[i];
      std::ostringstream oss;
      oss << "V" << std::hex << std::uppercase << i << ": 0x" << std::setw(2)
          << std::setfill('0') << (int)val;
      std::string str = oss.str();

      const float x = (i < 8) ? px : px + column_spacing;
      const float y =
          (i < 8) ? fy + i * line_height : fy + (i - 8) * line_height;

      DrawTextEx(fontTTF, str.c_str(), {(float)x, (float)y}, text_size, 0,
                 WHITE);
    }

    DrawRectangleLinesBetter({px - 10, py - 10, 207, py + line_height * 8 + 7},
                             1, GRAY);
  }

  void render_stack(float px, float py) {

    const int line_height = 25;

    py += 10;

    const auto stack_size = MeasureTextEx(fontTTF, "##Stack##", 20, 0);
    DrawTextEx(fontTTF, "  Stack  ", {(float)px, (float)py}, 20, 0, WHITE);

    float fy = py + 25;

    const int top_index = cpu.sp - 1;

    for (int i = 15; i >= 0; i -= 1) {
      const uint8_t val = cpu.stack[i];
      std::ostringstream oss;
      oss << "[" << "0x" << std::setw(2) << std::setfill('0') << (int)val
          << "]";

      const std::string str = oss.str();
      const auto val_size = MeasureTextEx(fontTTF, str.c_str(), 20, 0);

      if (i == top_index) {
        DrawRectangle((float)px + (stack_size.x - val_size.x) / 2, (float)fy,
                      val_size.x, val_size.y, RED);
      }

      DrawTextEx(fontTTF, str.c_str(),
                 {(float)px + (stack_size.x - val_size.x) / 2, (float)fy}, 20,
                 0, WHITE);

      fy += line_height;
    }

    DrawRectangleLinesBetter({px, py - 10, stack_size.x, fy - line_height + 27},
                             1, GRAY);
  }

  void render_pc_opcode_instruction(float px, float py) {

    float line_height = 25;

    std::ostringstream oss;

    DrawTextEx(fontTTF, "PC", {px, py}, 20, 0, WHITE);
    oss << "0x" << std::hex << std::setfill('0') << std::setw(4) << int(cpu.pc);
    DrawTextEx(fontTTF, oss.str().c_str(), {px, py + line_height}, 20, 0,
               WHITE);

    oss.str("");
    oss.clear();

    px += 75;

    DrawTextEx(fontTTF, "OPCODE", {px, py}, 20, 0, WHITE);
    oss << "0x" << std::hex << std::setfill('0') << std::setw(4)
        << int(cpu.opcode);
    DrawTextEx(fontTTF, oss.str().c_str(), {px, py + line_height}, 20, 0,
               WHITE);

    oss.str("");
    oss.clear();

    px += 90;

    DrawTextEx(fontTTF, "INSTRUCTION", {px, py}, 20, 0, WHITE);
    DrawTextEx(fontTTF, Disassembler::Decode(cpu.opcode).c_str(),
               {px, py + line_height}, 20, 0, WHITE);
  }

  void render_index_and_special_registers(float px, float py) {
    std::ostringstream oss;

    oss << "I: 0x" << std::hex << std::setfill('0') << std::setw(4)
        << int(cpu.index);
    oss << " ";

    oss << "DT: 0x" << std::hex << std::setfill('0') << std::setw(2)
        << int(cpu.delay);

    oss << " ";

    oss << "ST: 0x" << std::hex << std::setfill('0') << std::setw(2)
        << int(cpu.sound);

    DrawTextEx(fontTTF, oss.str().c_str(), {px, py}, 20, 0, WHITE);
  }

  void render_disassembled_code(float px, float py) {

    int line_height = 30;
    int current_index = (cpu.pc - 0x200) / 2;

    py += 5;

    // Display three lines: one before, current, and one after
    for (int i = -1; i <= 1; ++i) {
      int index = current_index + i;

      if (index < 0 || index >= disassembled_rom.size())
        continue; // skip out-of-bounds

      const std::string &line = disassembled_rom[index];

      Color color = (i == 0) ? RED : WHITE;

      DrawTextEx(fontTTF, std::to_string(cpu.pc - index).c_str(),
                 {px + 10, py + (i + 1) * line_height}, 20, 0, color);
      DrawTextEx(fontTTF, line.c_str(), {px + 60, py + (i + 1) * line_height},
                 20, 0, color);
    }

    DrawRectangleLinesBetter({px, py - 5, (float)VIDEO_SCREEN_WIDTH, 90}, 1,
                             GRAY);
  }

  void render_debug() {

    BeginDrawing();
    ClearBackground(BLACK);

    int vox = 20;
    int voy = 20;

    render_video(vox, voy);

    int sox = 15;
    int soy = 0;

    render_stack(VIDEO_SCREEN_WIDTH + vox + sox, voy + soy);

    int rox = 100;
    int roy = 0;

    render_registers(VIDEO_SCREEN_WIDTH + vox + sox + rox, voy + roy);

    int mox = 0;
    int moy = 270;

    render_memory(VIDEO_SCREEN_WIDTH + vox + sox + rox + mox, moy);

    int poy = 20;
    render_pc_opcode_instruction(vox, VIDEO_SCREEN_HEIGHT + voy + poy);

    // separator
    int sepox = 315;
    DrawRectangle(vox + sepox, VIDEO_SCREEN_HEIGHT + voy + poy, 1, 50, GRAY);

    int iox = 15;
    render_index_and_special_registers(vox + sepox + iox,
                                       VIDEO_SCREEN_HEIGHT + voy + poy);

    int doy = 60;
    render_disassembled_code(vox, VIDEO_SCREEN_HEIGHT + voy + poy + doy);

    EndDrawing();
  }

  void render_normal() {

    BeginDrawing();

    ClearBackground(BLACK);

    render_video(20, 20);

    EndDrawing();
  }

public:
  Emulator(Chip8 &cpu) : cpu(cpu) {
    // const int screenHeight = 800;
    const char *title = "CHIP 8";

    // ! RAYLIB SETUP
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title);
    SetTargetFPS(60);

    fontTTF = LoadFontEx("./fonts/scp-bold.ttf", 128, 0, 0);

    SetTextureFilter(fontTTF.texture, TEXTURE_FILTER_BILINEAR);

    disassembled_rom = Disassembler::DecodeRomFromArrayAsVector(cpu.rom, false);

    // ====== Mode-based properties ======

    if (mode == EmulatorModes::Normal) {
      VIDEO_SCREEN_WIDTH = WINDOW_WIDTH - 40;
    } else {
      VIDEO_SCREEN_WIDTH = 600;
    }

    VIDEO_X_COUNT = cpu.VIDEO_WIDTH;
    VIDEO_Y_COUNT = cpu.VIDEO_HEIGHT;

    VIDEO_GRID_SIZE = VIDEO_SCREEN_WIDTH / VIDEO_X_COUNT;
    VIDEO_SCREEN_HEIGHT = VIDEO_GRID_SIZE * VIDEO_Y_COUNT;
  }

  void Run() {
    while (!WindowShouldClose()) {

      handle_cpu_input();

      // ====== Mode-based rendering ======
      if (mode == EmulatorModes::Normal) {
        render_normal();
        for (int i = 0; i < normal_cycles_per_frame; i += 1) {
          cpu.Cycle();
        }
      } else {
        DrawFPS(10, 10);
        render_debug();
        for (int i = 0; i < debug_cycles_per_frame; i += 1) {
          cpu.Cycle();
        }
        cpu.Cycle();
      }
    }
  }

  ~Emulator() { CloseWindow(); }
};

int main(int argc, char *args[]) {

  auto rom = LoadRomFromFile("./roms/test/keypad.ch8");

  Chip8 cpu;

  cpu.LoadFromArray(rom.data(), rom.size());

  Emulator emu(cpu);

  emu.Run();

  return EXIT_SUCCESS;
}
