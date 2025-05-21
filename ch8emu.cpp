#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
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

class App {
private:
  // Chip8
  Chip8 &cpu;

  bool quit = false;

  // state
  constexpr static int WINDOW_WIDTH = 942;
  constexpr static int WINDOW_HEIGHT = 700;

  // video
  int VIDEO_SCREEN_WIDTH = 600;
  int VIDEO_X_COUNT = cpu.VIDEO_WIDTH;
  int VIDEO_Y_COUNT = cpu.VIDEO_HEIGHT;

  int VIDEO_GRID_SIZE = VIDEO_SCREEN_WIDTH / VIDEO_X_COUNT;
  int VIDEO_SCREEN_HEIGHT = VIDEO_GRID_SIZE * VIDEO_Y_COUNT;

  Font fontTTF;

  void handle_inputs() {}

  void render_video(int px, int py) {

    // int pos_x = (WINDOW_WIDTH - VIDEO_SCREEN_WIDTH) / 2.0f;
    int pos_x = px;
    // int pos_y = (WINDOW_HEIGHT - VIDEO_SCREEN_HEIGHT) / 2.0f;
    int pos_y = py;

    for (int j = 0; j < VIDEO_Y_COUNT; j += 1) {
      for (int i = 0; i < VIDEO_X_COUNT; i += 1) {
        int index = j * VIDEO_X_COUNT + i;
        uint8_t pixel = cpu.video[index];

        int x = pos_x + VIDEO_GRID_SIZE * i;
        int y = pos_y + VIDEO_GRID_SIZE * j;

        if (pixel) {
          Rectangle rec = {(float)x, (float)y, (float)VIDEO_GRID_SIZE,
                           (float)VIDEO_GRID_SIZE};
          DrawRectangleRec(rec, WHITE);
          DrawRectangleLinesBetter(rec, 1, BLACK);
        }
      }
    }

    // draw border
    Rectangle border = {(float)pos_x, (float)pos_y, (float)VIDEO_SCREEN_WIDTH,
                        (float)VIDEO_SCREEN_HEIGHT};
    DrawRectangleLinesBetter(border, 1, GRAY);
  }

  void render_memory(int px, int py) {

    int s = 3;

    for (size_t i = 0; i < 4096; i += 1) {
      size_t x = (i % 64) * s;
      size_t y = (i / 64) * s;

      uint8_t byte = cpu.memory[i];

      Color c = {byte, byte, byte, 255};

      if (i < cpu.FONTSET_START_ADDRESS) {
        c = {byte, 0, 0, 255};
      } else if (i >= cpu.FONTSET_START_ADDRESS &&
                 i < cpu.FONTSET_START_ADDRESS + cpu.FONTSET_SIZE) {
        c = {byte, byte, 0, 255};
      } else if (i > cpu.FONTSET_START_ADDRESS + cpu.FONTSET_SIZE &&
                 i < cpu.STARTING_ADDRESS) {

        c = {255, 0, 0, 255};
      }

      if (i == cpu.pc) {
        c = {0, 255, 0, 255};
      }

      Rectangle rec = {(float)(px + x), (float)(py + y), (float)s, (float)s};

      DrawRectangleRec(rec, c);
    }

    Rectangle border = {(float)px - 1, (float)py - 1, (float)64 * s + 1,
                        (float)64 * s + 1};
    DrawRectangleLinesBetter(border, 1, GRAY);
  }

  void render_ui() {}

  void render_registers(int px, int py) {

    const int line_height = 25;
    const int column_spacing = 110;
    const int text_size = 20;

    // Draw the title
    DrawTextEx(fontTTF, "::Registers::", {(float)px, (float)py}, text_size, 0,
               WHITE);

    // Starting point for registers
    int fy = py + line_height;

    for (int i = 0; i < 16; ++i) {
      uint8_t val = cpu.V[i];
      std::ostringstream oss;
      oss << "V" << std::hex << std::uppercase << i << ": 0x" << std::setw(2)
          << std::setfill('0') << (int)val;
      std::string str = oss.str();

      float x = (i < 8) ? px : px + column_spacing;
      float y = (i < 8) ? fy + i * line_height : fy + (i - 8) * line_height;

      DrawTextEx(fontTTF, str.c_str(), {(float)x, (float)y}, text_size, 0,
                 WHITE);
    }
  }

