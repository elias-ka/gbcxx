#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace gb
{
class Mbc
{
public:
    static std::unique_ptr<Mbc> MakeFromRom(std::vector<uint8_t> rom);

    virtual ~Mbc() = default;

    [[nodiscard]] virtual uint8_t ReadRom(uint16_t addr) const = 0;
    [[nodiscard]] virtual uint8_t ReadRam(uint16_t addr) const = 0;
    virtual void WriteRom(uint16_t addr, uint8_t val) = 0;
    virtual void WriteRam(uint16_t addr, uint8_t val) = 0;
};

[[nodiscard]] uint8_t CountRamBanks(uint8_t val);
[[nodiscard]] uint8_t CountRomBanks(uint8_t val);

}  // namespace gb
