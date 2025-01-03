#pragma once

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <memory>

#include "core/ppu.hpp"

// #define SPECIALIZE_DEFAULT_DELETE(type, destroy_fn) \
//   template <>                                       \
//   struct std::default_delete<type> {                \
//     void operator()(type* p) const {                \
//       if (p) {                                      \
//         destroy_fn(p);                              \
//       }                                             \
//     }                                               \
//   }

// SPECIALIZE_DEFAULT_DELETE(SDL_Window, SDL_DestroyWindow);
// SPECIALIZE_DEFAULT_DELETE(SDL_Texture, SDL_DestroyTexture);
// SPECIALIZE_DEFAULT_DELETE(SDL_Renderer, SDL_DestroyRenderer);

namespace gbcxx {
class Sdl_Window {
 public:
  Sdl_Window(int width, int height, const std::string& title);
  ~Sdl_Window();

  Sdl_Window(const Sdl_Window&) = delete;
  Sdl_Window(Sdl_Window&&) = delete;
  SDL_Window operator=(const Sdl_Window&) = delete;
  SDL_Window operator=(Sdl_Window&&) = delete;

  void draw(const Frame_Buffer& buf);
  void poll_events();

  [[nodiscard]] bool is_open() const { return m_is_open; }
  [[nodiscard]] int refresh_rate() const;

  void on_keypress(const SDL_KeyboardEvent* e);
  void on_windowevent(const SDL_WindowEvent* e);

 private:
  SDL_Window* m_window;
  SDL_Renderer* m_renderer;
  SDL_Texture* m_screen_texture;
  int m_width;
  int m_height;
  bool m_is_open{true};
};

}  // namespace gbcxx
