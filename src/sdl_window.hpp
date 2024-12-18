#pragma once

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <memory>
#include <string_view>

#define SPECIALIZE_DEFAULT_DELETE(type, destroy_fn)                                                \
    template <>                                                                                    \
    struct std::default_delete<type> {                                                             \
        void operator()(type* p) const                                                             \
        {                                                                                          \
            if (p) {                                                                               \
                destroy_fn(p);                                                                     \
            }                                                                                      \
        }                                                                                          \
    }

SPECIALIZE_DEFAULT_DELETE(SDL_Window, SDL_DestroyWindow);
SPECIALIZE_DEFAULT_DELETE(SDL_Texture, SDL_DestroyTexture);
SPECIALIZE_DEFAULT_DELETE(SDL_Renderer, SDL_DestroyRenderer);

namespace gbcxx {
class Sdl_Window {
public:
    Sdl_Window(int width, int height, std::string_view title);
    ~Sdl_Window();

    Sdl_Window(const Sdl_Window&) = delete;
    Sdl_Window(Sdl_Window&&) = delete;
    auto operator=(const Sdl_Window&) -> Sdl_Window = delete;
    auto operator=(Sdl_Window&&) -> Sdl_Window = delete;

    // auto draw(const gbcxx::FrameBuffer& buf) -> void;
    auto poll_events() -> void;

    auto set_title(std::string_view title) -> void;

    [[nodiscard]] auto is_open() const -> bool
    {
        return m_is_open;
    }

    [[nodiscard]] auto refresh_rate() const -> int;

    auto on_keypress(const SDL_KeyboardEvent* e) -> void;
    auto on_windowevent(const SDL_WindowEvent* e) -> void;

private:
    std::unique_ptr<SDL_Window> m_window;
    std::unique_ptr<SDL_Renderer> m_renderer;
    std::unique_ptr<SDL_Texture> m_screen_texture;
    int m_width;
    int m_height;
    bool m_is_open { true };
};

} // namespace gbcxx
