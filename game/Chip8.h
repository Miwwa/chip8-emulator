#pragma once

#include "Game.h"
#include "../chip8cpp/Chip8Core.h"

class Chip8 final : public Game
{
private:
    std::optional<chip8::Chip8Core> core;
    bool is_emulation_running = false;

    SDL_Surface* screen_surface = nullptr;
    SDL_Texture* screen_texture = nullptr;

    void render_menu();
    
public:
    Chip8(int argc, char* argv[]) : Game(
        argc, argv, window::WindowCreateInfo{
            .title = "Chip8 Emulator",
            .width = 1280,
            .height = 640,
        }
    )
    {
    }

    void init() override;
    void process_sdl_event(const SDL_Event& event) override;
    void fixed_update() override;
    void render() override;
};
