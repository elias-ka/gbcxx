#pragma once

#include <vector>

#include "util.hpp"

namespace gbcxx {
class Mbc {
 public:
  virtual ~Mbc() = default;

  virtual u8 read_rom(u16 address) const = 0;
  virtual u8 read_ram(u16 address) const = 0;
  virtual void write_rom(u16 address, u8 value) = 0;
  virtual void write_ram(u16 address, u8 value) = 0;
};

std::unique_ptr<Mbc> make_mbc(std::vector<u8> cartrom);
u8 count_ram_banks(u8 value);
u8 count_rom_banks(u8 value);

}  // namespace gbcxx
