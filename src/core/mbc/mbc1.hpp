#pragma once

#include "core/mbc.hpp"

namespace gbcxx {
class Mbc1 final : public Mbc {
public:
    explicit Mbc1(std::vector<uint8_t> cartrom);

    [[nodiscard]] auto read_rom(uint16_t address) const -> uint8_t override;
    [[nodiscard]] auto read_ram(uint16_t address) const -> uint8_t override;
    auto write_rom(uint16_t address, uint8_t value) -> void override;
    auto write_ram(uint16_t address, uint8_t value) -> void override;

private:
    std::vector<uint8_t> m_rom;
    uint8_t m_banking_mode { 0 };
    uint8_t m_rom_bank { 1 };
    uint8_t m_ram_bank { 0 };
    uint8_t m_rom_banks_count { 0 };
    uint8_t m_ram_banks_count { 0 };
    std::vector<uint8_t> m_ram;
    bool m_ram_enabled { false };
};
} // namespace gbcxx
