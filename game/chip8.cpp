#include "chip8.h"

#include <filesystem>
#include <unordered_map>

#include "imgui.h"
#include "platform/filesystem.h"
#include "platform/sdl_exception.h"
#include "SDL3/SDL.h"

namespace chip8 {
    constexpr std::array available_resolutions = {
        Resolution{.x = 640, .y = 320},
        Resolution{.x = 1280, .y = 640},
        Resolution{.x = 1920, .y = 960},
    };

    constexpr std::array available_palettes = {
        ColorPalette{.colors = {0xff2e3037, 0xffebe5ce}, .name = "IBM 8503"},
        ColorPalette{.colors = {0xff3e232c, 0xffedf6d6}, .name = "Pixel Ink"},
        ColorPalette{.colors = {0xff051b2c, 0xff8bc8fe}, .name = "Mac Paint"},
        ColorPalette{.colors = {0xff212c28, 0xff72a488}, .name = "Nokia 3310"},
        ColorPalette{.colors = {0xff000000, 0xff83b07e}, .name = "Casio"},
        ColorPalette{.colors = {0xff322f29, 0xffd7d4cc}, .name = "Playdate"},
    };

    // clang-format off
    const std::unordered_map<SDL_Keycode, uint8_t> key_map = {
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_Q, 0x4}, {SDLK_W, 0x5}, {SDLK_E, 0x6}, {SDLK_R, 0xD},
        {SDLK_A, 0x7}, {SDLK_S, 0x8}, {SDLK_D, 0x9}, {SDLK_F, 0xE},
        {SDLK_Z, 0xA}, {SDLK_X, 0x0}, {SDLK_C, 0xB}, {SDLK_V, 0xF},
    };
    // clang-format on

    Chip8::Chip8(int argc, char* argv[]) : Game(argc, argv) {
        current_resolution = available_resolutions[0];
        current_palette    = available_palettes[0];

        SDL_SetWindowTitle(window, "Chip8 Emulator");
        SDL_SetWindowSize(window, current_resolution.x, current_resolution.y);
    }

    void Chip8::init() {
        screen_surface = SDL_CreateSurface(screen_width, screen_height, SDL_PIXELFORMAT_RGBA8888);
        screen_texture = SDL_CreateTextureFromSurface(renderer, screen_surface);
        SDL_SetTextureScaleMode(screen_texture, SDL_SCALEMODE_NEAREST);

        std::string file_to_load;
        if (args.size() > 1) {
            // Get the ROM file path
            file_to_load = args[1];
            SDL_Log("ROM file path: %s", file_to_load.c_str());
        }

        if (!file_to_load.empty()) {
            load_rom(std::filesystem::path(file_to_load));
        }
    }

    void Chip8::process_sdl_event(const SDL_Event& event) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (core.has_value() && is_emulation_running) {
                if (key_map.contains(event.key.key)) {
                    auto key_index = key_map.at(event.key.key);
                    core->set_key(key_index, true);
                }
            }
        }
        if (event.type == SDL_EVENT_KEY_UP) {
            if (core.has_value() && is_emulation_running) {
                if (key_map.contains(event.key.key)) {
                    auto key_index = key_map.at(event.key.key);
                    core->set_key(key_index, false);
                }
            }

            if (event.key.key == SDLK_P) {
                toggle_emulation();
            }
        }
    }

    void Chip8::fixed_update() {
        if (!core.has_value() || !is_emulation_running) {
            return;
        }

        constexpr uint32_t max_cycles_per_second = 700;

        uint64_t cycles_to_emulate = fixed_delta_time_ns / max_cycles_per_second;
        uint64_t cycles_passed     = 0;

        while (cycles_passed < cycles_to_emulate && !core->emulate_cycle()) {
            cycles_passed++;
        }
        core->timers_tick();
    }

    void Chip8::render_menu() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open File", "Ctrl + O", false, true)) {
                    start_open_file_dialog();
                }
                if (ImGui::MenuItem("Close", nullptr, false, current_rom.has_value())) {
                    close_rom();
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Save state", "F5", false, true)) {
                    // todo: save custom state
                    SDL_Log("Save state pressed");
                }
                if (ImGui::MenuItem("Load state", "F8", false, true)) {
                    // todo: load custom state
                    SDL_Log("Load state pressed");
                }
                ImGui::Separator();

                if (ImGui::BeginMenu("Quick Save", true)) {
                    for (int i = 0; i < 10; i++) {
                        std::string label    = std::to_string(i);
                        std::string shortcut = "Shift + " + std::to_string(i);
                        if (ImGui::MenuItem(label.c_str(), shortcut.c_str(), false, true)) {
                            // todo: quick save
                            SDL_Log("Quick Save %d pressed", i);
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Quick Load", true)) {
                    for (int i = 0; i < 10; i++) {
                        std::string label    = std::to_string(i);
                        std::string shortcut = std::to_string(i);
                        if (ImGui::MenuItem(label.c_str(), shortcut.c_str(), false, true)) {
                            // todo: quick load
                            SDL_Log("Quick Load %d pressed", i);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "Alt + F4", false, true)) {
                    SDL_Log("Exit pressed");
                    is_emulation_running = false;
                    should_quit          = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Machine")) {
                if (ImGui::MenuItem("Quirks", nullptr, false, true)) {
                    // todo: quirks settings
                    SDL_Log("Quirks pressed");
                }
                ImGui::Separator();

                std::string pause_label = is_emulation_running ? "Pause" : "Play";
                if (ImGui::MenuItem(pause_label.c_str(), "P", false, current_rom.has_value())) {
                    toggle_emulation();
                }
                if (ImGui::MenuItem("Reset", nullptr, false, current_rom.has_value())) {
                    reset_emulation();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window resolution", true)) {
                for (uint32_t i = 0; i < available_resolutions.size(); i++) {
                    const auto& resolution = available_resolutions[i];
                    std::string label      = std::format("{}x: {}x{}", i + 1, resolution.x, resolution.y);
                    if (ImGui::MenuItem(label.c_str(), nullptr, false, true)) {
                        set_window_size(resolution.x, resolution.y + static_cast<int32_t>(main_menu_height));
                        current_resolution = resolution;
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Color palette", true)) {
                for (const auto& palette : available_palettes) {
                    if (ImGui::MenuItem(palette.name.data(), nullptr, false, true)) {
                        current_palette = palette;
                    }
                }
                ImGui::EndMenu();
            }

            float height = ImGui::GetWindowHeight();
            if (abs(height - main_menu_height) > std::numeric_limits<float>::epsilon()) {
                main_menu_height = height;
                set_window_size(current_resolution.x, current_resolution.y + static_cast<int32_t>(height));
            }
            ImGui::EndMainMenuBar();
        }
    }

    void Chip8::render_screen() const {
        if (!core.has_value()) {
            return;
        }

        for (uint16_t y = 0; y < screen_height; y++) {
            for (uint16_t x = 0; x < screen_width; x++) {
                uint16_t pixel_index = y * screen_width + x;
                auto pixel_value     = core->get_state().display[pixel_index];
                uint32_t pixel_color = current_palette.colors[pixel_value];
                uint32_t* pixels     = static_cast<uint32_t*>(screen_surface->pixels);
                pixels[pixel_index]  = pixel_color;
            }
        }

        SDL_FRect dst = {
            .x = 0,
            .y = main_menu_height,
            .w = static_cast<float>(current_resolution.x),
            .h = static_cast<float>(current_resolution.y),
        };
        SDL_UpdateTexture(screen_texture, nullptr, screen_surface->pixels, screen_surface->pitch);
        SDL_RenderTexture(renderer, screen_texture, nullptr, &dst);
    }

    void Chip8::render() {
        render_menu();
        render_screen();
    }

    void Chip8::start_open_file_dialog() {
        auto callback = [](void* userdata, const char* const* filelist, int filter) {
            if (!filelist) {
                throw sdl::SdlException("Open file dialog error");
            }
            if (!*filelist) {
                SDL_Log("User did not select any files");
                return;
            }

            auto self     = static_cast<Chip8*>(userdata);
            auto filepath = std::filesystem::path(*filelist);
            SDL_Log("Full path to selected file: '%s'", filepath.string().c_str());
            self->load_rom(filepath);
        };
        constexpr std::array filters = {
            SDL_DialogFileFilter{"Chip8 rom", "ch8"},
            SDL_DialogFileFilter{"All files", "*"},
        };
        const char* cwd = SDL_GetCurrentDirectory();
        SDL_Log("Show Open File Dialog");
        SDL_ShowOpenFileDialog(callback, this, window, filters.data(), filters.size(), cwd, false);
    }

    void Chip8::load_rom(const std::filesystem::path& filepath) {
        assert(std::filesystem::exists(filepath) && "File not exists");
        auto rom = platform::read_file(filepath);
        if (rom.has_value()) {
            core                 = Chip8Core(rom.value());
            current_rom          = filepath;
            is_emulation_running = true;
        }
    }

    void Chip8::close_rom() {
        core.reset();
        current_rom.reset();
        is_emulation_running = false;
    }

    void Chip8::toggle_emulation() { is_emulation_running = !is_emulation_running; }

    void Chip8::reset_emulation() {
        if (current_rom.has_value()) {
            load_rom(current_rom.value());
        }
    }
} // namespace chip8
