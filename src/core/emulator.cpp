#include "core/emulator.hpp"

#include <SDL2/SDL_timer.h>
#include <fmt/format.h>

namespace gbcxx {

Emulator::Emulator(std::vector<uint8_t> cartrom)
    : m_cpu(make_mbc(std::move(cartrom))) {}

void Emulator::run(SdlWindow* win) {
  static const double CLOCK_RATE = 4194304.0;
  static const double CLOCK_CYCLE = 1.0 / CLOCK_RATE;
  const double frame_time = 1.0 / win->refresh_rate();
  const auto cycles_per_frame = static_cast<size_t>(frame_time / CLOCK_CYCLE);
  auto& ppu = m_cpu.mmu().ppu();

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
      win->draw(ppu.buffer());
    }
  }
}

}  // namespace gbcxx
