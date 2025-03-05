#include <stdexcept>

#include "SDL3/SDL.h"

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

    return app;
}

void dispose(Application& app)
{
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
    while (!shouldQuit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                shouldQuit = true;
            }
        }

        // gray background
        SDL_SetRenderDrawColor(app.renderer, 92, 105, 108, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(app.renderer);

        SDL_RenderPresent(app.renderer);
    }

    dispose(app);
    return 0;
}
