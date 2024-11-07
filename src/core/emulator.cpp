#include "core/emulator.hpp"

#include <SDL2/SDL_timer.h>
#include <fmt/format.h>

#include "util.hpp"

namespace gbcxx {

Emulator::Emulator(std::vector<u8> cartrom)
    : m_cpu(make_mbc(std::move(cartrom))) {}

void Emulator::run(SdlWindow* win) {
  static const f64 CLOCK_RATE = 4194304.0;
  static const f64 CLOCK_CYCLE = 1 / CLOCK_RATE;
  const f64 frame_time = 1.0 / win->refresh_rate();
  const usz cycles_per_frame = static_cast<usz>(frame_time / CLOCK_CYCLE);
  auto& ppu = m_cpu.mmu().ppu();

  LOG_DEBUG("clock_cycle = {}", CLOCK_CYCLE);
  LOG_DEBUG("frame_time = {}", frame_time);
  LOG_DEBUG("cycles_per_frame = {}", cycles_per_frame);

  while (win->is_open()) {
    usz this_frame_cycles = 0;

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
