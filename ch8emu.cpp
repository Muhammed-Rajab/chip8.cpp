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

class App {
private:
  // Chip8
  Chip8 &cpu;

  bool quit = false;

  // state
  constexpr static int WINDOW_WIDTH = 500;
  constexpr static int WINDOW_HEIGHT = 500;

  void handle_inputs() {}

  void render_video() {

    int VIDEO_SCREEN_WIDTH = WINDOW_WIDTH * 0.95;
    int VIDEO_X_COUNT = cpu.VIDEO_WIDTH;
    int VIDEO_Y_COUNT = cpu.VIDEO_HEIGHT;

    int GRID_SIZE = VIDEO_SCREEN_WIDTH / VIDEO_X_COUNT;
    int VIDEO_SCREEN_HEIGHT = GRID_SIZE * VIDEO_Y_COUNT;

    int pos_x = (WINDOW_WIDTH - VIDEO_SCREEN_WIDTH) / 2.0f;
    int pos_y = (WINDOW_HEIGHT - VIDEO_SCREEN_HEIGHT) / 2.0f;

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
          DrawRectangleRec(rec, WHITE);
          DrawRectangle(rec.x, rec.y, rec.width, 1, BLACK); // top
          DrawRectangle(rec.x, rec.y + rec.height - 1, rec.width, 1,
                        BLACK);                              // bottom
          DrawRectangle(rec.x, rec.y, 1, rec.height, BLACK); // left
          DrawRectangle(rec.x + rec.width - 1, rec.y, 1, rec.height,
                        BLACK); // right
        }
      }
    }

    // render border
    int thickness = 1;
    Color color = WHITE;

    DrawRectangle(pos_x, pos_y, VIDEO_SCREEN_WIDTH, thickness, color); // top
    DrawRectangle(pos_x, pos_y + VIDEO_SCREEN_HEIGHT - thickness,
                  VIDEO_SCREEN_WIDTH, thickness, color); // bottom
    DrawRectangle(pos_x, pos_y, thickness, VIDEO_SCREEN_HEIGHT, color); // left
    DrawRectangle(pos_x + VIDEO_SCREEN_WIDTH - thickness, pos_y, thickness,
                  VIDEO_SCREEN_HEIGHT, color); // right
  }

  void render_ui() {
    ClearBackground(BLACK);
    DrawText("WOMP WOMP!", 0, 0, 28, LIGHTGRAY);
  }

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
