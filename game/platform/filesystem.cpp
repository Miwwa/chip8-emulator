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
            return std::unexpected{SDL_GetError()};
        }

        std::vector result(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size);
        SDL_free(data);
        return result;
    }

    std::expected<std::vector<uint8_t>, std::string> read_file(const std::filesystem::path& filepath)
    {
        return read_file(filepath.string().c_str());
    }
}