  void render_stack(int px, int py) {

    int line_height = 25;

    auto stack_size = MeasureTextEx(fontTTF, "::Stack::", 20, 0);
    DrawTextEx(fontTTF, "::Stack::", {(float)px, (float)py}, 20, 0, WHITE);

    int fy = py + 25;

    for (int i = 15; i >= 0; i -= 1) {
      uint8_t val = cpu.stack[i];
      std::ostringstream oss;
      oss << "[" << "0x" << std::setw(2) << std::setfill('0') << (int)val
          << "]";

      std::string str = oss.str();
      auto val_size = MeasureTextEx(fontTTF, str.c_str(), 20, 0);

      if (i == cpu.sp - 1) {
        DrawRectangle((float)px + (stack_size.x - val_size.x) / 2, (float)fy,
                      val_size.x, val_size.y, RED);
      }

      DrawTextEx(fontTTF, str.c_str(),
                 {(float)px + (stack_size.x - val_size.x) / 2, (float)fy}, 20,
                 0, WHITE);

      fy += line_height;
    }
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
    // DrawTextEx(fontTTF, Disassembler::Decode(cpu.opcode).c_str(),
    //            {px, py + line_height}, 20, 0, WHITE);
    //
    DrawTextEx(fontTTF, "DRW V0, V1, 15", {px, py + line_height}, 20, 0, WHITE);
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

  void render() {

    BeginDrawing();
    ClearBackground(BLACK);

    int vox = 20;
    int voy = 20;

    render_video(vox, voy);

    int sox = 10;
    int soy = 0;

    render_stack(VIDEO_SCREEN_WIDTH + vox + sox, voy + soy);

    int rox = 100;
    int roy = 0;

    render_registers(VIDEO_SCREEN_WIDTH + vox + sox + rox, voy + roy);

    int mox = 0;
    int moy = 250;

    render_memory(VIDEO_SCREEN_WIDTH + vox + sox + rox + mox, moy);

    int poy = 20;
    render_pc_opcode_instruction(vox, VIDEO_SCREEN_HEIGHT + voy + poy);

    // separator
    int sepox = 315;
    DrawRectangle(vox + sepox, VIDEO_SCREEN_HEIGHT + voy + poy, 1, 50, GRAY);

    int iox = 15;
    render_index_and_special_registers(vox + sepox + iox,
                                       VIDEO_SCREEN_HEIGHT + voy + poy);

    EndDrawing();
  }

public:
  App(Chip8 &cpu) : cpu(cpu) {
    // const int screenHeight = 800;
    const char *title = "CHIP 8";

    // ! RAYLIB SETUP
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title);
    SetTargetFPS(2);

    // const int screenWidth = GetMonitorWidth(0);
    // const int screenHeight = GetMonitorHeight(0);
    // SetWindowSize(screenWidth, screenHeight);

    fontTTF = LoadFontEx("./fonts/scp-bold.ttf", 128, 0, 0);

    SetTextureFilter(fontTTF.texture, TEXTURE_FILTER_BILINEAR);
  }

  ~App() { CloseWindow(); }

  void Run() {
    while (!WindowShouldClose() && !quit) {
      handle_inputs();

      render();

      cpu.Cycle();
    }
  }
};

int main(int argc, char *args[]) {

  auto rom = LoadRomFromFile("./roms/test/f_you.ch8");

  Chip8 cpu;

  cpu.LoadFromArray(rom.data(), rom.size());

  App app(cpu);

  app.Run();

  return EXIT_SUCCESS;
}
