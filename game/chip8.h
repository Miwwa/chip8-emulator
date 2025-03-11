#pragma once

#include <filesystem>

#include "../chip8cpp/chip8_core.h"
#include "game.h"

namespace chip8 {
    struct Resolution {
        int32_t x, y;
    };

    struct ColorPalette {
        std::array<uint32_t, 2> colors;
        std::string_view name;
    };

    class Chip8 final : public Game {
        SDL_Surface* screen_surface = nullptr;
        SDL_Texture* screen_texture = nullptr;

        std::optional<Chip8Core> core;
        bool is_emulation_running = false;
        bool should_show_main_menu = false;

        std::optional<std::filesystem::path> current_rom;
        Resolution current_resolution;
        ColorPalette current_palette;
        float main_menu_height = 0;

        void init() override;
        void process_sdl_event(const SDL_Event& event) override;
        void fixed_update() override;
        void render() override;

        void start_open_file_dialog();
        void load_rom(const std::filesystem::path& filepath);
        void close_rom();
        void toggle_emulation();
        void reset_emulation();

        void render_screen() const;
        void render_menu();

      public:
        Chip8(int argc, char* argv[]);
    };
} // namespace chip8
