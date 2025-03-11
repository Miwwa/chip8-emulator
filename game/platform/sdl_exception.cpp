#include "sdl_exception.h"

#include "SDL3/SDL_error.h"

#include <format>

sdl::SdlException::SdlException(const std::string& message) {
    formatted_message = std::format("[SDL Error] {}: {}", message, SDL_GetError());
}

const char* sdl::SdlException::what() const noexcept {
    return formatted_message.c_str();
}
