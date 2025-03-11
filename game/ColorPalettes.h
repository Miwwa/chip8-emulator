#pragma once

#include <array>
#include <cstdint>

namespace chip8 {
    struct ColorPalette {
        std::array<uint32_t, 2> colors;
        std::string_view name;
    };

    constexpr std::array available_palettes = {
        ColorPalette{.colors = {0xff2e3037, 0xffebe5ce}, .name = "IBM 8503"},
        ColorPalette{.colors = {0xff3e232c, 0xffedf6d6}, .name = "Pixel Ink"},
        ColorPalette{.colors = {0xff051b2c, 0xff8bc8fe}, .name = "Mac Paint"},
        ColorPalette{.colors = {0xff212c28, 0xff72a488}, .name = "Nokia 3310"},
        ColorPalette{.colors = {0xff000000, 0xff83b07e}, .name = "Casio"},
        ColorPalette{.colors = {0xff322f29, 0xffd7d4cc}, .name = "Playdate"},
    };
} // namespace chip8
