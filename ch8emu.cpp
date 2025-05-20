#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
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

  void handle_inputs() {}

  void render_video() {

    int VIDEO_SCREEN_WIDTH = WINDOW_WIDTH * 0.95;
    int VIDEO_X_COUNT = cpu.VIDEO_WIDTH;
    int VIDEO_Y_COUNT = cpu.VIDEO_HEIGHT;

    int GRID_SIZE = VIDEO_SCREEN_WIDTH / VIDEO_X_COUNT;
    int VIDEO_SCREEN_HEIGHT = GRID_SIZE * VIDEO_Y_COUNT;

    int pos_x = (WINDOW_WIDTH - VIDEO_SCREEN_WIDTH) / 2.0f;
    // int pos_y = (WINDOW_HEIGHT - VIDEO_SCREEN_HEIGHT) / 2.0f;
    int pos_y = 10;

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

  void render_ui() {}

  void render() {

    BeginDrawing();

    render_video();
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

  auto rom = LoadRomFromFile("./roms/test/ibm.ch8");

  Chip8 cpu;

  cpu.LoadFromArray(rom.data(), rom.size());

  App app(cpu);

  app.Run();

  return EXIT_SUCCESS;
}
