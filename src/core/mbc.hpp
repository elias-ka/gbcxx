#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace gbcxx {
class Mbc {
 public:
  virtual ~Mbc() = default;

  [[nodiscard]] virtual uint8_t read_rom(uint16_t address) const = 0;
  [[nodiscard]] virtual uint8_t read_ram(uint16_t address) const = 0;
  virtual void write_rom(uint16_t address, uint8_t value) = 0;
  virtual void write_ram(uint16_t address, uint8_t value) = 0;
};

[[nodiscard]] std::unique_ptr<Mbc> make_mbc(std::vector<uint8_t> cartrom);
[[nodiscard]] uint8_t count_ram_banks(uint8_t value);
[[nodiscard]] uint8_t count_rom_banks(uint8_t value);

}  // namespace gbcxx
