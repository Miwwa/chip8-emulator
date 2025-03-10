#pragma once

#include "Game.h"
#include "../chip8cpp/Chip8Core.h"

namespace chip8
{
    struct Resolution
    {
        int32_t x, y;
    };

    constexpr std::array available_resolutions = {
        Resolution{.x = 640, .y = 320},
        Resolution{.x = 1280, .y = 640},
        Resolution{.x = 1920, .y = 960},
    };

    class Chip8 final : public Game
    {
    private:
        std::optional<Chip8Core> core;
        bool is_emulation_running = false;

        SDL_Surface* screen_surface = nullptr;
        SDL_Texture* screen_texture = nullptr;

        Resolution current_resolution;

        void init() override;
        void process_sdl_event(const SDL_Event& event) override;
        void fixed_update() override;
        void render() override;

        void render_menu();
        void render_screen() const;

    public:
        Chip8(int argc, char* argv[]): Game(argc, argv)
        {
            current_resolution = available_resolutions[0];
            
            SDL_SetWindowTitle(window, "Chip8 Emulator");
            SDL_SetWindowSize(window, current_resolution.x, current_resolution.y);
        }
    };
}
