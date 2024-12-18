#pragma once

#include "core/mbc.hpp"

namespace gbcxx {
class Mbc0 final : public Mbc {
public:
    explicit Mbc0(std::vector<uint8_t> cartrom);

    [[nodiscard]] auto read_rom(uint16_t address) const -> uint8_t override;
    [[nodiscard]] auto read_ram(uint16_t address) const -> uint8_t override;
    auto write_rom(uint16_t address, uint8_t value) -> void override;
    auto write_ram(uint16_t address, uint8_t value) -> void override;

private:
    std::vector<uint8_t> m_rom;
};

} // namespace gbcxx
