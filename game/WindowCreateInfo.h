#pragma once

#include <string>

namespace window {
    struct WindowCreateInfo {
        std::string title;
        int32_t width;
        int32_t height;
    };
} // namespace window
