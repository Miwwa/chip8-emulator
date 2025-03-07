#include "filesystem.h"

#include <SDL3/SDL.h>

namespace platform
{
    std::expected<std::vector<uint8_t>, std::string> read_file(const char* filepath)
    {
        size_t size;
        void* data = SDL_LoadFile(filepath, &size);
        if (!data)
        {
            std::string message = SDL_GetError();
            return std::unexpected{message};
        }

        std::vector result(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size);
        SDL_free(data);
        return result;
    }
}
