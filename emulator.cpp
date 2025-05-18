#include <SDL2/SDL.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "./include/chip8.hpp"

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

namespace SDL {

bool Init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "failed to init SDL: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  return true;
}

SDL_Window *CreateWindow(const char *title, const int baseWidth,
                         const int baseHeight) {
  SDL_Window *window = SDL_CreateWindow(
      "Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, baseWidth,
      baseHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

  if (!window) {
    std::cerr << "failed to create window: " << SDL_GetError() << std::endl;
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  SDL_MaximizeWindow(window);

  return window;
}

SDL_Renderer *CreateRenderer(SDL_Window *window) {
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "failed to create renderer: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }
  return renderer;
}

float GetDisplayScale() {
  float scale = 1.0f;
  float ddpi, hdpi = 96.0f, vdpi;
  if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
    std::cerr << "failed to get display DPI: " << SDL_GetError() << std::endl;
  } else {
    scale = hdpi / 96.0f;
    std::cout << "hdpi: " << hdpi << std::endl;
    std::cout << "scale: " << scale << std::endl;
  }

  return scale;
}

struct LogicalDimensions {
  int WIDTH;
  int HEIGHT;
};

LogicalDimensions SetupLogicalWidthHeight(SDL_Renderer *renderer, float scale) {
  int drawableWidth, drawableHeight;
  SDL_GetRendererOutputSize(renderer, &drawableWidth, &drawableHeight);
  std::cout << "drawable width: " << drawableWidth << " "
            << "drawable height: " << drawableHeight << std::endl;

  const int WIDTH = std::round(drawableWidth / scale);
  const int HEIGHT = std::round(drawableHeight / scale);

  SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
  std::cout << "logical width: " << WIDTH << " "
            << "logical height: " << HEIGHT << std::endl;

  return {WIDTH, HEIGHT};
}

} // namespace SDL

int main(int argc, char *args[]) {

  const int baseWidth = 1280;
  const int baseHeight = 720;

  SDL_Window *window = nullptr;

  if (SDL::Init()) {
    window = SDL::CreateWindow("My Window", baseWidth, baseHeight);
  }

  SDL_Renderer *renderer = SDL::CreateRenderer(window);

  // get scale
  float scale = SDL::GetDisplayScale();

  // set logical size based on scale
  auto dimensions = SDL::SetupLogicalWidthHeight(renderer, scale);
  const int WIDTH = dimensions.WIDTH;
  const int HEIGHT = dimensions.HEIGHT;

  bool running = true;
  SDL_Event event;

  // ================================================================
  // RENDERING LOGIC
  // ================================================================
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  const int VW = 64;
  const int VH = 32;

  const int GRID_W = (WIDTH / 1.5) / 64;
  const int GRID_H = GRID_W;

  const int SCREEN_W = GRID_W * VW;
  const int SCREEN_H = GRID_H * VH;

  const int SCREEN_X_OFF = 10;
  const int SCREEN_Y_OFF = 10;

  std::cout << "GRID W: " << GRID_W << " " << "GRID H: " << GRID_H << std::endl;
  std::cout << "SCREEN W: " << SCREEN_W << " " << "SCREEN H: " << SCREEN_H
            << std::endl;

  auto ibm = LoadRomFromFile("./roms/test/octojam.ch8");

  Chip8 cpu;

  cpu.LoadFromArray(ibm.data(), ibm.size());

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }

      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_q:
          running = false;
          break;
        }
      }
    }

    SDL_RenderClear(renderer);

    for (size_t y = 0; y < VH; y += 1) {
      for (size_t x = 0; x < VW; x += 1) {
        size_t index = y * VW + x;
        uint8_t pixel = cpu.video[index];

        int rx = x * GRID_W + SCREEN_X_OFF;
        int ry = y * GRID_H + SCREEN_Y_OFF;
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
    SDL_Rect border{SCREEN_X_OFF - padding, SCREEN_Y_OFF - padding,
                    SCREEN_W + 2 * padding, SCREEN_H + 2 * padding};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &border);

    // SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);

    cpu.Cycle();

    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
