#include "core/mbc.hpp"

#include "core/mbc/mbc0.hpp"
#include "core/mbc/mbc1.hpp"
#include "util.hpp"

namespace gbcxx {
auto make_mbc(std::vector<uint8_t> cartrom) -> std::unique_ptr<Mbc>
{
    switch (cartrom.at(0x147)) {
    case 0x00: {
        LOG_INFO("Selected MBC0");
        return std::make_unique<Mbc0>(std::move(cartrom));
    }
    case 0x01:
    case 0x02:
    case 0x03: {
        LOG_INFO("Selected MBC1");
        return std::make_unique<Mbc1>(std::move(cartrom));
    }
    }
    DIE("Unimplemented cartridge type {:#02x}", cartrom[0x0147]);
}

auto count_ram_banks(uint8_t value) -> uint8_t
{
    switch (value) {
    case 1:
    case 2: return 1;
    case 3: return 4;
    case 4: return 16;
    case 5: return 8;
    default: return 0;
    }
}

auto count_rom_banks(uint8_t value) -> uint8_t
{
    if (value < 9) {
        return uint8_t(2 << value);
    }
    return 0;
}

} // namespace gbcxx
