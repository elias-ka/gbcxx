#pragma once

#include "core/emulator.h"
#include "core/video.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <memory>
#include <string_view>

namespace frontend
{
    struct sdl_deleter
    {
        void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
        void operator()(SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); }
        void operator()(SDL_Texture* texture) { SDL_DestroyTexture(texture); }
    };

    class sdl_window : public cb::window
    {
    public:
        sdl_window(int width, int height, std::string_view title);
        ~sdl_window();

        sdl_window(const sdl_window&) = delete;
        sdl_window(sdl_window&&) = delete;
        sdl_window operator=(const sdl_window&) = delete;
        sdl_window operator=(sdl_window&&) = delete;

        void draw(cb::video::frame_buffer& buf) override;
        void poll_events() override;

        void set_title(std::string_view title) override;
        bool is_open() const override { return m_is_open; }
        int refresh_rate() const override;

        void on_keypress(const SDL_KeyboardEvent* event) override;
        void on_windowevent(const SDL_WindowEvent* event) override;

    private:
        std::unique_ptr<SDL_Window, sdl_deleter> m_window;
        std::unique_ptr<SDL_Renderer, sdl_deleter> m_renderer;
        std::unique_ptr<SDL_Texture, sdl_deleter> m_screen_texture;
        int m_width;
        int m_height;
        bool m_is_open{true};
    };

} // namespace frontend
