#include "core/mbc/mbc0.hpp"

namespace gbcxx {
Mbc0::Mbc0(std::vector<uint8_t> cartrom)
    : m_rom(std::move(cartrom))
{
}

auto Mbc0::read_rom(uint16_t address) const -> uint8_t
{
    return m_rom.at(address);
}
auto Mbc0::read_ram(uint16_t /*address*/) const -> uint8_t
{
    return 0;
}
auto Mbc0::write_rom(uint16_t /*address*/, uint8_t /*value*/) -> void { }
auto Mbc0::write_ram(uint16_t /*address*/, uint8_t /*value*/) -> void { }
} // namespace gbcxx
