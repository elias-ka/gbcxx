#include "core/mbc/mbc1.hpp"

namespace gbcxx {
Mbc1::Mbc1(std::vector<u8> cartrom)
    : m_rom(std::move(cartrom)),
      m_rom_banks_count(count_rom_banks(m_rom.at(0x148))),
      m_ram_banks_count(count_ram_banks(m_rom.at(0x149))),
      m_ram(static_cast<usz>(m_rom_banks_count * 0x2000)) {}

u8 Mbc1::read_rom(u16 address) const {
  const u8 bank = (address < 0x4000)
                      ? (m_banking_mode == 0 ? 0 : (m_rom_bank & 0b0001'1111))
                      : m_rom_bank;
  const u16 offset = bank * 0x4000 | (address & 0x3FFF);
  if (offset >= m_rom.size()) {
    DIE("MBC1: Invalid ROM read at {:#06x} (bank {:#02x})", address, bank);
  }
  LOG_TRACE("MBC1::read_rom({:#06x}), bank {:#02x} -> {:#02x}", address, bank,
            m_rom.at(offset));
  return m_rom.at(offset);
}

u8 Mbc1::read_ram(u16 address) const {
  if (!m_ram_enabled) return 0xFF;
  const u8 ram_bank = m_banking_mode == 1 ? m_ram_bank : 0;
  LOG_TRACE("MBC1::read_ram({:#06x}), bank {:#02x} -> {:#02x}", address,
            ram_bank, m_ram.at((ram_bank * 0x2000) | (address & 0x1FFF)));
  return m_ram.at((ram_bank * 0x2000) | (address & 0x1FFF));
}

void Mbc1::write_rom(u16 address, u8 value) {
  if (0x0000 <= address && address <= 0x1FFF) {
    bool new_ram_enabled = (value & 0xF) == 0xA;
    LOG_TRACE("MBC1: RAM enable {} -> {}", m_ram_enabled, new_ram_enabled);
    m_ram_enabled = (value & 0xF) == 0xA;
  } else if (0x2000 <= address && address <= 0x3FFF) {
    const u8 lower_bits = (value & 0x1F) == 0 ? 1 : (value & 0x1F);
    const u8 new_rom_bank =
        ((m_rom_bank & 0x60) | lower_bits) % m_rom_banks_count;
    LOG_TRACE("MBC1: ROM bank {} -> {}", m_rom_bank, new_rom_bank);
    m_rom_bank = new_rom_bank;
  } else if (0x4000 <= address && address <= 0x5FFF) {
    if (m_rom_banks_count > 32) {
      const u8 upper_bits = (value & 0x03) % (m_rom_banks_count >> 5);
      const u8 new_rom_bank =
          static_cast<u8>((m_rom_bank & 0x1F) | (upper_bits << 5));
      LOG_TRACE("MBC1: ROM bank {} -> {}", m_rom_bank, new_rom_bank);
      m_rom_bank = new_rom_bank;
    }
    if (m_ram_banks_count > 1) {
      const u8 new_ram_bank = value & 0x03;
      LOG_TRACE("MBC1: RAM bank {} -> {}", m_ram_bank, new_ram_bank);
      m_ram_bank = new_ram_bank;
    }
  } else if (0x6000 <= address && address <= 0x7FFF) {
    const u8 banking_mode = value & 0x01;
    LOG_TRACE("MBC1: Banking mode {} -> {}", m_banking_mode, banking_mode);
    m_banking_mode = banking_mode;
  } else {
    DIE("MBC1: Unhandled write to ROM {:#04x} <- {:#02x}", address, value);
  }
}

void Mbc1::write_ram(u16 address, u8 value) {
  if (!m_ram_enabled) return;
  const u8 ram_bank = m_banking_mode == 1 ? m_ram_bank : 0;
  m_ram.at(ram_bank * 0x2000 + (address & 0x1FFF)) = value;
}
}  // namespace gbcxx
