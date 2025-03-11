#pragma once

#include <exception>
#include <string>

namespace sdl {
    class SdlException : public std::exception {
        std::string formatted_message;

    public:
        SdlException(const std::string& message);

        const char* what() const noexcept override;
    };
}