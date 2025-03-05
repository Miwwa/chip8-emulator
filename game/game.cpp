#include <stdexcept>

#include "SDL3/SDL.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

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
        1280,
        720,
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

        // gray background
        SDL_SetRenderDrawColor(app.renderer, 92, 105, 108, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(app.renderer);

        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app.renderer);

        SDL_RenderPresent(app.renderer);
    }

    dispose(app);
    return 0;
}
