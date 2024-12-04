#pragma once

#include "core/processor.hpp"
#include "sdl_window.hpp"

namespace gbcxx {
class Emulator {
 public:
  explicit Emulator(std::vector<uint8_t> cartrom);

  void run(SdlWindow* win);

 private:
  Cpu m_cpu;
};
}  // namespace gbcxx
