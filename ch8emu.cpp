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

std::string hex_to_string(size_t val, size_t width) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::setfill('0') << std::setw(width)
      << size_t(val);
  return oss.str();
}

using Clock = std::chrono::high_resolution_clock;

enum class EmulatorModes { Normal, Debug };

constexpr int WINDOW_WIDTH = 955;
constexpr int WINDOW_HEIGHT = 500;

struct EmulatorTheme {
  Color background;
  Color video_pixel;
  Color text;
  Color disabled_text;
  Color border;
  Color stack_pointer;
  Color current_instruction;
  Color controls_overlay_backgroud;
  Color controls_overlay_text;
};

namespace EmulatorThemes {
constexpr EmulatorTheme DEFAULT = {BLACK, WHITE, WHITE, DARKGRAY, GRAY,
                                   RED,   RED,   BLACK, WHITE};
}

class Emulator {
public:
  Emulator(Chip8 &cpu, EmulatorModes emulator_mode)
      : cpu(cpu), mode(emulator_mode),
        disassembled_rom(
            Disassembler::DecodeRomFromArrayAsVector(cpu.rom, false)) {
    initialize_raylib();
    initialize_video_settings();
  }

  void Run() {

    auto last_timer_tick = Clock::now();

    while (!WindowShouldClose()) {

      handle_cpu_input();
      handle_ui_input();

      // ====== Cycles and Timers ======
      if (!paused)
        execute_cycles();

      // ====== Timer update at 60HZ ======
      update_timers(last_timer_tick);

      // ====== Rendering ======
      render();

      // ====== Sound ======
      handle_sound();
    }
  }

  ~Emulator() {
    UnloadSound(beep);
    UnloadFont(fontTTF);
    CloseWindow();
    CloseAudioDevice();
  }

private:
  // Chip8
  Chip8 &cpu;
  std::vector<std::string> disassembled_rom;

  // Emulator state
  bool paused = false;
  int cycles_per_frame = 15;
  bool showControlsOverlay = false;
  EmulatorModes mode = EmulatorModes::Debug;
  EmulatorTheme theme = EmulatorThemes::DEFAULT;

  // video
  int VIDEO_SCREEN_WIDTH = 600;
  int VIDEO_SCREEN_HEIGHT = 0;
  int VIDEO_X_COUNT = 0;
  int VIDEO_Y_COUNT = 0;
  int VIDEO_GRID_SIZE = 0;

  // raylib resources
  Font fontTTF;
  Sound beep;

