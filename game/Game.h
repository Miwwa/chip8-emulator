﻿#pragma once
#include <string>
#include <vector>

#include "SDL3/SDL.h"

constexpr uint64_t fixed_tps = 60;
constexpr uint64_t fixed_delta_time_ns = 1'000'000'000 / fixed_tps;
constexpr uint64_t max_delta_time_ns = 100'000'000;

class Game
{
protected:
    SDL_Window* window;
    SDL_Renderer* renderer;

    std::vector<std::string> args;

public:
    Game(int argc, char* argv[]);
    virtual ~Game();

    Game(const Game& other) = delete;
    Game(Game&& other) noexcept = delete;
    Game& operator=(const Game& other) = delete;
    Game& operator=(Game&& other) noexcept = delete;

    void run();

    virtual void init() = 0;
    virtual void process_sdl_event(SDL_Event& event) = 0;
    virtual void fixed_update() = 0;
    virtual void render() = 0;
};
