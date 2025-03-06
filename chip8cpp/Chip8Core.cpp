#include "Chip8Core.h"

namespace
{
    uint8_t get_bit(uint8_t value, uint8_t n)
    {
        assert(n < 8 && "Bit index out of range");
        uint8_t shift = 7 - n;
        uint8_t mask = static_cast<uint8_t>(1 << shift);
        return (value & mask) >> shift;
    }
}

namespace chip8
{
    void WaitForKeyCommand::execute(Chip8State& state, uint8_t x)
    {
        if (waitState == wait_state::none)
        {
            waitState = wait_state::waiting_for_key_pressed;
            keysOnStart = state.keys;
            state.pc -= 2;
            return;
        }

        if (waitState == wait_state::waiting_for_key_pressed)
        {
            for (uint8_t i = 0; i < 16; i++)
            {
                if (keysOnStart[i] == 0 && state.keys[i] == 1)
                {
                    waitForReleaseOfKey = i;
                    waitState = wait_state::waiting_for_key_released;
                    break;
                }
            }
            state.pc -= 2;
            return;
        }

        if (waitState == wait_state::waiting_for_key_released)
        {
            if (state.keys[waitForReleaseOfKey] == 0)
            {
                waitState = wait_state::none;
                state.v[x] = waitForReleaseOfKey;
            }
            else
            {
                state.pc -= 2;
            }
            return;
        }

        assert(false && "Invalid wait state");
    }

    void Chip8Core::draw_sprite(uint8_t x, uint8_t y, uint8_t spriteHeight)
    {
        x %= screen_width;
        y %= screen_height;

        state.v[0xf] = 0;
        for (uint8_t byte_index = 0; byte_index < spriteHeight && y + byte_index < screen_height; byte_index++)
        {
            uint8_t sprite_byte = state.ram[state.i + byte_index];
            for (uint8_t bit_index = 0; bit_index < 8 && x + bit_index < screen_width; bit_index++)
            {
                uint32_t display_index = x + bit_index + (y + byte_index) * screen_width;
                uint8_t sprite_pixel = get_bit(sprite_byte, bit_index);
                uint8_t display_pixel = state.display[display_index];

                state.display[display_index] = display_pixel ^ sprite_pixel;
                // Set VF to 01 if any set pixels are changed to unset
                if (display_pixel == 1 && sprite_pixel == 1)
                {
                    state.v[0xf] = 1;
                }
            }
        }
    }

