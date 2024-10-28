#pragma once

#include "core/mbc.hpp"

namespace gbcxx {
class Mbc0 final : public Mbc {
 public:
  explicit Mbc0(std::vector<u8> cartrom);

  u8 read_rom(u16 address) const override;
  u8 read_ram(u16 address) const override;
  void write_rom(u16, u8) override;
  void write_ram(u16 address, u8 value) override;

 private:
  std::vector<u8> m_rom;
  std::vector<u8> m_ram;
};

}  // namespace gbcxx
