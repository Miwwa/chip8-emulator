#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "Chip8.h"

int SDL_main(int argc, char* argv[])
{
    try
    {
        auto app = chip8::Chip8(argc, argv);
        app.run();
    }
    catch (const std::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", e.what());
        return 1;
    }

    return 0;
}
