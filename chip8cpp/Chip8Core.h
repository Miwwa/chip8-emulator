#pragma once

#include <cstdint>
#include <format>

#include "Chip8State.h"
#include "effolkronium/random.hpp"

namespace chip8
{
    class UnknownOpcodeException : public std::exception
    {
        uint16_t opcode;
        std::string message;

    public:
        UnknownOpcodeException(uint16_t opcode): opcode(opcode)
        {
            message = std::format("Unknown opcode: 0x{:04X}", opcode);
        }

        const char* what() const noexcept override
        {
            return message.c_str();
        }
    };

    class WaitForKeyCommand
    {
        enum class wait_state: uint8_t
        {
            none,
            waiting_for_key_pressed,
            waiting_for_key_released,
        };

        wait_state waitState = wait_state::none;
        std::array<uint8_t, 16> keysOnStart = {};
        uint8_t waitForReleaseOfKey = 0;

    public:
        void execute(Chip8State& state, uint8_t x);
    };

    class Chip8Core
    {
    private:
        Chip8State state;
        effolkronium::random_local rnd;
        WaitForKeyCommand waitForKeyCommand;

        void draw_sprite(uint8_t x, uint8_t y, uint8_t spriteHeight);

    public:
        Chip8Core(const std::vector<uint8_t>& rom): state(Chip8State(rom))
        {
        }

        void emulate_cycle();
        void timers_tick();
        void set_key(uint8_t key, bool pressed);
    };
}
