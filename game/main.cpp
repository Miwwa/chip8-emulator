#include "SDL3/SDL.h"
#include "Chip8.h"

int main(int argc, char* argv[])
{
    try
    {
        Chip8 app = Chip8(argc, argv);
        app.run();
    }
    catch (const std::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", e.what());
        return 1;
    }
    
    return 0;
}