  // ====== Initializations ======
  void initialize_raylib() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT |
                   FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "");
    SetTargetFPS(60);

    fontTTF = LoadFontEx("./fonts/scp-bold.ttf", 128, 0, 0);

    InitAudioDevice();
    beep = LoadSound("./sounds/sine.wav");

    SetTextureFilter(fontTTF.texture, TEXTURE_FILTER_BILINEAR);
  }

  void initialize_video_settings() {
    if (mode == EmulatorModes::Normal) {
      VIDEO_SCREEN_WIDTH = WINDOW_WIDTH - 40;
      cycles_per_frame = 15;
    } else {
      VIDEO_SCREEN_WIDTH = 600;
      cycles_per_frame = 15;
    }

    VIDEO_X_COUNT = cpu.VIDEO_WIDTH;
    VIDEO_Y_COUNT = cpu.VIDEO_HEIGHT;

    VIDEO_GRID_SIZE = VIDEO_SCREEN_WIDTH / VIDEO_X_COUNT;
    VIDEO_SCREEN_HEIGHT = VIDEO_GRID_SIZE * VIDEO_Y_COUNT;
  }

  // ====== Input Handling ======
  void handle_ui_input() {
    if (IsKeyPressed(KEY_SPACE)) {
      showControlsOverlay = !showControlsOverlay;
    }

    if (mode == EmulatorModes::Debug) {
      if (IsKeyPressed(KEY_LEFT_BRACKET)) {
        cycles_per_frame -= 1;
        if (cycles_per_frame < 1)
          cycles_per_frame = 1;
      } else if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
        cycles_per_frame += 1;
      } else if (IsKeyPressed(KEY_P)) {
        paused = !paused;
      } else if (IsKeyPressed(KEY_N) && paused) {
        execute_cycles();
      }
    }
  }

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

  void handle_sound() {
    if (cpu.sound > 0 && !IsSoundPlaying(beep)) {
      PlaySound(beep);
    }
  }

  // ====== Execution ======
  void execute_cycles() {
    for (int i = 0; i < cycles_per_frame; i += 1) {
      cpu.Cycle();
    }
  }

  void update_timers(std::chrono::time_point<Clock> &last_timer_tick) {
    auto now = Clock::now();
    auto elapsed = std::chrono::duration<float>(now - last_timer_tick);
    if (elapsed.count() >= (1.0f / 60.0f)) {
      cpu.UpdateTimers();
      last_timer_tick = now;
    }
  }

  // ====== Rendering ======
  void render() {
    // ====== Mode-based rendering ======
    if (mode == EmulatorModes::Normal) {
      render_normal();

    } else {
      DrawFPS(10, 10);
      render_debug();
    }
  }

  void render_debug() {

    BeginDrawing();
    ClearBackground(theme.background);

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

    int doy = 20;
    render_disassembled_code_with_pc_opcode_and_instructions(
        vox, VIDEO_SCREEN_HEIGHT + voy + doy);

    // special register stuff
    int iox = 330;
    render_index_and_special_registers(vox + iox,
                                       VIDEO_SCREEN_HEIGHT + voy + doy);

    int poy = 10;
    render_debugger_params(vox + iox, VIDEO_SCREEN_HEIGHT + voy + doy + poy);

    // help message
    int hox = 0;
    int hoy = 140;
    DrawTextEx(fontTTF, "press [SPACE] to show debugger shortcuts",
               {(float)vox + hox, (float)VIDEO_SCREEN_HEIGHT + voy + doy + hoy},
               14, 0, theme.disabled_text);

    // render shortcuts
    if (showControlsOverlay) {
      render_controls_overlay();
    }

    EndDrawing();
  }

  void render_normal() {

    BeginDrawing();
    ClearBackground(theme.background);

    float vox = 20;
    float voy = 20;
    render_video(vox, voy);

    float hox = 0;
    float hoy = 10;
    DrawTextEx(fontTTF, "press [SPACE] to show emulator shortcuts",
               {vox + hox, (float)VIDEO_SCREEN_HEIGHT + voy + hoy}, 14, 0,
               theme.disabled_text);

    // render shortcuts
    if (showControlsOverlay) {
      render_controls_overlay();
    }
    EndDrawing();
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
          DrawRectangleRec(rec, theme.video_pixel);
          DrawRectangleLinesBetter(rec, 1, theme.background);
        }
      }
    }

    // draw border
    const Rectangle border = {(float)px, (float)py, (float)VIDEO_SCREEN_WIDTH,
                              (float)VIDEO_SCREEN_HEIGHT};
    DrawRectangleLinesBetter(border, 1, theme.border);
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
    DrawRectangleLinesBetter(border, 1, theme.border);
  }

  void render_registers(float px, float py) {

    const int line_height = 25;
    const int column_spacing = 110;
    const int text_size = 20;

    px += 10;
    py += 10;

    DrawTextEx(fontTTF, "Registers", {(float)px, (float)py}, text_size, 0,
               theme.disabled_text);

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
                 theme.text);
    }

    DrawRectangleLinesBetter({px - 10, py - 10, 207, py + line_height * 8 + 7},
                             1, theme.border);
  }

  void render_index_and_special_registers(float px, float py) {
    const std::string str = "I: " + hex_to_string(cpu.index, 4) + " " +
                            "DT: " + hex_to_string(cpu.delay, 2) + " " +
                            "ST: " + hex_to_string(cpu.sound, 2);

    DrawTextEx(fontTTF, str.c_str(), {px, py}, 20, 0, theme.text);
  }

  void render_stack(float px, float py) {

    const int line_height = 25;

    py += 10;

    const auto stack_size = MeasureTextEx(fontTTF, "##Stack##", 20, 0);
    DrawTextEx(fontTTF, "  Stack  ", {(float)px, (float)py}, 20, 0,
               theme.disabled_text);

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
                      val_size.x, val_size.y, theme.stack_pointer);
      }

      DrawTextEx(fontTTF, str.c_str(),
                 {(float)px + (stack_size.x - val_size.x) / 2, (float)fy}, 20,
                 0, theme.text);

      fy += line_height;
    }

    DrawRectangleLinesBetter({px, py - 10, stack_size.x, fy - line_height + 27},
                             1, theme.border);
  }

  void render_disassembled_code_with_pc_opcode_and_instructions(float px,
                                                                float py) {
    const int line_height = 30;
    const int current_index = (cpu.pc - 0x200) / 2;

    py += 5;

    DrawTextEx(fontTTF, "PC", {px + 10, py}, 20, 0, theme.disabled_text);

    DrawTextEx(fontTTF, "OPCODE", {px + 70, py}, 20, 0, theme.disabled_text);

    DrawTextEx(fontTTF, "INSTRUCTION", {px + 140, py}, 20, 0,
               theme.disabled_text);

    py += line_height;

    // Display one before, current, one after
    for (int i = -1; i <= 1; ++i) {
      const int index = current_index + i;

      if (index < 0 || index >= (int)disassembled_rom.size())
        continue;

      const std::string &line = disassembled_rom[index];
      const Color color = (i == 0) ? theme.current_instruction : theme.text;

      const std::string label = hex_to_string(0x200 + index * 2, 3);

      const std::string opcode = hex_to_string(cpu.opcode, 4);

      DrawTextEx(fontTTF, label.c_str(), {px + 10, py + (i + 1) * line_height},
                 20, 0, color);

      DrawTextEx(fontTTF, opcode.c_str(), {px + 70, py + (i + 1) * line_height},
                 20, 0, color);

      DrawTextEx(fontTTF, line.c_str(), {px + 140, py + (i + 1) * line_height},
                 20, 0, color);
    }

    DrawRectangleLinesBetter({px, py - (5 + line_height), (float)315, 130}, 1,
                             theme.border);
  }

  void render_debugger_params(float px, float py) {

    const int line_height = 30;

    py += 20;

    // cycles per frame
    std::string cycles_per_frame_str =
        "cycles per frame: " + std::to_string(cycles_per_frame);
    DrawTextEx(fontTTF, cycles_per_frame_str.c_str(), {px, py}, 20, 0,
               theme.text);
  }

  void render_controls_overlay() {
    float width = 300;
    float height = 140;
    float x = WINDOW_WIDTH - width;
    float y = WINDOW_HEIGHT - height;
    Rectangle rec = {x, y, width, height};
    DrawRectangleRec(rec, theme.controls_overlay_backgroud);

    const float line_height = 20;

    x += 10;
    y += 10;

    DrawTextEx(fontTTF, "Shortcuts", {x, y}, 16, 0, theme.disabled_text);

    y += 20;

    if (mode == EmulatorModes::Debug) {

      DrawTextEx(fontTTF, "[ : decrease cpu cycles per frame", {x, y}, 16, 0,
                 theme.controls_overlay_text);
      y += line_height;
      DrawTextEx(fontTTF, "] : increase cpu cycles per frame", {x, y}, 16, 0,
                 theme.controls_overlay_text);
      y += line_height;
      DrawTextEx(fontTTF, "p : pause/resume cpu cycle", {x, y}, 16, 0,
                 theme.controls_overlay_text);
      y += line_height;
      DrawTextEx(fontTTF, "n : run next cycle (if paused)", {x, y}, 16, 0,
                 theme.controls_overlay_text);
      y += line_height;
    } else if (mode == EmulatorModes::Normal) {
      DrawTextEx(fontTTF, "ESC : quit", {x, y}, 16, 0,
                 theme.controls_overlay_text);
    }

    DrawRectangleLinesBetter(rec, 1, theme.border);
  }
};

