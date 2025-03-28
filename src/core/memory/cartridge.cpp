#include "core/memory/cartridge.hpp"

#include "core/memory/mbc/mbc0.hpp"
#include "core/memory/mbc/mbc1.hpp"
#include "core/memory/mbc/mbc3.hpp"
#include "core/util.hpp"

namespace gb::memory
{
Cartridge Cartridge::FromRom(std::vector<uint8_t> rom)
{
    switch (rom.at(0x147))
    {
    case 0x00:
    {
        LOG_INFO("Cartridge: Type MBC0");
        return {.mbc = std::make_unique<Mbc0>(std::move(rom))};
    }
    case 0x01:
    case 0x02:
    case 0x03:
    {
        LOG_INFO("Cartridge: Type MBC1");
        return {.mbc = std::make_unique<Mbc1>(std::move(rom))};
    }
    case 0x0f:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    {
        LOG_INFO("Cartridge: Type MBC3");
        return {.mbc = std::make_unique<Mbc3>(std::move(rom))};
    }
    default: DIE("MBC: Unimplemented cartridge type {:X}", rom[0x147]);
    }
}

size_t CountRamBanks(uint8_t val)
{
    switch (val)
    {
    case 1:
    case 2: return 1;
    case 3: return 4;
    case 4: return 16;
    case 5: return 8;
    default: return 0;
    }
}

size_t CountRomBanks(uint8_t val)
{
    if (val <= 8) return static_cast<size_t>(2) << val;
    return 0;
}

}  // namespace gb::memory
