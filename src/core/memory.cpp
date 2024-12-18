#include "core/memory.hpp"

#include <cassert>

#include "core/constants.hpp"
#include "util.hpp"

namespace gbcxx {

namespace {
constexpr size_t wram_size = 0x8000;
constexpr size_t hram_size = 0x7F;
} // namespace

// Extracted from SameBoy's DMG boot ROM:
// https://github.com/LIJI32/SameBoy/tree/master/BootROMs/dmg_boot.asm
static const std::array<uint8_t, 0x100> dmg_bootrom = {
    0x31, 0xFE, 0xFF, 0x21, 0x00, 0x80, 0xAF, 0x22, 0xCB, 0x6C, 0x28, 0xFB, 0x3E, 0x80, 0xE0, 0x26,
    0xE0, 0x11, 0x3E, 0xF3, 0xE0, 0x12, 0xE0, 0x25, 0x3E, 0x77, 0xE0, 0x24, 0x3E, 0x54, 0xE0, 0x47,
    0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0x47, 0xCD, 0xA3, 0x00, 0xCD, 0xA3, 0x00, 0x13, 0x7B,
    0xEE, 0x34, 0x20, 0xF2, 0x11, 0xD2, 0x00, 0x0E, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x0D, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF5, 0x3E, 0x1E, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x16, 0x89, 0x0E,
    0x0F, 0xCD, 0xB8, 0x00, 0x7A, 0xCB, 0x2F, 0xCB, 0x2F, 0xE0, 0x42, 0x7A, 0x81, 0x57, 0x79, 0xFE,
    0x08, 0x20, 0x04, 0x3E, 0xA8, 0xE0, 0x47, 0x0D, 0x20, 0xE7, 0x3E, 0xFC, 0xE0, 0x47, 0x3E, 0x83,
    0xCD, 0xCB, 0x00, 0x06, 0x05, 0xCD, 0xC4, 0x00, 0x3E, 0xC1, 0xCD, 0xCB, 0x00, 0x06, 0x3C, 0xCD,
    0xC4, 0x00, 0x21, 0xB0, 0x01, 0xE5, 0xF1, 0x21, 0x4D, 0x01, 0x01, 0x13, 0x00, 0x11, 0xD8, 0x00,
    0xC3, 0xFE, 0x00, 0x3E, 0x04, 0x0E, 0x00, 0xCB, 0x20, 0xF5, 0xCB, 0x11, 0xF1, 0xCB, 0x11, 0x3D,
    0x20, 0xF5, 0x79, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xE5, 0x21, 0x0F, 0xFF, 0xCB, 0x86, 0xCB, 0x46,
    0x28, 0xFC, 0xE1, 0xC9, 0xCD, 0xB8, 0x00, 0x05, 0x20, 0xFA, 0xC9, 0xE0, 0x13, 0x3E, 0x87, 0xE0,
    0x14, 0xC9, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x50,
};

Mmu::Mmu(std::unique_ptr<Mbc> mbc)
    : m_bootrom(dmg_bootrom)
    , m_wram(wram_size)
    , m_hram(hram_size)
    , m_mbc(std::move(mbc))
{
}

auto Mmu::tick() -> void
{
    m_timer.tick();
    m_intf |= m_timer.get_and_clear_interrupts();
    // m_ppu.tick();
    // m_intf |= m_ppu.get_and_clear_interrupts();
}

auto Mmu::read(uint16_t address) const -> uint8_t
{
#ifdef GBCXX_TESTING
    return m_wram.at(address);
#endif

    if (cartridge_start <= address && address <= cartridge_end) {
        if (m_bootrom_enabled && address < sizeof(dmg_bootrom)) {
            return m_bootrom.at(address);
        }
        return m_mbc->read_rom(address);
    }
    if ((Bus_Area::vram_start <= address && address <= Bus_Area::vram_end)
        || (Bus_Area::oam_start <= address && address <= Bus_Area::oam_end)
        || (Io_Reg::lcdc <= address && address <= Io_Reg::vbk)) {
        // return m_ppu.read(address);
    }
    if (Bus_Area::external_ram_start <= address && address <= Bus_Area::external_ram_end) {
        return m_mbc->read_ram(address);
    }
    if (Bus_Area::work_ram_start <= address && address <= Bus_Area::work_ram_end) {
        return m_wram.at(address - Bus_Area::work_ram_start);
    }
    if (Bus_Area::echo_ram_start <= address && address <= Bus_Area::echo_ram_end) {
        return m_wram.at(address & 0x1FFF);
    }
    if (Bus_Area::not_usable_start <= address && address <= Bus_Area::not_usable_end) {
        return 0x00;
    }
    if (Bus_Area::high_ram_start <= address && address <= Bus_Area::high_ram_end) {
        return m_hram.at(address - Bus_Area::high_ram_start);
    }
    if (address == Io_Reg::ie) {
        return m_inte;
    }
    if (address == Io_Reg::bootrom) {
        return m_bootrom_enabled;
    }
    if (Bus_Area::io_start <= address && address <= Bus_Area::io_end) {
        switch (address) {
        case Io_Reg::if_: return m_intf & 0x1F;
        case Io_Reg::joyp:
        case Io_Reg::sb:
        case Io_Reg::sc: return 0xFF;
        case Io_Reg::div:
        case Io_Reg::tima:
        case Io_Reg::tma:
        case Io_Reg::tac: return m_timer.read(address);
        case Io_Reg::nr10:
        case Io_Reg::nr11:
        case Io_Reg::nr12:
        case Io_Reg::nr13:
        case Io_Reg::nr14:
        case Io_Reg::nr21:
        case Io_Reg::nr22:
        case Io_Reg::nr23:
        case Io_Reg::nr24:
        case Io_Reg::nr30:
        case Io_Reg::nr31:
        case Io_Reg::nr32:
        case Io_Reg::nr33:
        case Io_Reg::nr34:
        case Io_Reg::nr41:
        case Io_Reg::nr42:
        case Io_Reg::nr43:
        case Io_Reg::nr44:
        case Io_Reg::nr50:
        case Io_Reg::nr51:
        case Io_Reg::nr52:
        case Io_Reg::wave_pattern_start:
        case Io_Reg::wave_pattern_end: return 0xFF;
        }
    }

    LOG_ERROR("Unhandled read from address {:#06x}", address);
    return 0xFF;
}

auto Mmu::read16(uint16_t address) const -> uint16_t
{
    const uint8_t lo = read(address);
    const uint8_t hi = read(address + 1);
    return static_cast<uint16_t>(hi << 8) | lo;
}

auto Mmu::write(uint16_t address, uint8_t value) -> void
{
#ifdef GBCXX_TESTING
    m_wram.at(address) = value;
    return;
#endif
    if (Bus_Area::cartridge_start <= address && address <= Bus_Area::cartridge_end) {
        if (m_bootrom_enabled && address < sizeof(dmg_bootrom)) {
            LOG_ERROR("Attempted write to boot ROM address {:#06x}", address);
            return;
        }
        m_mbc->write_rom(address, value);
    }
    else if ((Bus_Area::vram_start <= address && address <= Bus_Area::vram_end)
             || (Bus_Area::oam_start <= address && address <= Bus_Area::oam_end)
             || (Io_Reg::lcdc <= address && address <= Io_Reg::vbk)) {
        // m_ppu.write(address, value);
    }
    else if (Bus_Area::external_ram_start <= address && address <= Bus_Area::external_ram_end) {
        m_mbc->write_ram(address, value);
    }
    else if (Bus_Area::work_ram_start <= address && address <= Bus_Area::work_ram_end) {
        m_wram.at(address - Bus_Area::work_ram_start) = value;
    }
    else if (Bus_Area::echo_ram_start <= address && address <= Bus_Area::echo_ram_end) {
        m_wram.at(address & 0x1FFF) = value;
    }
    else if (Bus_Area::not_usable_start <= address && address <= Bus_Area::not_usable_end) {
        return;
    }
    else if (Bus_Area::high_ram_start <= address && address <= Bus_Area::high_ram_end) {
        m_hram.at(address - Bus_Area::high_ram_start) = value;
    }
    else if (address == Io_Reg::ie) {
        m_inte = value;
    }
    else if (address == Io_Reg::bootrom) {
        m_bootrom_enabled = value;
    }
    else if (Bus_Area::io_start <= address && address <= Bus_Area::io_end) {
        switch (address) {
        case Io_Reg::if_: m_intf = value & 0x1F; return;
        case Io_Reg::bootrom: m_bootrom_enabled = value; return;
        case Io_Reg::joyp:
        case Io_Reg::sb:
        case Io_Reg::sc: return;
        case Io_Reg::div:
        case Io_Reg::tima:
        case Io_Reg::tma:
        case Io_Reg::tac: m_timer.write(address, value); return;
        case Io_Reg::nr10:
        case Io_Reg::nr11:
        case Io_Reg::nr12:
        case Io_Reg::nr13:
        case Io_Reg::nr14:
        case Io_Reg::nr21:
        case Io_Reg::nr22:
        case Io_Reg::nr23:
        case Io_Reg::nr24:
        case Io_Reg::nr30:
        case Io_Reg::nr31:
        case Io_Reg::nr32:
        case Io_Reg::nr33:
        case Io_Reg::nr34:
        case Io_Reg::nr41:
        case Io_Reg::nr42:
        case Io_Reg::nr43:
        case Io_Reg::nr44:
        case Io_Reg::nr50:
        case Io_Reg::nr51:
        case Io_Reg::nr52:
        case Io_Reg::wave_pattern_start:
        case Io_Reg::wave_pattern_end: return;
        default: LOG_ERROR("MMU: Unhandled write to address {:#06x}", address);
        }
    }
}

auto Mmu::write16(uint16_t address, uint16_t value) -> void
{
    write(address, static_cast<uint8_t>(value));
    write(address + 1, static_cast<uint8_t>(value >> 8));
}

} // namespace gbcxx
