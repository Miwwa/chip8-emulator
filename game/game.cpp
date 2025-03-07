#include <algorithm>
#include <stdexcept>

#include "SDL3/SDL.h"
#include "imgui.h"
#include "../chip8cpp/Chip8Core.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "platform/filesystem.h"

constexpr uint64_t fixed_tps = 60;
constexpr uint64_t fixed_delta_time_ns = 1'000'000'000 / fixed_tps;
constexpr uint64_t max_delta_time_ns = 100'000'000;

struct Application
{
    SDL_Window* window;
    SDL_Renderer* renderer;
};

Application init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(SDL_GetError());
    }

    Application app;
    bool isWindowCreated = SDL_CreateWindowAndRenderer(
        "Game",
        1920,
        960,
        0,
        &app.window,
        &app.renderer
    );
    if (!isWindowCreated)
    {
        throw std::runtime_error(SDL_GetError());
    }

    if (!SDL_SetRenderVSync(app.renderer, 1))
    {
        throw std::runtime_error(SDL_GetError());
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForSDLRenderer(app.window, app.renderer);
    ImGui_ImplSDLRenderer3_Init(app.renderer);

    return app;
}

void dispose(const Application& app)
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    std::string file_to_load;
    if (argc == 2)
    {
        // Get the ROM file path
        file_to_load = argv[1];
        SDL_Log("ROM file path: %s", file_to_load.c_str());
    }

    Application app;
    try
    {
        app = init();
    }
    catch (const std::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", e.what());
        dispose(app);
        return 1;
    }

    std::optional<chip8::Chip8Core> core;
    if (!file_to_load.empty())
    {
        auto rom = platform::read_file(file_to_load.c_str());
        if (rom.has_value())
        {
            core = chip8::Chip8Core(rom.value());
        }
    }

    uint64_t current_time = SDL_GetTicksNS();
    uint64_t accumulator = 0;

    bool shouldQuit = false;
    while (!shouldQuit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE))
            {
                shouldQuit = true;
            }
        }

        uint64_t new_time = SDL_GetTicksNS();
        uint64_t delta_time_ns = new_time - current_time;
        delta_time_ns = std::min(delta_time_ns, max_delta_time_ns);

        accumulator += delta_time_ns;
        while (accumulator >= fixed_delta_time_ns)
        {
            accumulator -= fixed_delta_time_ns;

            if (core.has_value())
            {
                constexpr uint32_t cycles_per_second = 700;
                uint32_t cycles_to_emulate = fixed_delta_time_ns / cycles_per_second;
                for (uint32_t i = 0; i < cycles_to_emulate; i++)
                {
                    core->emulate_cycle();
                }
                core->timers_tick();
            }
        }

        // gray background
        SDL_SetRenderDrawColor(app.renderer, 92, 105, 108, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(app.renderer);

        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        if (core.has_value())
        {
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

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app.renderer);

        SDL_RenderPresent(app.renderer);
    }

    dispose(app);
    return 0;
}
