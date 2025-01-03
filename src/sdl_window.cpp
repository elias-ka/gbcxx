#include "sdl_window.hpp"

#include <SDL2/SDL.h>
#include <fmt/format.h>

#include "util.hpp"

namespace gbcxx {

Sdl_Window::Sdl_Window(int width, int height, const std::string& window_title)
    : m_width(width), m_height(height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    DIE("Failed to initialize SDL video subsystem: {}", SDL_GetError());
  }

  m_window = SDL_CreateWindow(window_title.data(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, m_width, m_height, 0);

  if (!m_window) {
    DIE("Failed to create window handle: {}", SDL_GetError());
  }

  m_renderer = SDL_CreateRenderer(
      m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!m_renderer) {
    DIE("Failed to create renderer: {}", SDL_GetError());
  }

  m_screen_texture =
      SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, lcd_width, lcd_height);
  if (!m_screen_texture) {
    DIE("Failed to create screen texture: {}", SDL_GetError());
  }
}

Sdl_Window::~Sdl_Window() {
  if (m_screen_texture) {
    SDL_DestroyTexture(m_screen_texture);
  }
  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
  }
  if (m_window) {
    SDL_DestroyWindow(m_window);
  }
}

void Sdl_Window::poll_events() {
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

void Sdl_Window::draw(const Frame_Buffer& buf) {
  if (SDL_RenderClear(m_renderer)) {
    DIE("Failed to clear renderer: {}", SDL_GetError());
  }

  uint8_t* pixel_buf{};
  int pitch{};
  if (SDL_LockTexture(m_screen_texture, nullptr,
                      reinterpret_cast<void**>(&pixel_buf), &pitch)) {
    DIE("Failed to lock screen texture: {}", SDL_GetError());
  }
  if (pitch == (lcd_width * sizeof(Rgba))) {
    std::memcpy(pixel_buf, buf.data(), lcd_width * lcd_height * sizeof(Rgba));
  } else {
    for (size_t i = 0; i < lcd_height; i++) {
      std::memcpy(pixel_buf, buf.data(), lcd_width * sizeof(Rgba));
      pixel_buf += lcd_width;
      pixel_buf += pitch;
    }
  }

  SDL_UnlockTexture(m_screen_texture);

  if (SDL_RenderCopy(m_renderer, m_screen_texture, nullptr, nullptr)) {
    DIE("Failed to copy texture to renderer: {}", SDL_GetError());
  }

  SDL_RenderPresent(m_renderer);
}

int Sdl_Window::refresh_rate() const {
  static const int default_rate{60};
  const int display_index{SDL_GetWindowDisplayIndex(m_window)};
  SDL_DisplayMode mode{};

  if (SDL_GetDesktopDisplayMode(display_index, &mode) != 0 ||
      !mode.refresh_rate) {
    return default_rate;
  }

  return mode.refresh_rate;
}

void Sdl_Window::on_keypress(const SDL_KeyboardEvent* e) {
  switch (e->keysym.sym) {
    case SDLK_ESCAPE: m_is_open = false; break;
    default: break;
  }
}

void Sdl_Window::on_windowevent(const SDL_WindowEvent* e) {
  switch (e->event) {
    case SDL_WINDOWEVENT_CLOSE: m_is_open = false; break;
    default: break;
  }
}
}  // namespace gbcxx
