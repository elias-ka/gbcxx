#pragma once

#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_render.h>
#include <imgui.h>

#include "core/core.hpp"

class MainApp
{
public:
    explicit MainApp(std::vector<uint8_t> rom_data);
    ~MainApp();

    void StartApplicationLoop();
    void LoadRom(const std::string& rom_path);
    void ShowErrorMessageBox(const std::string& message);

private:
    void LcdDrawCallback(const std::array<gb::video::Color, gb::kLcdSize>& lcd_buf);
    void PollEvents();
    void MainMenu();

    gb::Core core_;
    bool quit_{};

    SDL_Window* window_{};
    SDL_Renderer* renderer_{};

    SDL_Texture* lcd_texture_{};
    std::array<gb::video::Color, gb::kLcdSize> lcd_fb_{};

    SDL_Texture* vram_bg_texture_{};
    std::vector<gb::video::Color> vram_bg_fb_ =
        std::vector<gb::video::Color>(0x10000, {0xff, 0xff, 0xff});

    SDL_Texture* vram_window_texture_{};
    std::vector<gb::video::Color> vram_window_fb_ =
        std::vector<gb::video::Color>(0x10000, {0xff, 0xff, 0xff});

    bool show_imgui_demo_{};
    bool show_vram_debug_window_{};

    float menu_bar_height_{};

    ImFont* font_monospace_{};
    ImFont* font_body_{};
};
