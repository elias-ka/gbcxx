#include "core/memory.hpp"

#include <cassert>

#include "core/constants.hpp"
#include "util.hpp"

namespace gbcxx {

namespace {
const usz WRAM_SIZE = 0x8000;
const usz HRAM_SIZE = 0x7F;
}  // namespace

// Extracted from SameBoy's DMG boot ROM:
// https://github.com/LIJI32/SameBoy/tree/master/BootROMs/dmg_boot.asm
static const std::array<u8, 0x100> DMG_BOOTROM = {
    0x31, 0xFE, 0xFF, 0x21, 0x00, 0x80, 0xAF, 0x22, 0xCB, 0x6C, 0x28, 0xFB,
    0x3E, 0x80, 0xE0, 0x26, 0xE0, 0x11, 0x3E, 0xF3, 0xE0, 0x12, 0xE0, 0x25,
    0x3E, 0x77, 0xE0, 0x24, 0x3E, 0x54, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21,
    0x10, 0x80, 0x1A, 0x47, 0xCD, 0xA3, 0x00, 0xCD, 0xA3, 0x00, 0x13, 0x7B,
    0xEE, 0x34, 0x20, 0xF2, 0x11, 0xD2, 0x00, 0x0E, 0x08, 0x1A, 0x13, 0x22,
    0x23, 0x0D, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
    0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18,
    0xF5, 0x3E, 0x1E, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x16, 0x89, 0x0E,
    0x0F, 0xCD, 0xB8, 0x00, 0x7A, 0xCB, 0x2F, 0xCB, 0x2F, 0xE0, 0x42, 0x7A,
    0x81, 0x57, 0x79, 0xFE, 0x08, 0x20, 0x04, 0x3E, 0xA8, 0xE0, 0x47, 0x0D,
    0x20, 0xE7, 0x3E, 0xFC, 0xE0, 0x47, 0x3E, 0x83, 0xCD, 0xCB, 0x00, 0x06,
    0x05, 0xCD, 0xC4, 0x00, 0x3E, 0xC1, 0xCD, 0xCB, 0x00, 0x06, 0x3C, 0xCD,
    0xC4, 0x00, 0x21, 0xB0, 0x01, 0xE5, 0xF1, 0x21, 0x4D, 0x01, 0x01, 0x13,
    0x00, 0x11, 0xD8, 0x00, 0xC3, 0xFE, 0x00, 0x3E, 0x04, 0x0E, 0x00, 0xCB,
    0x20, 0xF5, 0xCB, 0x11, 0xF1, 0xCB, 0x11, 0x3D, 0x20, 0xF5, 0x79, 0x22,
    0x23, 0x22, 0x23, 0xC9, 0xE5, 0x21, 0x0F, 0xFF, 0xCB, 0x86, 0xCB, 0x46,
    0x28, 0xFC, 0xE1, 0xC9, 0xCD, 0xB8, 0x00, 0x05, 0x20, 0xFA, 0xC9, 0xE0,
    0x13, 0x3E, 0x87, 0xE0, 0x14, 0xC9, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5,
    0x42, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0x50,
};

Mmu::Mmu(std::unique_ptr<Mbc> mbc)
    : m_bootrom(DMG_BOOTROM),
      m_wram(WRAM_SIZE),
      m_hram(HRAM_SIZE),
      m_mbc(std::move(mbc)) {}

void Mmu::tick() {
  m_ppu.tick();
  m_intf |= m_ppu.get_and_clear_interrupts();
}

u8 Mmu::read(u16 address) const {
#ifdef TESTING
  return m_wram.at(address);
#endif

  if (CARTRIDGE_START <= address && address <= CARTRIDGE_END) {
    if (m_bootrom_enabled && address < sizeof(DMG_BOOTROM)) {
      return m_bootrom.at(address);
    }
    return m_mbc->read_rom(address);
  } else if ((VRAM_START <= address && address <= VRAM_END) ||
             (OAM_START <= address && address <= OAM_END) ||
             (REG_LCDC <= address && address <= REG_VBK)) {
    return m_ppu.read(address);
  } else if (EXTERNAL_RAM_START <= address && address <= EXTERNAL_RAM_END) {
    return m_mbc->read_ram(address);
  } else if (WORK_RAM_START <= address && address <= WORK_RAM_END) {
    return m_wram.at(address - WORK_RAM_START);
  } else if (ECHO_RAM_START <= address && address <= ECHO_RAM_END) {
    return m_wram.at(address - 0x1FFF);
  } else if (NOT_USABLE_START <= address && address <= NOT_USABLE_END) {
    return 0xFF;
  } else if (HIGH_RAM_START <= address && address <= HIGH_RAM_END) {
    return m_hram.at(address & 0x7F);
  }

  switch (address) {
    case REG_IE: return m_inte;
    case REG_IF: return m_intf;
    case REG_JOYP:
    case REG_SB:
    case REG_SC:
    case REG_DIV:
    case REG_TIMA:
    case REG_TMA:
    case REG_TAC:
    case REG_NR10:
    case REG_NR11:
    case REG_NR12:
    case REG_NR13:
    case REG_NR14:
    case REG_NR21:
    case REG_NR22:
    case REG_NR23:
    case REG_NR24:
    case REG_NR30:
    case REG_NR31:
    case REG_NR32:
    case REG_NR33:
    case REG_NR34:
    case REG_NR41:
    case REG_NR42:
    case REG_NR43:
    case REG_NR44:
    case REG_NR50:
    case REG_NR51:
    case REG_NR52:
    case WAVE_PATTERN_START:
    case WAVE_PATTERN_END: return 0xFF;
  }

  LOG_ERROR("Unhandled read from address {:#06x}", address);
  return 0xFF;
}

u16 Mmu::read16(u16 address) const {
  const auto lo = static_cast<u16>(read(address));
  const auto hi = static_cast<u16>(read(address + 1));
  return static_cast<u16>(hi << 8) | lo;
}

void Mmu::write(u16 address, u8 data) {
#ifdef TESTING
  m_wram.at(address) = data;
  return;
#endif
  if (CARTRIDGE_START <= address && address <= CARTRIDGE_END) {
    if (m_bootrom_enabled && address < sizeof(DMG_BOOTROM)) {
      LOG_ERROR("Attempted write to boot ROM address {:#06x}", address);
      return;
    }
    m_mbc->write_rom(address, data);
  } else if ((VRAM_START <= address && address <= VRAM_END) ||
             (OAM_START <= address && address <= OAM_END) ||
             (REG_LCDC <= address && address <= REG_VBK)) {
    m_ppu.write(address, data);
  } else if (EXTERNAL_RAM_START <= address && address <= EXTERNAL_RAM_END) {
    m_mbc->write_ram(address, data);
  } else if (WORK_RAM_START <= address && address <= WORK_RAM_END) {
    m_wram.at(address - WORK_RAM_START) = data;
  } else if (ECHO_RAM_START <= address && address <= ECHO_RAM_END) {
    m_wram.at(address & 0x1FFF) = data;
  } else if (NOT_USABLE_START <= address && address <= NOT_USABLE_END) {
    return;
  } else if (HIGH_RAM_START <= address && address <= HIGH_RAM_END)
    m_hram.at(address & 0x7F) = data;
  else {
    switch (address) {
      case REG_BOOTROM: m_bootrom_enabled = data; return;
      case REG_IE: m_inte = data; return;
      case REG_IF: m_intf = data; return;
      case REG_JOYP:
      case REG_SB:
      case REG_SC:
      case REG_DIV:
      case REG_TIMA:
      case REG_TMA:
      case REG_TAC:
      case REG_NR10:
      case REG_NR11:
      case REG_NR12:
      case REG_NR13:
      case REG_NR14:
      case REG_NR21:
      case REG_NR22:
      case REG_NR23:
      case REG_NR24:
      case REG_NR30:
      case REG_NR31:
      case REG_NR32:
      case REG_NR33:
      case REG_NR34:
      case REG_NR41:
      case REG_NR42:
      case REG_NR43:
      case REG_NR44:
      case REG_NR50:
      case REG_NR51:
      case REG_NR52:
      case WAVE_PATTERN_START:
      case WAVE_PATTERN_END: return;
      default: LOG_ERROR("MMU: Unhandled write to address {:#06x}", address);
    }
  }
}

void Mmu::write16(u16 addr, u16 data) {
  write(addr, static_cast<u8>(data));
  write(addr + 1, static_cast<u8>(data >> 8));
}

}  // namespace gbcxx
