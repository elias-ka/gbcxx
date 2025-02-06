#include "core/mbc.hpp"

#include "core/mbc/mbc0.hpp"
#include "core/mbc/mbc1.hpp"
#include "util.hpp"

namespace gb
{
std::unique_ptr<Mbc> Mbc::MakeFromRom(std::vector<uint8_t> rom)
{
    switch (rom.at(0x147))
    {
        case 0x00:
        {
            LOG_INFO("MBC: Cartridge type MBC0");
            return std::make_unique<Mbc0>(std::move(rom));
        }
        case 0x01:
        case 0x02:
        case 0x03:
        {
            LOG_INFO("MBC: Cartridge type MBC1");
            return std::make_unique<Mbc1>(std::move(rom));
        }
        default: DIE("MBC: Unimplemented cartridge type {:X}", rom[0x147]);
    }
}

uint8_t CountRamBanks(uint8_t val)
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

uint8_t CountRomBanks(uint8_t val)
{
    if (val <= 8)
    {
        return static_cast<uint8_t>(2 << val);
    }
    return 0;
}

}  // namespace gb
