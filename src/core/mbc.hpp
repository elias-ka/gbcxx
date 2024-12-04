#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace gbcxx {
class Mbc {
 public:
  virtual ~Mbc() = default;

  virtual uint8_t read_rom(uint16_t address) const = 0;
  virtual uint8_t read_ram(uint16_t address) const = 0;
  virtual void write_rom(uint16_t address, uint8_t value) = 0;
  virtual void write_ram(uint16_t address, uint8_t value) = 0;
};

std::unique_ptr<Mbc> make_mbc(std::vector<uint8_t> cartrom);
uint8_t count_ram_banks(uint8_t value);
uint8_t count_rom_banks(uint8_t value);

}  // namespace gbcxx
