#pragma once

#include "core/mbc.hpp"

namespace gbcxx {
class Mbc0 final : public Mbc {
 public:
  explicit Mbc0(std::vector<uint8_t> cartrom);

  [[nodiscard]] uint8_t read_rom(uint16_t address) const override;
  [[nodiscard]] uint8_t read_ram(uint16_t address) const override;
  void write_rom(uint16_t address, uint8_t value) override;
  void write_ram(uint16_t address, uint8_t value) override;

 private:
  std::vector<uint8_t> m_rom;
};

}  // namespace gbcxx
