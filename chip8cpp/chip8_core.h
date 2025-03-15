#pragma once

#include <cstdint>
#include <format>

#include "chip8_state.h"
#include "effolkronium/random.hpp"
#include "flagpp/flags.hpp"

namespace chip8 {
    enum class Chip8Quirks : uint8_t {
        None        = 0,
        VfReset     = 1 << 0,
        Memory      = 1 << 2,
        DisplayWait = 1 << 3,
        Clipping    = 1 << 4,
        Shifting    = 1 << 5,
        Jumping     = 1 << 6,
        All         = 0x11,
    };
}

template <>
constexpr bool flagpp::enabled<chip8::Chip8Quirks> = true;

namespace chip8 {
    class UnknownOpcodeException : public std::exception {
        uint16_t opcode;
        std::string message;

      public:
        UnknownOpcodeException(uint16_t opcode) : opcode(opcode) {
            message = std::format("Unknown opcode: 0x{:04X}", opcode);
        }

        const char* what() const noexcept override { return message.c_str(); }
        uint16_t get_opcode() const { return opcode; }
    };

    class WaitForKeyCommand {
        enum class wait_state : uint8_t {
            none,
            waiting_for_key_pressed,
            waiting_for_key_released,
        };

        wait_state waitState                = wait_state::none;
        std::array<uint8_t, 16> keysOnStart = {};
        uint8_t waitForReleaseOfKey         = 0;

      public:
        void execute(Chip8State& state, uint8_t x);
    };

    class Chip8Core {
      private:
        Chip8State state;
        effolkronium::random_local rnd;
        WaitForKeyCommand waitForKeyCommand;
        Chip8Quirks quirks = Chip8Quirks::All;

        void draw_sprite(uint8_t x, uint8_t y, uint8_t spriteHeight);

      public:
        Chip8Core(const std::vector<uint8_t>& rom) : state(Chip8State(rom)) {}
        Chip8Core(const std::vector<uint8_t>& rom, Chip8Quirks quirks) : state(Chip8State(rom)), quirks(quirks) {}

        void emulate_cycle();
        void timers_tick();
        void set_key(uint8_t key, bool pressed);

        bool is_active_quirk(Chip8Quirks quirk) const { return quirk & quirks; }
        void toggle_quirk(Chip8Quirks quirk) { quirks ^= quirk; }

        const Chip8State& get_state() const { return state; }
    };
} // namespace chip8
