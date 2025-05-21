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
  constexpr static int WINDOW_WIDTH = 600;
  constexpr static int WINDOW_HEIGHT = 600;

  Font fontTTF;

  void handle_inputs() {}

  void render_video(int px, int py) {

    int VIDEO_SCREEN_WIDTH = WINDOW_WIDTH * 0.95;
    int VIDEO_X_COUNT = cpu.VIDEO_WIDTH;
    int VIDEO_Y_COUNT = cpu.VIDEO_HEIGHT;

    int GRID_SIZE = VIDEO_SCREEN_WIDTH / VIDEO_X_COUNT;
    int VIDEO_SCREEN_HEIGHT = GRID_SIZE * VIDEO_Y_COUNT;

    // int pos_x = (WINDOW_WIDTH - VIDEO_SCREEN_WIDTH) / 2.0f;
    int pos_x = px;
    // int pos_y = (WINDOW_HEIGHT - VIDEO_SCREEN_HEIGHT) / 2.0f;
    int pos_y = py;

    for (int j = 0; j < VIDEO_Y_COUNT; j += 1) {
      for (int i = 0; i < VIDEO_X_COUNT; i += 1) {
        int index = j * VIDEO_X_COUNT + i;
        uint8_t pixel = cpu.video[index];

        int x = pos_x + GRID_SIZE * i;
        int y = pos_y + GRID_SIZE * j;

        if (pixel) {
          Rectangle rec = {(float)x, (float)y, (float)GRID_SIZE,
                           (float)GRID_SIZE};
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

    int line_height = 30;

    auto reg_size = MeasureTextEx(fontTTF, "::Registers::", 20, 0);
    DrawTextEx(fontTTF, "::Registers::", {(float)px, (float)py}, 20, 0, WHITE);

    int fy = py + 25;

    for (int i = 0; i < 16; i += 1) {
      uint8_t val = cpu.V[i];
      std::ostringstream oss;
      oss << "V" << std::hex << i << ": 0x" << std::setw(2) << std::setfill('0')
          << (int)val;

      std::string str = oss.str();
      // DrawText(str.c_str(), px, py + i * line_height, 20, WHITE);
      auto val_size = MeasureTextEx(fontTTF, str.c_str(), 20, 0);

      DrawTextEx(fontTTF, str.c_str(),
                 {(float)px + (reg_size.x - val_size.x) / 2, (float)fy}, 20, 0,
                 WHITE);

      fy += line_height;
    }
  }

  void render_stack(int px, int py) {

    int line_height = 30;

    auto stack_size = MeasureTextEx(fontTTF, "::Stack::", 20, 0);
    DrawTextEx(fontTTF, "::Stack::", {(float)px, (float)py}, 20, 0, WHITE);

    int fy = py + 25;

    for (int i = 15; i >= 0; i -= 1) {
      uint8_t val = cpu.stack[i];
      std::ostringstream oss;
      oss << "[" << ": 0x" << std::setw(2) << std::setfill('0') << (int)val
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

  void render_pc_opcode_instruction() {

    float px = 0;
    float py = 10;
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
    DrawTextEx(fontTTF, "ADD V, 0xFF", {px, py + line_height}, 20, 0, WHITE);
  }

  void render() {

    BeginDrawing();
    ClearBackground(BLACK);

    // render_video(10, 10);

    auto mpos = GetMousePosition();
    // render_registers(mpos.x, mpos.y);
    // render_stack(mpos.x + 150, mpos.y);
    // render_memory(mpos.x + 300, mpos.y);
    // render_memory(mpos.x, mpos.y);
    render_pc_opcode_instruction();

    render_ui();

    EndDrawing();
  }

public:
  App(Chip8 &cpu) : cpu(cpu) {
    // const int screenHeight = 800;
    const char *title = "CHIP 8";

    // ! RAYLIB SETUP
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title);
    SetTargetFPS(60);

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