// ====== CLI ======
namespace CLI {
void print_usage(const std::string &programName) {
  std::cout << "ch8emu Usage:\n";
  std::cout << "  " << programName << " <rom_path> [options]\n\n";
  std::cout << "options:\n";
  std::cout << "  -m, --mode <mode>    set emulator mode (debug or normal, "
               "default: debug)\n";
  std::cout << "  -h, --help           show this help message\n";
}

EmulatorModes parse_mode(const std::string &modeStr) {
  std::string lowerMode = modeStr;
  std::transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (lowerMode == "normal") {
    return EmulatorModes::Normal;
  } else if (lowerMode == "debug") {
    return EmulatorModes::Debug;
  }
  throw std::invalid_argument("invalid mode. must be 'debug' or 'normal'");
}
} // namespace CLI

int main(int argc, char *args[]) {
  std::string romPath;
  EmulatorModes mode = EmulatorModes::Debug; // Default mode

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = args[i];

    if (arg == "-h" || arg == "--help") {
      CLI::print_usage(args[0]);
      return EXIT_SUCCESS;
    } else if (arg == "-m" || arg == "--mode") {
      if (i + 1 >= argc) {
        std::cerr << "Error: Missing argument for mode\n";
        CLI::print_usage(args[0]);
        return EXIT_FAILURE;
      }
      try {
        mode = CLI::parse_mode(args[++i]);
      } catch (const std::invalid_argument &e) {
        std::cerr << "Error: " << e.what() << "\n";
        CLI::print_usage(args[0]);
        return EXIT_FAILURE;
      }
    } else {
      // Assume this is the ROM path
      if (romPath.empty()) {
        romPath = arg;
      } else {
        std::cerr << "Error: Unexpected argument '" << arg << "'\n";
        CLI::print_usage(args[0]);
        return EXIT_FAILURE;
      }
    }
  }

  if (romPath.empty()) {
    std::cerr << "Error: No ROM path specified\n";
    CLI::print_usage(args[0]);
    return EXIT_FAILURE;
  }

  try {
    auto rom = LoadRomFromFile(romPath);
    Chip8 cpu;
    cpu.LoadFromArray(rom.data(), rom.size());

    Emulator emu(cpu, mode);
    emu.Run();

    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
