#pragma once

#include "core/memory/cartridge.hpp"

namespace gb::memory
{
class Mbc3 : public Mbc
{
public:
    explicit Mbc3(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;

    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

    void LoadRam(std::ifstream& save_file) override;
    void SaveRam(std::ofstream& save_file) const override;

private:
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;
    size_t rom_bank_{1};
    size_t ram_bank_{0};
    bool ram_enabled_{false};
};
}  // namespace gb::memory
