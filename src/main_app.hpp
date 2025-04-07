#pragma once

#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_render.h>
#include <imgui.h>

#include "core/core.hpp"

class MainApp
{
public:
    explicit MainApp(const std::filesystem::path& rom_file);
    ~MainApp();

    [[nodiscard]] bool QuitRequested() const { return quit_; }

    void Step();
    void LoadRom(const std::string& rom_path);
    void ShowErrorMessageBox(const std::string& message) const;

private:
    void LcdDrawCallback(const std::array<gb::video::Color, gb::kLcdSize>& lcd_buf);
    void PollEvents();
    void MainMenu();

    gb::Core core_;
    bool quit_{};

    SDL_Window* window_{};
    SDL_Renderer* renderer_{};

    SDL_Texture* lcd_texture_{};
    gb::video::LcdBuffer lcd_buf_{};

    float menu_bar_height_{};

    ImFont* font_monospace_{};
    ImFont* font_body_{};
};
