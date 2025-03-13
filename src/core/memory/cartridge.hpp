#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "core/constants.hpp"
#include "core/util.hpp"

namespace gb::memory
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
    virtual void LoadRam(std::vector<uint8_t> ram) = 0;
};

[[nodiscard]] size_t CountRamBanks(uint8_t val);
[[nodiscard]] size_t CountRomBanks(uint8_t val);

struct Cartridge
{
    std::unique_ptr<Mbc> mbc;

    static Cartridge MakeFromRom(std::vector<uint8_t> rom_data);

    void LoadGame(std::vector<uint8_t> ram_data) const { mbc->LoadRam(std::move(ram_data)); }

    [[nodiscard]] uint8_t ReadByte(uint16_t addr) const
    {
        if (addr >= kCartridgeStart && addr <= kCartridgeEnd)
            return mbc->ReadRom(addr);

        if (addr >= kExternalRamStart && addr <= kExternalRamEnd)
            return mbc->ReadRam(addr);

        LOG_ERROR("Cartrdige: Unmapped read {:X}", addr);
        return 0x00;
    }

    void WriteByte(uint16_t addr, uint8_t val) const
    {
        if (addr >= kCartridgeStart && addr <= kCartridgeEnd)
            mbc->WriteRom(addr, val);
        else if (addr >= kExternalRamStart && addr <= kExternalRamEnd)
            mbc->WriteRam(addr, val);
        else
            LOG_ERROR("Cartrdige: Unmapped write {:X} <- {:X}", addr, val);
    }
};

}  // namespace gb::memory
