#pragma once

#include "core/mbc.hpp"
#include "core/video.hpp"
#include "util.hpp"

namespace gbcxx {
class Mmu {
 public:
  explicit Mmu(std::unique_ptr<Mbc> mbc);

  void tick();

  u8 read(u16 address) const;
  u16 read16(u16 address) const;

  void write(u16 address, u8 data);
  void write16(u16 address, u16 data);

  void resize_memory(usz new_size) { m_wram.resize(new_size); }

  Ppu& ppu() { return m_ppu; }

 private:
  Ppu m_ppu{};

  std::array<u8, 0x100> m_bootrom;
  std::vector<u8> m_wram;
  std::vector<u8> m_hram;
  std::unique_ptr<Mbc> m_mbc;
  bool m_bootrom_enabled{false};  // to-do: change default to true
  u8 m_inte{0x00};
  u8 m_intf{0xE1};
};

}  // namespace gbcxx
