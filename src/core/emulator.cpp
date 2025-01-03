#include "core/emulator.hpp"

#include <SDL2/SDL.h>
#include <fmt/format.h>

namespace gbcxx {

Emulator::Emulator(std::vector<uint8_t> cartrom)
    : m_cpu(make_mbc(std::move(cartrom))) {}

void Emulator::run(Sdl_Window* win) {
  static const double clock_rate = 4194304.0;
  static const double clock_cycle = 1.0 / clock_rate;
  const double frame_time = 1.0 / win->refresh_rate();
  const auto cycles_per_frame = size_t(frame_time / clock_cycle);
  Ppu& ppu = m_cpu.mmu().ppu();

  while (win->is_open()) {
    size_t this_frame_cycles = 0;

    ppu.clear_should_redraw();
    win->poll_events();

    while (this_frame_cycles < cycles_per_frame) {
      m_cpu.step();
      this_frame_cycles += m_cpu.cycles_elapsed();
    }

    this_frame_cycles -= cycles_per_frame;
    m_cpu.subtract_cycles(this_frame_cycles);

    if (ppu.should_redraw()) {
      win->draw(ppu.frame_buffer());
    }
  }
}

}  // namespace gbcxx
