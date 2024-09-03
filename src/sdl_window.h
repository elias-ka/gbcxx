#pragma once

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL_render.h>
#include <memory>
#include <string_view>

namespace frontend
{
    struct sdl_deleter
    {
        void operator()(SDL_Window* window)
        {
            SDL_DestroyWindow(window);
        }
    };

    class sdl_window
    {
    public:
        sdl_window(int width, int height, std::string_view window_title);
        ~sdl_window();

        void wait_event();

        void set_title(std::string_view window_title);

        bool is_open() const
        {
            return m_is_open;
        }

    private:
        void on_keypress(const SDL_Event* event);
        void on_windowevent(const SDL_Event* event);

        int m_width;
        int m_height;
        std::unique_ptr<SDL_Window, sdl_deleter> m_window;
        bool m_is_open{true};
    };

} // namespace frontend
