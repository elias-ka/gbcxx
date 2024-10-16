#pragma once

#include "core/video.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <memory>
#include <string_view>

#define SPECIALIZE_DEFAULT_DELETE(type, destroy_fn)                                                \
    template <>                                                                                    \
    struct std::default_delete<type>                                                               \
    {                                                                                              \
        void operator()(type* p) const { destroy_fn(p); }                                          \
    }

SPECIALIZE_DEFAULT_DELETE(SDL_Window, SDL_DestroyWindow);
SPECIALIZE_DEFAULT_DELETE(SDL_Texture, SDL_DestroyTexture);
SPECIALIZE_DEFAULT_DELETE(SDL_Renderer, SDL_DestroyRenderer);

namespace cb
{
    class SdlWindow
    {
    public:
        SdlWindow(int width, int height, std::string_view title);
        ~SdlWindow();

        SdlWindow(const SdlWindow&) = delete;
        SdlWindow(SdlWindow&&) = delete;
        SdlWindow operator=(const SdlWindow&) = delete;
        SdlWindow operator=(SdlWindow&&) = delete;

        void draw(const cb::FrameBuffer& buf);
        void poll_events();

        void set_title(std::string_view title);
        bool is_open() const { return m_is_open; }
        int refresh_rate() const;

        void on_keypress(const SDL_KeyboardEvent* event);
        void on_windowevent(const SDL_WindowEvent* event);

    private:
        std::unique_ptr<SDL_Window> m_window;
        std::unique_ptr<SDL_Renderer> m_renderer;
        std::unique_ptr<SDL_Texture> m_screen_texture;
        int m_width;
        int m_height;
        bool m_is_open{true};
    };

} // namespace cb
