#pragma once

#include <cstdint>
#include <memory>

#include "core/mbc.hpp"
#include "core/ppu.hpp"
#include "core/timer.hpp"

namespace gbcxx {
class Mmu {
 public:
  explicit Mmu(std::unique_ptr<Mbc> mbc);

  void tick();

  [[nodiscard]] uint8_t read(uint16_t address) const;
  [[nodiscard]] uint16_t read16(uint16_t address) const;

  void write(uint16_t address, uint8_t value);
  void write16(uint16_t address, uint16_t value);

  void resize_memory(size_t new_size) { m_wram.resize(new_size); }

  Ppu& ppu() { return m_ppu; }

 private:
  Ppu m_ppu;
  Timer m_timer;
  std::array<uint8_t, 0x100> m_bootrom;
  std::vector<uint8_t> m_wram;
  std::vector<uint8_t> m_hram;
  std::unique_ptr<Mbc> m_mbc;
  std::vector<char> m_serial_buffer;
  bool m_bootrom_enabled{false};
  uint8_t m_inte{0x00};
  uint8_t m_intf{0xE1};
};

}  // namespace gbcxx
