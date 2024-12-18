#include "sdl_window.hpp"

#include <SDL2/SDL.h>
#include <fmt/format.h>

#include "util.hpp"

namespace gbcxx {

Sdl_Window::Sdl_Window(int width, int height, std::string_view m_windowtitle)
    : m_width(width)
    , m_height(height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        DIE("Failed to initialize SDL video subsystem: {}", SDL_GetError());
    }

    m_window.reset(SDL_CreateWindow(m_windowtitle.data(), SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_SHOWN));

    if (!m_window) {
        DIE("Failed to create window handle: {}", SDL_GetError());
    }

    m_renderer.reset(SDL_CreateRenderer(m_window.get(), -1,
                                        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!m_renderer) {
        DIE("Failed to create renderer: {}", SDL_GetError());
    }

    m_screen_texture.reset(SDL_CreateTexture(m_renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING, 160, 144));
    if (!m_screen_texture) {
        DIE("Failed to create screen texture: {}", SDL_GetError());
    }
}

Sdl_Window::~Sdl_Window()
{
    SDL_Quit();
}

auto Sdl_Window::set_title(std::string_view m_windowtitle) -> void
{
    SDL_SetWindowTitle(m_window.get(), m_windowtitle.data());
}

auto Sdl_Window::poll_events() -> void
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT: m_is_open = false; break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: on_keypress(&event.key); break;
        case SDL_WINDOWEVENT: on_windowevent(&event.window); break;
        default: break;
        }
    }
}

// auto Sdl_Window::draw(const FrameBuffer& buf) -> void {
//   if (SDL_RenderClear(m_renderer.get())) {
//     DIE("Failed to clear renderer: {}", SDL_GetError());
//   }

//   uint8_t* pixel_buf = nullptr;
//   int pitch = 0;
//   if (SDL_LockTexture(m_screen_texture.get(), nullptr,
//                       reinterpret_cast<void**>(&pixel_buf), &pitch)) {
//     DIE("Failed to lock screen texture: {}", SDL_GetError());
//   }

//   if (pitch == (SCREEN_WIDTH * sizeof(Rgba))) {
//     std::memcpy(pixel_buf, buf.data(),
//                 SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Rgba));
//   } else {
//     for (size_t i = 0; i < SCREEN_HEIGHT; i++) {
//       std::memcpy(pixel_buf, buf.data(), SCREEN_WIDTH * sizeof(Rgba));
//       pixel_buf += SCREEN_WIDTH;
//       pixel_buf += pitch;
//     }
//   }

//   SDL_UnlockTexture(m_screen_texture.get());

//   if (SDL_RenderCopy(m_renderer.get(), m_screen_texture.get(), nullptr,
//                      nullptr)) {
//     DIE("Failed to copy texture to renderer: {}", SDL_GetError());
//   }

//   SDL_RenderPresent(m_renderer.get());
// }

auto Sdl_Window::refresh_rate() const -> int
{
    static const int default_rate = 60;
    const int display_index = SDL_GetWindowDisplayIndex(m_window.get());
    SDL_DisplayMode mode {};

    if (SDL_GetDesktopDisplayMode(display_index, &mode) != 0 || !mode.refresh_rate) {
        return default_rate;
    }

    return mode.refresh_rate;
}

auto Sdl_Window::on_keypress(const SDL_KeyboardEvent* e) -> void
{
    switch (e->keysym.sym) {
    case SDLK_ESCAPE: m_is_open = false; break;
    default: break;
    }
}

auto Sdl_Window::on_windowevent(const SDL_WindowEvent* e) -> void
{
    switch (e->event) {
    case SDL_WINDOWEVENT_CLOSE: m_is_open = false; break;
    default: break;
    }
}
} // namespace gbcxx
