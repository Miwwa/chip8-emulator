#pragma once

#include <array>
#include <cstdint>

namespace chip8
{
    struct Resolution
    {
        int32_t x, y;
    };

    constexpr std::array available_resolutions = {
        Resolution{.x = 640, .y = 320},
        Resolution{.x = 1280, .y = 640},
        Resolution{.x = 1920, .y = 960},
    };
}
