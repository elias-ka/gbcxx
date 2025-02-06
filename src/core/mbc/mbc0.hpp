#pragma once

#include "core/mbc.hpp"

namespace gb
{
class Mbc0 final : public Mbc
{
public:
    explicit Mbc0(std::vector<uint8_t> cartrom);

    [[nodiscard]] uint8_t ReadRom(uint16_t addr) const override;
    [[nodiscard]] uint8_t ReadRam(uint16_t addr) const override;
    void WriteRom(uint16_t addr, uint8_t val) override;
    void WriteRam(uint16_t addr, uint8_t val) override;

private:
    std::vector<uint8_t> rom_;
};

}  // namespace gb
