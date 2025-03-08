#include "Game.h"

#include <stdexcept>

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

Game::Game(int argc, char* argv[])
{
    // construct args vector for future use
    for (int i = 0; i < argc; i++)
    {
        this->args.emplace_back(argv[i]);
    }

    // SDL init
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(SDL_GetError());
    }

    bool isWindowCreated = SDL_CreateWindowAndRenderer(
        "Game",
        1920,
        960,
        0,
        &window,
        &renderer
    );
    if (!isWindowCreated)
    {
        throw std::runtime_error(SDL_GetError());
    }

    if (!SDL_SetRenderVSync(renderer, 1))
    {
        throw std::runtime_error(SDL_GetError());
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

Game::~Game()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::run()
{
    init();

    uint64_t current_time = SDL_GetTicksNS();
    uint64_t accumulator = 0;

    bool shouldQuit = false;
    while (!shouldQuit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                shouldQuit = true;
            }

            ImGui_ImplSDL3_ProcessEvent(&event);
            process_sdl_event(event);
        }

        uint64_t new_time = SDL_GetTicksNS();
        uint64_t delta_time_ns = new_time - current_time;
        delta_time_ns = std::min(delta_time_ns, max_delta_time_ns);

        accumulator += delta_time_ns;
        while (accumulator >= fixed_delta_time_ns)
        {
            accumulator -= fixed_delta_time_ns;
            /* --------- update --------- */
            fixed_update();
        }

        // gray background
        SDL_SetRenderDrawColor(renderer, 92, 105, 108, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        /* --------- render --------- */
        render();

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }
}
