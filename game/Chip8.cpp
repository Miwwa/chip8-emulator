#include "Chip8.h"

#include <unordered_map>

#include "SDL3/SDL.h"
#include <imgui.h>

namespace
{
    const std::unordered_map<SDL_Keycode, uint8_t> key_map = {
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_Q, 0x4}, {SDLK_W, 0x5}, {SDLK_E, 0x6}, {SDLK_R, 0xD},
        {SDLK_A, 0x7}, {SDLK_S, 0x8}, {SDLK_D, 0x9}, {SDLK_F, 0xE},
        {SDLK_Z, 0xA}, {SDLK_X, 0x0}, {SDLK_C, 0xB}, {SDLK_V, 0xF},
    };
}

void Chip8::init()
{
}

void Chip8::process_sdl_event(SDL_Event& event)
{
    if (!core.has_value() || !is_emulation_running)
    {
        return;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if (key_map.contains(event.key.key))
        {
            auto key_index = key_map.at(event.key.key);
            core->set_key(key_index, true);
        }
    }
    if (event.type == SDL_EVENT_KEY_UP)
    {
        if (key_map.contains(event.key.key))
        {
            auto key_index = key_map.at(event.key.key);
            core->set_key(key_index, false);
        }
    }
}

void Chip8::fixed_update()
{
    if (!core.has_value() || !is_emulation_running)
    {
        return;
    }

    constexpr uint32_t cycles_per_second = 700;
    uint64_t cycles_to_emulate = fixed_delta_time_ns / cycles_per_second;
    for (uint32_t i = 0; i < cycles_to_emulate; i++)
    {
        core->emulate_cycle();
    }
    core->timers_tick();
}

void Chip8::render()
{
    if (!core.has_value())
    {
        return;
    }

    bool is_window_open = true;
    ImGui::Begin("Chip8 Display", &is_window_open, ImGuiWindowFlags_AlwaysAutoResize);
    for (uint16_t y = 0; y < chip8::screen_height; y++)
    {
        for (uint16_t x = 0; x < chip8::screen_width; x++)
        {
            uint16_t pixel_index = (y * chip8::screen_width) + x;
            const std::string s = core->get_state().display[pixel_index] == 1 ? "X" : " ";
            ImGui::SameLine();
            ImGui::Text(s.c_str());
        }
        ImGui::NewLine();
    }
    ImGui::End();
}
