#pragma once

#include "core/memory/cartridge.hpp"

namespace gb::memory
{
class Mbc0 final : public Mbc
{
public:
    explicit Mbc0(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;

    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

    void LoadRam(std::ifstream& save_file) override;
    void SaveRam(std::ofstream& save_file) const override;

private:
    std::vector<uint8_t> rom_;
};

}  // namespace gb::memory
