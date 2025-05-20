#ifndef SDL_4_CHIP8_HPP
#define SDL_4_CHIP8_HPP

#include <SDL2/SDL.h>
#include <iostream>

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

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); // Not "linear"
  SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
  std::cout << "logical width: " << WIDTH << " "
            << "logical height: " << HEIGHT << std::endl;

  return {WIDTH, HEIGHT};
}
} // namespace SDL

#endif
