#pragma once

#include "Game.h"
#include "Resolution.h"
#include "../chip8cpp/Chip8Core.h"

namespace chip8
{
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

        void toggle_emulation();
        
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
