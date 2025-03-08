#pragma once

#include "Game.h"
#include "../chip8cpp/Chip8Core.h"
#include "platform/filesystem.h"

class Chip8 final : public Game
{
private:
    std::optional<chip8::Chip8Core> core;
    bool is_emulation_running = false;

    SDL_Surface* screen_surface = nullptr;
    SDL_Texture* screen_texture = nullptr;
    
public:
    Chip8(int argc, char* argv[]) : Game(argc, argv)
    {
        std::string file_to_load;
        if (argc == 2)
        {
            // Get the ROM file path
            file_to_load = argv[1];
            SDL_Log("ROM file path: %s", file_to_load.c_str());
        }

        if (!file_to_load.empty())
        {
            auto rom = platform::read_file(file_to_load.c_str());
            if (rom.has_value())
            {
                core = chip8::Chip8Core(rom.value());
                is_emulation_running = true;
            }
        }
    }

    void init() override;
    void process_sdl_event(SDL_Event& event) override;
    void fixed_update() override;
    void render() override;
};
