#include "core/mbc/mbc0.hpp"

namespace gbcxx {
Mbc0::Mbc0(std::vector<u8> cartrom) : m_rom(std::move(cartrom)), m_ram(8_KiB) {}
u8 Mbc0::read_rom(u16 address) const { return m_rom.at(address); }
u8 Mbc0::read_ram(u16 address) const { return m_ram.at(address); }
void Mbc0::write_rom(u16, u8) { /* do nothing */ }
void Mbc0::write_ram(u16 address, u8 value) { m_ram.at(address) = value; }
}  // namespace gbcxx
