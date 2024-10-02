#pragma once

#include "core/emulator.h"
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

template <auto F>
struct deleter
{
    decltype(auto) operator()(auto obj) const
        requires requires { F(obj); }
    {
        return F(obj);
    }
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

    void draw(cb::frame_buffer& buf) override;
    void poll_events() override;

    void set_title(std::string_view title) override;
    bool is_open() const override { return m_is_open; }
    int refresh_rate() const override;

    void on_keypress(const SDL_KeyboardEvent* event) override;
    void on_windowevent(const SDL_WindowEvent* event) override;

private:
    std::unique_ptr<SDL_Window> m_window;
    std::unique_ptr<SDL_Renderer> m_renderer;
    std::unique_ptr<SDL_Texture> m_screen_texture;
    int m_width;
    int m_height;
    bool m_is_open{true};
};
