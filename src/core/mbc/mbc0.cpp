#include "core/mbc/mbc0.hpp"

namespace gbcxx {
Mbc0::Mbc0(std::vector<uint8_t> cartrom) : m_rom(std::move(cartrom)) {}
uint8_t Mbc0::read_rom(uint16_t address) const { return m_rom.at(address); }
uint8_t Mbc0::read_ram(uint16_t /*address*/) const { return 0; }
void Mbc0::write_rom(uint16_t /*address*/, uint8_t /*value*/) { /* do nothing */
}
void Mbc0::write_ram(uint16_t /*address*/, uint8_t /*value*/) { /* do nothing */
}
}  // namespace gbcxx
