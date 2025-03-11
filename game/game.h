#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SDL3/SDL.h"
#include "window_create_info.h"

constexpr uint64_t fixed_tps = 60;
constexpr uint64_t fixed_delta_time_ns = 1'000'000'000 / fixed_tps;
constexpr uint64_t max_delta_time_ns = 100'000'000;

class Game {
    void _sdl_init();
    void _imgui_init();
    void _process_sdl_event(const SDL_Event& event);

  protected:
    bool should_quit = false;

    SDL_Window* window;
    SDL_Renderer* renderer;

    std::vector<std::string> args;

    virtual void init() = 0;
    virtual void process_sdl_event(const SDL_Event& event) = 0;
    virtual void fixed_update() = 0;
    virtual void render() = 0;

    void set_window_size(int32_t width, int32_t height) const;

  public:
    Game(int argc, char* argv[]);
    Game(int argc, char* argv[], const window::WindowCreateInfo&);

    virtual ~Game();

    Game(const Game& other) = delete;
    Game(Game&& other) noexcept = delete;
    Game& operator=(const Game& other) = delete;
    Game& operator=(Game&& other) noexcept = delete;

    void run();
};
