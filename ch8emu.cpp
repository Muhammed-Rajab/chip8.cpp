#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "./include/chip8.hpp"
#include "./include/sdl/sdl.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_renderer.h"

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
  // SDL2
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  float SCREEN_SCALE = 1;
  int SCREEN_WIDTH = 0;
  int SCREEN_HEIGHT = 0;

  bool quit = false;

  // Chip8
  Chip8 &cpu;

  // video
  int GRID_W{};
  int GRID_H{};

  int VIDEO_W{};
  int VIDEO_H{};

  int VIDEO_X_OFF = 0;
  int VIDEO_Y_OFF = 0;

  struct nk_context *ctx;

public:
  App(Chip8 &cpu) : cpu(cpu) {

    // Setup window
    const int baseWidth = 1280;
    const int baseHeight = 720;
    if (SDL::Init()) {
      window = SDL::CreateWindow("My Window", baseWidth, baseHeight);
    }

    // Setup renderer
    renderer = SDL::CreateRenderer(window);

    // get scale
    SCREEN_SCALE = SDL::GetDisplayScale();

    auto dimensions = SDL::SetupLogicalWidthHeight(renderer, SCREEN_SCALE);
    SCREEN_WIDTH = dimensions.WIDTH;
    SCREEN_HEIGHT = dimensions.HEIGHT;

    GRID_W = (SCREEN_WIDTH) / 64;
    GRID_H = GRID_W;

    VIDEO_W = GRID_W * cpu.VIDEO_WIDTH;
    VIDEO_H = GRID_H * cpu.VIDEO_HEIGHT;

    ctx = nk_sdl_init(window, renderer);

    struct nk_font_atlas *atlas;

    nk_sdl_font_stash_begin(&atlas);

    struct nk_font *font = nk_font_atlas_add_default(atlas, 18.0f, 0);

    nk_sdl_font_stash_end();

    nk_style_set_font(ctx, &font->handle);
  }

  ~App() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  // handle inputs
  void handle_inputs() {
    SDL_Event event;

    nk_input_begin(ctx);

    while (SDL_PollEvent(&event)) {

      nk_sdl_handle_event(&event);

      switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          quit = true;
          break;
        case SDLK_x:
          cpu.keypad[0x0] = 1;
          break;
        case SDLK_1:
          cpu.keypad[0x1] = 1;
          break;
        case SDLK_2:
          cpu.keypad[0x2] = 1;
          break;
        case SDLK_3:
          cpu.keypad[0x3] = 1;
          break;
        case SDLK_q:
          cpu.keypad[0x4] = 1;
          break;
        case SDLK_w:
          cpu.keypad[0x5] = 1;
          break;
        case SDLK_e:
          cpu.keypad[0x6] = 1;
          break;
        case SDLK_a:
          cpu.keypad[0x7] = 1;
          break;
        case SDLK_s:
          cpu.keypad[0x8] = 1;
          break;
        case SDLK_d:
          cpu.keypad[0x9] = 1;
          break;
        case SDLK_z:
          cpu.keypad[0xA] = 1;
          break;
        case SDLK_c:
          cpu.keypad[0xB] = 1;
          break;
        case SDLK_4:
          cpu.keypad[0xC] = 1;
          break;
        case SDLK_r:
          cpu.keypad[0xD] = 1;
          break;
        case SDLK_f:
          cpu.keypad[0xE] = 1;
          break;
        case SDLK_v:
          cpu.keypad[0xF] = 1;
          break;
        }
        break;

      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_x:
          cpu.keypad[0x0] = 0;
          break;
        case SDLK_1:
          cpu.keypad[0x1] = 0;
          break;
        case SDLK_2:
          cpu.keypad[0x2] = 0;
          break;
        case SDLK_3:
          cpu.keypad[0x3] = 0;
          break;
        case SDLK_q:
          cpu.keypad[0x4] = 0;
          break;
        case SDLK_w:
          cpu.keypad[0x5] = 0;
          break;
        case SDLK_e:
          cpu.keypad[0x6] = 0;
          break;
        case SDLK_a:
          cpu.keypad[0x7] = 0;
          break;
        case SDLK_s:
          cpu.keypad[0x8] = 0;
          break;
        case SDLK_d:
          cpu.keypad[0x9] = 0;
          break;
        case SDLK_z:
          cpu.keypad[0xA] = 0;
          break;
        case SDLK_c:
          cpu.keypad[0xB] = 0;
          break;
        case SDLK_4:
          cpu.keypad[0xC] = 0;
          break;
        case SDLK_r:
          cpu.keypad[0xD] = 0;
          break;
        case SDLK_f:
          cpu.keypad[0xE] = 0;
          break;
        case SDLK_v:
          cpu.keypad[0xF] = 0;
          break;
        }
        break;
      }
    }

    nk_input_end(ctx);
  }

  // render video
  void render_video(int px, int py) {
    for (size_t y = 0; y < cpu.VIDEO_HEIGHT; y += 1) {
      for (size_t x = 0; x < cpu.VIDEO_WIDTH; x += 1) {
        size_t index = y * cpu.VIDEO_WIDTH + x;
        uint8_t pixel = cpu.video[index];

        int rx = px + x * GRID_W + VIDEO_X_OFF;
        int ry = py + y * GRID_H + VIDEO_Y_OFF;
        SDL_Rect rect{rx, ry, GRID_W, GRID_H};

        if (pixel) {
          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
          SDL_RenderFillRect(renderer, &rect);
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
          SDL_RenderDrawRect(renderer, &rect);
        } else {
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }

    // DRAW A SCREEN BORDER
    int padding = 5;
    SDL_Rect border{px + VIDEO_X_OFF - padding, py + VIDEO_Y_OFF - padding,
                    VIDEO_W + 2 * padding, VIDEO_H + 2 * padding};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &border);
  }

  void render_ui() {
    if (nk_begin(ctx, "Emulator UI", nk_rect(50, 50, 220, 300),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
      nk_layout_row_static(ctx, 30, 200, 1);
      nk_label(ctx, "Hello, emulator!", NK_TEXT_LEFT);
      if (nk_button_label(ctx, "Reset CPU")) {
        cpu.Reset();
      }
    }
    nk_end(ctx);
  }

  void render() {
    render_ui();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    render_video((SCREEN_WIDTH - VIDEO_W) / 2, (SCREEN_HEIGHT - VIDEO_H) / 2);

    nk_sdl_render(NK_ANTI_ALIASING_ON);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPresent(renderer);
  }

  bool Quit() const { return quit; }
};

int main(int argc, char *args[]) {

  auto ibm = LoadRomFromFile("./roms/test/octojam.ch8");

  Chip8 cpu;

  cpu.LoadFromArray(ibm.data(), ibm.size());

  App app(cpu);

  while (!app.Quit()) {

    auto frame_start = std::chrono::high_resolution_clock::now();

    app.handle_inputs();

    app.render();

    cpu.Cycle();

    auto frame_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = frame_end - frame_start;
    float frame_time = elapsed.count();
    float target_time = 1.0f / 60.0f;

    if (frame_time < target_time) {
      std::this_thread::sleep_for(
          std::chrono::duration<float>(target_time - frame_time));
    }
  }

  return EXIT_SUCCESS;
}
