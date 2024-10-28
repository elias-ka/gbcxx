#pragma once

#include "core/mbc.hpp"

namespace gbcxx {
class Mbc1 final : public Mbc {
 public:
  explicit Mbc1(std::vector<u8> cartrom);

  u8 read_rom(u16 address) const override;
  u8 read_ram(u16 address) const override;
  void write_rom(u16, u8) override;
  void write_ram(u16 address, u8 value) override;

 private:
  std::vector<u8> m_rom;
  u8 m_banking_mode{0};
  u8 m_rom_bank{1};
  u8 m_ram_bank{0};
  u8 m_rom_banks_count{0};
  u8 m_ram_banks_count{0};
  std::vector<u8> m_ram;
  bool m_ram_enabled{false};
};
}  // namespace gbcxx