    void Chip8Core::emulate_cycle()
    {
        uint16_t opcode = static_cast<uint16_t>(state.ram[state.pc] << 8) | state.ram[state.pc + 1];
        state.pc += 2;

        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;
        uint8_t n = opcode & 0x000F;
        uint8_t nn = opcode & 0x00FF;
        uint16_t nnn = opcode & 0x0FFF;
        uint8_t flag;

        switch (opcode & 0xf000)
        {
        case 0x0000:
            switch (opcode)
            {
            // Clear the screen
            case 0x00E0:
                state.display.fill(0);
                break;
            // Return from a subroutine
            case 0x00EE:
                state.pc = state.stack.top();
                state.stack.pop();
                break;
            default:
                throw UnknownOpcodeException(opcode);
            }

            break;
        // 1NNN Jump to address NNN
        case 0x1000:
            state.pc = nnn;
            break;
        // Execute subroutine starting at address NNN
        case 0x2000:
            state.stack.push(state.pc);
            state.pc = nnn;
            break;
        // Skip the following instruction if the value of register VX equals NN
        case 0x3000:
            if (state.v[x] == nn)
            {
                state.pc += 2;
            }
            break;
        // Skip the following instruction if the value of register VX is not equal to NN
        case 0x4000:
            if (state.v[x] != nn)
            {
                state.pc += 2;
            }
            break;
        // Skip the following instruction if the value of register VX is equal to the value of register VY
        case 0x5000:
            if (state.v[x] == state.v[y])
            {
                state.pc += 2;
            }
            break;
        // 6XNN Store number NN in register VX
        case 0x6000:
            state.v[x] = nn;
            break;
        // 7XNN Add the value NN to register VX
        case 0x7000:
            state.v[x] += nn;
            break;
        case 0x8000:
            switch (n)
            {
            case 0:
                // 8XY0 - Store the value of register VY in register VX
                state.v[x] = state.v[y];
                break;
            case 1:
                // 8XY1 - Set VX to VX OR VY
                state.v[x] |= state.v[y];
                break;
            case 2:
                // 8XY2 - Set VX to VX AND VY
                state.v[x] &= state.v[y];
                break;
            case 3:
                // 8XY3 - Set VX to VX XOR VY
                state.v[x] ^= state.v[y];
                break;
            case 4:
                // 8XY4 - Add the value of register VY to register VX. Set VF to 01 if a carry occurs. Set VF to 00 if a carry does not occur
                flag = state.v[x] + state.v[y] > 255 ? 1 : 0;
                state.v[x] += state.v[y];
                state.v[0xf] = flag;
                break;
            case 5:
                // 8XY5 - Subtract the value of register VY from register VX. Set VF to 00 if a borrow occurs. Set VF to 01 if a borrow does not occur
                flag = state.v[x] - state.v[y] >= 0 ? 1 : 0;
                state.v[x] -= state.v[y];
                state.v[0xf] = flag;
                break;
            case 6:
                // 8XY6 - Store the value of register VY shifted right one bit in register VX. Set register VF to the least significant bit prior to the shift. VY is unchanged
                // https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#8xy6-and-8xye-shift
                flag = get_bit(state.v[y], 7);
                state.v[x] = state.v[y];
                state.v[x] = state.v[x] >> 1;
                state.v[0xf] = flag;
                break;
            case 7:
                // 8XY7 - Set register VX to the value of VY minus VX. Set VF to 00 if a borrow occurs. Set VF to 01 if a borrow does not occur
                flag = state.v[y] - state.v[x] >= 0 ? 1 : 0;
                state.v[x] = state.v[y] - state.v[x];
                state.v[0xf] = flag;
                break;
            case 0xE:
                // 8XYE - Store the value of register VY shifted left one bit in register VX. Set register VF to the most significant bit prior to the shift. VY is unchanged
                flag = get_bit(state.v[y], 0);
                state.v[x] = state.v[y];
                state.v[x] = static_cast<uint8_t>(state.v[x] << 1);
                state.v[0xf] = flag;
                break;
            default:
                throw UnknownOpcodeException(opcode);
            }

            break;
        // 9XY0 Skip the following instruction if the value of register VX is not equal to the value of register VY
        case 0x9000:
            if (state.v[x] != state.v[y])
            {
                state.pc += 2;
            }
            break;
        // ANNN Store memory address NNN in register I
        case 0xA000:
            state.i = nnn;
            break;
        // BNNN Jump to address NNN + V0
        case 0xB000:
            state.pc = nnn + state.v[0];
            break;
        // CXNN Set VX to a random number with a mask of NN
        case 0xC000:
            state.v[x] = rnd.get<uint8_t>(0, 255) && nn;
            break;
        // DXYN Draw a sprite
        case 0xD000:
            draw_sprite(state.v[x], state.v[y], n);
            break;
        case 0xE000:
            switch (nn)
            {
            case 0x9E:
                // EX9E Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed
                assert(state.v[x] < state.keys.size() && "VX is out of range");
                if (state.keys[state.v[x]] == 1)
                {
                    state.pc += 2;
                }

                break;
            case 0xA1:
                // EXA1 Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed
                assert(state.v[x] < state.keys.size() && "VX is out of range");
                if (state.keys[state.v[x]] == 0)
                {
                    state.pc += 2;
                }

                break;
            default:
                throw UnknownOpcodeException(opcode);
            }

            break;
        case 0xf000:
            switch (nn)
            {
            case 0x07:
                // FX07 Store the current value of the delay timer in register VX
                state.v[x] = state.delay_timer;
                break;
            case 0x0A:
                // FX0A Wait for a keypress and store the result in register VX
                waitForKeyCommand.execute(state, x);
                break;
            case 0x15:
                // FX15 Set the delay timer to the value of register VX
                state.delay_timer = state.v[x];
                break;
            case 0x18:
                // FX18 Set the sound timer to the value of register VX
                state.sound_timer = state.v[x];
                break;
            case 0x1E:
                // FX1E Add the value stored in register VX to register I
                flag = state.i + state.v[x] > 0xfff ? 1 : 0;
                state.i += state.v[x];
                state.v[0xf] = flag;
                break;
            case 0x29:
                // FX29 Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
                state.i = font_offset + state.v[x];
                break;
            case 0x33:
                // FX33 Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I + 1, and I + 2
                state.ram[state.i] = state.v[x] % 1000 / 100;
                state.ram[state.i + 1] = state.v[x] % 100 / 10;
                state.ram[state.i + 2] = state.v[x] % 10;
                break;
            case 0x55:
                // FX55 Store the values of registers V0 to VX inclusive in memory starting at address I. I is set to I + X + 1 after operation
                for (uint16_t i = 0; i <= x; i++)
                {
                    state.ram[state.i + 1] = state.v[i];
                }
                state.i += x + 1;
                break;
            case 0x65:
                // FX65 Fill registers V0 to VX inclusive with the values stored in memory starting at address I. I is set to I + X + 1 after operation
                for (uint16_t i = 0; i <= x; i++)
                {
                    state.v[i] = state.ram[state.i + 1];
                }
                state.i += x + 1;
                break;
            default:
                throw UnknownOpcodeException(opcode);
            }
            break;
        default:
            throw UnknownOpcodeException(opcode);
        }
    }

    void Chip8Core::timers_tick()
    {
        if (state.delay_timer > 0)
        {
            state.delay_timer--;
        }
        if (state.sound_timer > 0)
        {
            state.sound_timer--;
        }
    }

    void Chip8Core::set_key(const uint8_t key, const bool pressed)
    {
        assert(key < state.keys.size() && "Key index out of range");
        state.keys[key] = pressed ? 1 : 0;
    }
}
