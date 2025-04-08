#pragma once

#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_render.h>

#include "core/core.hpp"

class MainApp
{
public:
    explicit MainApp(const std::filesystem::path& rom_file);
    ~MainApp();

    void Step();

    [[nodiscard]] bool QuitRequested() const { return quit_; }

private:
    void LcdDrawCallback(const std::array<gb::video::Color, gb::kLcdSize>& lcd_buf);
    void PollEvents();

    gb::Core core_;
    bool quit_{};

    SDL_Window* window_{};
    SDL_Renderer* renderer_{};

    SDL_Texture* lcd_texture_{};
    gb::video::LcdBuffer lcd_buf_{};
};
