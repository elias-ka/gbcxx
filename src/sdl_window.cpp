#include "sdl_window.h"
#include "util.h"
#include <SDL2/SDL.h>
#include <fmt/format.h>

namespace cb
{

    sdl_window::sdl_window(int width, int height, std::string_view window_title)
        : m_width(width)
        , m_height(height)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            LOG_ERROR("Failed to initialize SDL video subsystem: {}", SDL_GetError());
            std::exit(1);
        }

        m_window.reset(SDL_CreateWindow(window_title.data(), SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN));

        if (!m_window)
        {
            LOG_ERROR("Failed to create window handle: {}", SDL_GetError());
            std::exit(1);
        }

        m_renderer.reset(SDL_CreateRenderer(m_window.get(), -1,
                                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
        if (!m_renderer)
        {
            LOG_ERROR("Failed to create renderer: {}", SDL_GetError());
            std::exit(1);
        }

        m_screen_texture.reset(SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                                 SDL_TEXTUREACCESS_STREAMING, 160, 144));
        if (!m_screen_texture)
        {
            LOG_ERROR("Failed to create screen texture: {}", SDL_GetError());
            std::exit(1);
        }
    }

    sdl_window::~sdl_window() { SDL_Quit(); }

    void sdl_window::set_title(std::string_view window_title)
    {
        SDL_SetWindowTitle(m_window.get(), window_title.data());
    }

    void sdl_window::poll_events()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: m_is_open = false; break;
            case SDL_KEYDOWN:
            case SDL_KEYUP: on_keypress(&event.key); break;
            case SDL_WINDOWEVENT: on_windowevent(&event.window); break;
            default: break;
            }
        }
    }

    void sdl_window::draw(const frame_buffer& buf)
    {
        void* pixels_ptr = nullptr;
        int pitch = 0;

        SDL_LockTexture(m_screen_texture.get(), nullptr, &pixels_ptr, &pitch);
        auto* pixels = static_cast<u32*>(pixels_ptr);
        for (u32 y = 0; y < screen_height; y++)
        {
            for (u32 x = 0; x < screen_width; x++)
            {
                const auto color = buf.pixel_color(x, y);
                pixels[screen_width * y + x] = static_cast<u32>(color);
            }
        }
        SDL_UnlockTexture(m_screen_texture.get());
        SDL_RenderCopy(m_renderer.get(), m_screen_texture.get(), nullptr, nullptr);
        SDL_RenderPresent(m_renderer.get());
        SDL_RenderClear(m_renderer.get());
    }

    int sdl_window::refresh_rate() const
    {
        static const int default_rate = 60;
        const int display_index = SDL_GetWindowDisplayIndex(m_window.get());
        SDL_DisplayMode mode{};

        if (SDL_GetDesktopDisplayMode(display_index, &mode) != 0 || !mode.refresh_rate)
            return default_rate;

        return mode.refresh_rate;
    }

    void sdl_window::on_keypress(const SDL_KeyboardEvent* event)
    {
        switch (event->keysym.sym)
        {
        case SDLK_ESCAPE: m_is_open = false; break;
        default: break;
        }
    }

    void sdl_window::on_windowevent(const SDL_WindowEvent* event)
    {
        switch (event->event)
        {
        case SDL_WINDOWEVENT_CLOSE: m_is_open = false; break;
        default: break;
        }
    }
} // namespace cb
