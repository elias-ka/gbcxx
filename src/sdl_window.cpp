#include "sdl_window.h"
#include "common/logging.h"
#include <SDL2/SDL.h>
#include <fmt/format.h>

namespace frontend
{
    sdl_window::sdl_window(int width, int height, std::string_view window_title)
        : m_width(width)
        , m_height(height)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            LOG_CRITICAL("%s",
                         fmt::format("Failed to initialize SDL video subsystem: {}", SDL_GetError())
                             .c_str());
            std::exit(1);
        }

        m_window.reset(SDL_CreateWindow(window_title.data(), SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, m_width, m_height,
                                        SDL_WINDOW_SHOWN));
        if (!m_window)
        {
            LOG_CRITICAL("%s",
                         fmt::format("Failed to create window handle: {}", SDL_GetError()).c_str());
            std::exit(1);
        }
    }

    sdl_window::~sdl_window() { SDL_Quit(); }

    void sdl_window::set_title(std::string_view window_title)
    {
        SDL_SetWindowTitle(m_window.get(), window_title.data());
    }

    void sdl_window::wait_event()
    {
        SDL_Event event;
        if (SDL_WaitEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
            case SDL_KEYUP: on_keypress(&event); break;
            case SDL_WINDOWEVENT: on_windowevent(&event); break;
            }
        }
    }

    void sdl_window::on_keypress(const SDL_Event* event)
    {
        switch (event->key.keysym.sym)
        {
            // todo
        }
    }

    void sdl_window::on_windowevent(const SDL_Event* event)
    {
        switch (event->window.event)
        {
        case SDL_WINDOWEVENT_CLOSE: m_is_open = false; break;
        }
    }

} // namespace frontend
