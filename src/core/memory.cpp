#include "core/memory.hpp"

#include <cassert>
#include <print>

#include "core/constants.hpp"
#include "util.hpp"

namespace gbcxx {

namespace {
constexpr size_t wram_size = 0x8000;
constexpr size_t hram_size = 0x7f;
}  // namespace

// Extracted from SameBoy's DMG boot ROM:
// https://github.com/LIJI32/SameBoy/tree/master/BootROMs/dmg_boot.asm
static const std::array<uint8_t, 0x100> dmg_bootrom = {
    0x31, 0xfe, 0xff, 0x21, 0x00, 0x80, 0xaf, 0x22, 0xcb, 0x6c, 0x28, 0xfb,
    0x3e, 0x80, 0xe0, 0x26, 0xe0, 0x11, 0x3e, 0xf3, 0xe0, 0x12, 0xe0, 0x25,
    0x3e, 0x77, 0xe0, 0x24, 0x3e, 0x54, 0xe0, 0x47, 0x11, 0x04, 0x01, 0x21,
    0x10, 0x80, 0x1a, 0x47, 0xcd, 0xa3, 0x00, 0xcd, 0xa3, 0x00, 0x13, 0x7b,
    0xee, 0x34, 0x20, 0xf2, 0x11, 0xd2, 0x00, 0x0e, 0x08, 0x1a, 0x13, 0x22,
    0x23, 0x0d, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99,
    0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18,
    0xf5, 0x3e, 0x1e, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x16, 0x89, 0x0e,
    0x0f, 0xcd, 0xb8, 0x00, 0x7a, 0xcb, 0x2f, 0xcb, 0x2f, 0xe0, 0x42, 0x7a,
    0x81, 0x57, 0x79, 0xfe, 0x08, 0x20, 0x04, 0x3e, 0xa8, 0xe0, 0x47, 0x0d,
    0x20, 0xe7, 0x3e, 0xfc, 0xe0, 0x47, 0x3e, 0x83, 0xcd, 0xcb, 0x00, 0x06,
    0x05, 0xcd, 0xc4, 0x00, 0x3e, 0xc1, 0xcd, 0xcb, 0x00, 0x06, 0x3c, 0xcd,
    0xc4, 0x00, 0x21, 0xb0, 0x01, 0xe5, 0xf1, 0x21, 0x4d, 0x01, 0x01, 0x13,
    0x00, 0x11, 0xd8, 0x00, 0xc3, 0xfe, 0x00, 0x3e, 0x04, 0x0e, 0x00, 0xcb,
    0x20, 0xf5, 0xcb, 0x11, 0xf1, 0xcb, 0x11, 0x3d, 0x20, 0xf5, 0x79, 0x22,
    0x23, 0x22, 0x23, 0xc9, 0xe5, 0x21, 0x0f, 0xff, 0xcb, 0x86, 0xcb, 0x46,
    0x28, 0xfc, 0xe1, 0xc9, 0xcd, 0xb8, 0x00, 0x05, 0x20, 0xfa, 0xc9, 0xe0,
    0x13, 0x3e, 0x87, 0xe0, 0x14, 0xc9, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5,
    0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xe0, 0x50,
};

Mmu::Mmu(std::unique_ptr<Mbc> mbc)
    : m_bootrom(dmg_bootrom),
      m_wram(wram_size),
      m_hram(hram_size),
      m_mbc(std::move(mbc)) {}

void Mmu::tick() {
  static int mmu_ticks = 0;
  mmu_ticks++;
  m_ppu.tick();
  m_intf |= m_ppu.get_and_clear_interrupts();
  if (mmu_ticks == 4) {
    m_timer.tick();
    m_intf |= m_timer.get_and_clear_interrupts();
    mmu_ticks = 0;
  }
}

uint8_t Mmu::read(uint16_t address) const {
#ifdef GBCXX_TESTING
  return m_wram.at(address);
#endif

  if (cartridge_start <= address && address <= cartridge_end) {
    if (m_bootrom_enabled && address < dmg_bootrom.size()) {
      return m_bootrom[address];
    }
    return m_mbc->read_rom(address);
  }
  if ((vram_start <= address && address <= vram_end) ||
      (oam_start <= address && address <= oam_end) ||
      (reg_lcdc <= address && address <= reg_vbk)) {
    return m_ppu.read(address);
  }
  if (external_ram_start <= address && address <= external_ram_end) {
    return m_mbc->read_ram(address);
  }
  if (work_ram_start <= address && address <= work_ram_end) {
    return m_wram.at(address - work_ram_start);
  }
  if (echo_ram_start <= address && address <= echo_ram_end) {
    return m_wram.at(address - 0x1fff);
  }
  if (not_usable_start <= address && address <= not_usable_end) {
    return 0x00;
  }
  if (high_ram_start <= address && address <= high_ram_end) {
    return m_hram.at(address & 0x7f);
  }
  if (address == reg_ie) {
    return m_inte;
  }
  if (address == reg_bootrom) {
    return m_bootrom_enabled;
  }
  if (io_start <= address && address <= io_end) {
    switch (address) {
      case reg_if: return m_intf;
      case reg_joyp:
      case reg_sb:
      case reg_sc: return 0xff;
      case reg_div:
      case reg_tima:
      case reg_tma:
      case reg_tac: return m_timer.read(address);
      case reg_nr10:
      case reg_nr11:
      case reg_nr12:
      case reg_nr13:
      case reg_nr14:
      case reg_nr21:
      case reg_nr22:
      case reg_nr23:
      case reg_nr24:
      case reg_nr30:
      case reg_nr31:
      case reg_nr32:
      case reg_nr33:
      case reg_nr34:
      case reg_nr41:
      case reg_nr42:
      case reg_nr43:
      case reg_nr44:
      case reg_nr50:
      case reg_nr51:
      case reg_nr52:
      case wave_pattern_start:
      case wave_pattern_end: return 0xFF;
      default: break;
    }
  }

  LOG_ERROR("MMU: Unmapped read {:X}", address);
  return 0xFF;
}

uint16_t Mmu::read16(uint16_t address) const {
  const uint8_t lo = read(address);
  const uint8_t hi = read(address + 1);
  return uint16_t(hi << 8) | lo;
}

void Mmu::write(uint16_t address, uint8_t value) {
#ifdef GBCXX_TESTING
  m_wram.at(address) = value;
  return;
#endif
  if (cartridge_start <= address && address <= cartridge_end) {
    if (m_bootrom_enabled && address < sizeof(dmg_bootrom)) {
      LOG_ERROR("MMU: Tried writing to boot ROM address {:X}", address);
      return;
    }
    m_mbc->write_rom(address, value);
  } else if ((vram_start <= address && address <= vram_end) ||
             (oam_start <= address && address <= oam_end) ||
             (reg_lcdc <= address && address <= reg_vbk)) {
    m_ppu.write(address, value);
  } else if (external_ram_start <= address && address <= external_ram_end) {
    m_mbc->write_ram(address, value);
  } else if (work_ram_start <= address && address <= work_ram_end) {
    m_wram.at(address - work_ram_start) = value;
  } else if (echo_ram_start <= address && address <= echo_ram_end) {
    m_wram.at(address & 0x1fff) = value;
  } else if (not_usable_start <= address && address <= not_usable_end) {
    return;
  } else if (high_ram_start <= address && address <= high_ram_end) {
    m_hram.at(address - high_ram_start) = value;
  } else if (address == reg_ie) {
    m_inte = value;
  } else if (address == reg_bootrom) {
    m_bootrom_enabled = value;
  } else if (io_start <= address && address <= io_end) {
    switch (address) {
      case reg_if: m_intf = value; return;
      case reg_bootrom: m_bootrom_enabled = value; return;
      case reg_joyp:
      case reg_sb: {
        m_serial_buffer.push_back(char(value));
        if (value == '\n') {
          LOG_DEBUG("Serial: {}", std::string{m_serial_buffer.data(),
                                              m_serial_buffer.size()});
          m_serial_buffer.clear();
        }
        break;
      }
      case reg_sc: return;
      case reg_div:
      case reg_tima:
      case reg_tma:
      case reg_tac: m_timer.write(address, value); return;
      case reg_nr10:
      case reg_nr11:
      case reg_nr12:
      case reg_nr13:
      case reg_nr14:
      case reg_nr21:
      case reg_nr22:
      case reg_nr23:
      case reg_nr24:
      case reg_nr30:
      case reg_nr31:
      case reg_nr32:
      case reg_nr33:
      case reg_nr34:
      case reg_nr41:
      case reg_nr42:
      case reg_nr43:
      case reg_nr44:
      case reg_nr50:
      case reg_nr51:
      case reg_nr52:
      case wave_pattern_start:
      case wave_pattern_end: return;
      default: LOG_ERROR("MMU: Unmapped write {:X} <- {:X}", address, value);
    }
  }
}

void Mmu::write16(uint16_t address, uint16_t value) {
  write(address, uint8_t(value));
  write(address + 1, uint8_t(value >> 8));
}

}  // namespace gbcxx
