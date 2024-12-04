#include "core/mbc/mbc1.hpp"

#include "util.hpp"

namespace gbcxx {
Mbc1::Mbc1(std::vector<uint8_t> cartrom)
    : m_rom(std::move(cartrom)),
      m_rom_banks_count(count_rom_banks(m_rom.at(0x148))),
      m_ram_banks_count(count_ram_banks(m_rom.at(0x149))),
      m_ram(static_cast<size_t>(m_rom_banks_count * 0x2000)) {}

uint8_t Mbc1::read_rom(uint16_t address) const {
  const uint8_t bank =
      (address < 0x4000)
          ? (m_banking_mode == 0 ? 0 : (m_rom_bank & 0b0001'1111))
          : m_rom_bank;
  const uint16_t offset = bank * 0x4000 | (address & 0x3FFF);
  if (offset >= m_rom.size()) {
    DIE("MBC1: Invalid ROM read at {:#06x} (bank {:#02x})", address, bank);
  }
  return m_rom.at(offset);
}

uint8_t Mbc1::read_ram(uint16_t address) const {
  if (!m_ram_enabled) return 0xFF;
  const uint8_t ram_bank = m_banking_mode == 1 ? m_ram_bank : 0;
  return m_ram.at((ram_bank * 0x2000) | (address & 0x1FFF));
}

void Mbc1::write_rom(uint16_t address, uint8_t value) {
  if (0x0000 <= address && address <= 0x1FFF) {
    m_ram_enabled = (value & 0xF) == 0xA;
  } else if (0x2000 <= address && address <= 0x3FFF) {
    const uint8_t lower_bits = (value & 0x1F) == 0 ? 1 : (value & 0x1F);
    const uint8_t new_rom_bank =
        ((m_rom_bank & 0x60) | lower_bits) % m_rom_banks_count;
    m_rom_bank = new_rom_bank;
  } else if (0x4000 <= address && address <= 0x5FFF) {
    if (m_rom_banks_count > 32) {
      const uint8_t upper_bits = (value & 0x03) % (m_rom_banks_count >> 5);
      const auto new_rom_bank =
          static_cast<uint8_t>((m_rom_bank & 0x1F) | (upper_bits << 5));
      m_rom_bank = new_rom_bank;
    }
    if (m_ram_banks_count > 1) {
      m_ram_bank = value & 0x03;
    }
  } else if (0x6000 <= address && address <= 0x7FFF) {
    m_banking_mode = value & 0x01;
  } else {
    DIE("MBC1: Unhandled write to ROM {:#04x} <- {:#02x}", address, value);
  }
}

void Mbc1::write_ram(uint16_t address, uint8_t value) {
  if (!m_ram_enabled) return;
  const uint8_t ram_bank = m_banking_mode == 1 ? m_ram_bank : 0;
  m_ram.at(ram_bank * 0x2000 + (address & 0x1FFF)) = value;
}
}  // namespace gbcxx
