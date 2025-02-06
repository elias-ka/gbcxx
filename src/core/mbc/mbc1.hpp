#pragma once

#include "core/mbc.hpp"

namespace gb
{
class Mbc1 final : public Mbc
{
public:
    explicit Mbc1(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;
    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

private:
    std::vector<uint8_t> rom_;
    uint8_t banking_mode_{0};
    uint8_t rom_bank_{1};
    uint8_t ram_bank_{0};
    uint8_t nr_rom_banks_{0};
    uint8_t nr_ram_banks_{0};
    std::vector<uint8_t> ram_;
    bool ram_enabled_{false};
};
}  // namespace gb
