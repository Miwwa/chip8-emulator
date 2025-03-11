#include "Chip8State.h"

#include <cassert>

namespace chip8 {
    Chip8State::Chip8State(std::vector<uint8_t> rom) {
        assert(rom.size() <= max_rom_size && "ROM is too big");

        ram.fill(0);
        display.fill(0);
        v.fill(0);
        keys.fill(0);
        pc = 0x200;
        i = 0;
        delay_timer = 0;
        sound_timer = 0;

        std::copy(build_in_font.begin(), build_in_font.end(), ram.begin() + font_offset);
        std::copy(rom.begin(), rom.end(), ram.begin() + rom_offset);
    }
} // namespace chip8
