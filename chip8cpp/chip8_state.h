﻿#pragma once

#include <array>
#include <cstdint>
#include <stack>
#include <vector>

namespace chip8 {
    constexpr uint8_t screen_width = 64;
    constexpr uint8_t screen_height = 32;
    constexpr uint16_t screen_size = screen_width * screen_height;

    constexpr uint16_t ram_size = 4096;
    constexpr uint16_t font_offset = 0x50;
    constexpr uint16_t rom_offset = 0x200;
    constexpr uint16_t max_rom_size = ram_size - rom_offset;

    constexpr uint8_t max_stack_size = 16;

    constexpr static std::array<uint8_t, 80> build_in_font = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };

    struct Chip8State {
        std::array<uint8_t, ram_size> ram;
        std::array<uint8_t, screen_size> display;
        std::array<uint8_t, 16> v;
        std::array<uint8_t, 16> keys;
        std::stack<uint16_t> stack;
        uint16_t pc;
        uint16_t i;
        uint8_t delay_timer;
        uint8_t sound_timer;

        Chip8State(std::vector<uint8_t> rom);
    };
} // namespace chip8
