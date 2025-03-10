#include "Chip8.h"

#include <unordered_map>

#include "SDL3/SDL.h"
#include <imgui.h>

#include "platform/filesystem.h"
#include "platform/SdlException.h"

namespace
{
    const std::unordered_map<SDL_Keycode, uint8_t> key_map = {
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_Q, 0x4}, {SDLK_W, 0x5}, {SDLK_E, 0x6}, {SDLK_R, 0xD},
        {SDLK_A, 0x7}, {SDLK_S, 0x8}, {SDLK_D, 0x9}, {SDLK_F, 0xE},
        {SDLK_Z, 0xA}, {SDLK_X, 0x0}, {SDLK_C, 0xB}, {SDLK_V, 0xF},
    };

    constexpr std::array palette = {0xff382b26, 0xffb8c2b9};
}

namespace chip8
{
    void Chip8::init()
    {
        screen_surface = SDL_CreateSurface(chip8::screen_width, chip8::screen_height, SDL_PIXELFORMAT_RGBA8888);
        screen_texture = SDL_CreateTextureFromSurface(renderer, screen_surface);
        SDL_SetTextureScaleMode(screen_texture, SDL_SCALEMODE_NEAREST);


        std::string file_to_load;
        if (args.size() > 1)
        {
            // Get the ROM file path
            file_to_load = args[1];
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

    void Chip8::process_sdl_event(const SDL_Event& event)
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

    void Chip8::render_menu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open File", "Ctrl + O", false, true))
                {
                    SDL_Log("Open File pressed");
                }
                if (ImGui::MenuItem("Close", nullptr, false, true))
                {
                    SDL_Log("Close pressed");
                }
                if (ImGui::BeginMenu("Recent files", false))
                {
                    ImGui::EndMenu();
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Save state", "F5", false, true))
                {
                    SDL_Log("Save state pressed");
                }
                if (ImGui::MenuItem("Load state", "F8", false, true))
                {
                    SDL_Log("Load state pressed");
                }
                ImGui::Separator();

                if (ImGui::BeginMenu("Quick Save", true))
                {
                    for (int i = 0; i < 10; i++)
                    {
                        std::string label = std::to_string(i);
                        std::string shortcut = "Shift + " + std::to_string(i);
                        if (ImGui::MenuItem(label.c_str(), shortcut.c_str(), false, true))
                        {
                            SDL_Log("Quick Save %d pressed", i);
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Quick Load", true))
                {
                    for (int i = 0; i < 10; i++)
                    {
                        std::string label = std::to_string(i);
                        std::string shortcut = std::to_string(i);
                        if (ImGui::MenuItem(label.c_str(), shortcut.c_str(), false, true))
                        {
                            SDL_Log("Quick Load %d pressed", i);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "Alt + F4", false, true))
                {
                    SDL_Log("Exit pressed");
                    is_emulation_running = false;
                    should_quit = true;
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            if (ImGui::BeginMenu("Machine"))
            {
                if (ImGui::MenuItem("Quirks", nullptr, false, true))
                {
                    SDL_Log("Quirks pressed");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Pause", "P", false, true))
                {
                    SDL_Log("Pause pressed");
                }
                if (ImGui::MenuItem("Reset", nullptr, false, true))
                {
                    SDL_Log("Reset pressed");
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            if (ImGui::BeginMenu("Window resolution", true))
            {
                for (uint32_t i = 0; i < available_resolutions.size(); i++)
                {
                    const auto& resolution = available_resolutions[i];
                    std::string label = std::format("{}x: {}x{}", i + 1, resolution.x, resolution.y);
                    if (ImGui::MenuItem(label.c_str(), nullptr, false, true))
                    {
                        if (!SDL_SetWindowSize(window, resolution.x, resolution.y))
                        {
                            throw sdl::SdlException("Window size change error");
                        }
                        current_resolution = resolution;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            if (ImGui::BeginMenu("Color palette", false))
            {
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void Chip8::render_screen() const
    {
        if (!core.has_value())
        {
            return;
        }

        for (uint16_t y = 0; y < chip8::screen_height; y++)
        {
            for (uint16_t x = 0; x < chip8::screen_width; x++)
            {
                uint16_t pixel_index = y * chip8::screen_width + x;
                auto pixel_value = core->get_state().display[pixel_index];
                uint32_t pixel_color = palette[pixel_value];
                uint32_t* pixels = static_cast<uint32_t*>(screen_surface->pixels);
                pixels[pixel_index] = pixel_color;
            }
        }

        SDL_UpdateTexture(screen_texture, nullptr, screen_surface->pixels, screen_surface->pitch);
        SDL_RenderTexture(renderer, screen_texture, nullptr, nullptr);
    }

    void Chip8::render()
    {
        render_menu();
        render_screen();
    }
}
