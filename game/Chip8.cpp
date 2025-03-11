#include "Chip8.h"

#include <filesystem>
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
            load_rom(std::filesystem::path(file_to_load));
        }
    }

    void Chip8::process_sdl_event(const SDL_Event& event)
    {
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (core.has_value() && is_emulation_running)
            {
                if (key_map.contains(event.key.key))
                {
                    auto key_index = key_map.at(event.key.key);
                    core->set_key(key_index, true);
                }
            }
        }
        if (event.type == SDL_EVENT_KEY_UP)
        {
            if (core.has_value() && is_emulation_running)
            {
                if (key_map.contains(event.key.key))
                {
                    auto key_index = key_map.at(event.key.key);
                    core->set_key(key_index, false);
                }
            }

            if (event.key.key == SDLK_P)
            {
                toggle_emulation();
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
                    // todo: open rom
                    SDL_Log("Open File pressed");
                }
                if (ImGui::MenuItem("Close", nullptr, false, current_rom.has_value()))
                {
                    close_rom();
                }
                if (ImGui::BeginMenu("Recent files", false))
                {
                    ImGui::EndMenu();
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Save state", "F5", false, true))
                {
                    // todo: save custom state
                    SDL_Log("Save state pressed");
                }
                if (ImGui::MenuItem("Load state", "F8", false, true))
                {
                    // todo: load custom state
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
                            // todo: quick save
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
                            // todo: quick load
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
                    // todo: quirks settings
                    SDL_Log("Quirks pressed");
                }
                ImGui::Separator();

                std::string pause_label = is_emulation_running ? "Pause" : "Play";
                if (ImGui::MenuItem(pause_label.c_str(), "P", false, current_rom.has_value()))
                {
                    toggle_emulation();
                }
                if (ImGui::MenuItem("Reset", nullptr, false, current_rom.has_value()))
                {
                    reset_emulation();
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
                        set_window_size(resolution.x, resolution.y);
                        current_resolution = resolution;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            if (ImGui::BeginMenu("Color palette", false))
            {
                // todo: color palettes
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

        for (uint16_t y = 0; y < screen_height; y++)
        {
            for (uint16_t x = 0; x < screen_width; x++)
            {
                uint16_t pixel_index = y * screen_width + x;
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

    void Chip8::load_rom(const std::filesystem::path& filepath)
    {
        assert(std::filesystem::exists(filepath) && "File not exists");
        auto rom = platform::read_file(filepath);
        if (rom.has_value())
        {
            core = Chip8Core(rom.value());
            current_rom = filepath;
            is_emulation_running = true;
        }
    }

    void Chip8::close_rom()
    {
        core.reset();
        current_rom.reset();
        is_emulation_running = false;
    }

    void Chip8::toggle_emulation()
    {
        is_emulation_running = !is_emulation_running;
    }

    void Chip8::reset_emulation()
    {
        if (current_rom.has_value())
        {
            load_rom(current_rom.value());
        }
    }
}
