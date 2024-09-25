#include "core/memory.h"
#include "util.h"

namespace cb::memory
{
    const std::array<u8, 256> default_bios = {
        0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF,
        0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E,
        0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96,
        0x00, 0x13, 0x7B, 0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22,
        0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D,
        0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0,
        0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20,
        0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
        0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0,
        0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, 0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F,
        0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23,
        0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
        0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E,
        0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC,
        0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C, 0x21,
        0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
        0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0,
        0x50};

    u8 cartridge::read(uint16_t addr) const
    {
        return cb::variant_match(mbc,
                                 [&](const mbc_rom_only& mapper) { return mapper.rom.at(addr); });
    }

    u8 cartridge::read_external_ram(uint16_t addr) const
    {
        return cb::variant_match(mbc,
                                 [&](const mbc_rom_only& mapper) { return mapper.ram.at(addr); });
    }

    void cartridge::write_external_ram(uint16_t addr, u8 value)
    {
        cb::variant_match(mbc, [&](mbc_rom_only& mapper) { mapper.rom.at(addr) = value; });
    }

    mmu::mmu(bios_t bios)
        : m_bios(MOV(bios))
    {
    }

    u8 mmu::read8(u16 addr) const
    {
        if (interval(0x000, 0x7FFF).contains(addr)) // ROM
        {
            if (!static_cast<bool>(m_bootrom_disabled) && addr <= 0xFF)
                return m_bios.at(addr);

            return m_cartridge.read(addr);
        }

        if (interval(0x8000, 0x9FFF).contains(addr)) // VRAM
            return m_ppu.read8(addr);

        if (interval(0xA000, 0xBFFF).contains(addr)) // External RAM
            return m_cartridge.read_external_ram(addr - 0xA000);

        if (interval(0xC000, 0xCFFF).contains(addr)) // Work RAM
            return m_wram.at(addr - 0xC000);

        if (interval(0xD000, 0xDFFF).contains(addr)) // Work RAM
            return m_wram.at(addr - 0xD000);

        if (interval(0xE000, 0xFDFF).contains(addr)) // Echo RAM
            return read8(addr - 0x2000);

        if (interval(0xFE00, 0xFE9F).contains(addr)) // OAM
        {
            LOG_UNIMPLEMENTED("read from OAM");
            return 0xFF;
        }

        if (interval(0xFEA0, 0xFEFF).contains(addr)) // Not usable
            return 0xFF;

        if (interval(0xFF00, 0xFF7F).contains(addr)) // I/O registers
            return read8_io(addr);

        if (interval(0xFF80, 0xFFFE).contains(addr)) // High RAM
            return m_hram.at(addr - 0xFF80);

        if (addr == 0xFFFF) // Interrupt Enable register
        {
            LOG_UNIMPLEMENTED("read from Interrupt Enable register");
            return 0xFF;
        }

        LOG_DEBUG("unmapped read from {:#04x}", addr);
        return 0xFF;
    }

    u16 mmu::read16(u16 addr) const
    {
        const auto lo = static_cast<u16>(read8(addr));
        const auto hi = static_cast<u16>(read8(addr + 1));
        return static_cast<u16>(hi << 8) | lo;
    }

    void mmu::write8(u16 addr, u8 data)
    {
        if (interval(0x8000, 0x9FFF).contains(addr)) // VRAM
        {
            m_ppu.write8(addr, data);
        }
        else if (interval(0xA000, 0xBFFF).contains(addr)) // External RAM
        {
            m_cartridge.write_external_ram(addr - 0xA000, data);
        }
        else if (interval(0xC000, 0xCFFF).contains(addr)) // Work RAM
        {
            m_wram.at(addr - 0xC000) = data;
        }
        else if (interval(0xD000, 0xDFFF).contains(addr)) // Work RAM
        {
            m_wram.at(addr - 0xD000) = data;
        }
        else if (interval(0xE000, 0xFDFF).contains(addr)) // Echo RAM
        {
            write8(addr - 0x2000, data);
        }
        else if (interval(0xFE00, 0xFE9F).contains(addr)) // OAM
        {
            LOG_UNIMPLEMENTED("write to OAM");
            return;
        }
        else if (interval(0xFEA0, 0xFEFF).contains(addr)) // Not usable
        {
            return;
        }
        else if (interval(0xFF00, 0xFF7F).contains(addr)) // I/O registers
        {
            write8_io(addr, data);
        }
        else if (interval(0xFF80, 0xFFFE).contains(addr)) // High RAM
        {
            m_hram.at(addr - 0xFF80) = data;
        }
        else if (addr == 0xFFFF) // Interrupt Enable register
        {
            LOG_UNIMPLEMENTED("write to Interrupt Enable register");
        }

        LOG_UNIMPLEMENTED("write to unmapped address {:#04x} with data {:#02x}", addr, data);
    }

    void mmu::write16(u16 addr, u16 data)
    {
        write8(addr, static_cast<u8>(data));
        write8(addr + 1, static_cast<u8>(data >> 8));
    }

    u8 mmu::read8_io(u16 addr) const
    {
        switch (addr)
        {
        case video::reg::lcdc:
        case video::reg::stat:
        case video::reg::scy:
        case video::reg::scx:
        case video::reg::ly:
        case video::reg::lyc:
        case video::reg::bgp:
            // case video::reg::obp0:
            // case video::reg::obp1:
            // case video::reg::wx:
            // case video::reg::wy:
            return m_ppu.read8(addr);
        case 0xFF50: return m_bootrom_disabled;
        }
        LOG_UNIMPLEMENTED("read from unmapped I/O address {:#04x}", addr);
        return 0XFF;
    }

    void mmu::write8_io(u16 addr, u8 data)
    {
        switch (addr)
        {
        case video::reg::lcdc:
        case video::reg::stat:
        case video::reg::scy:
        case video::reg::scx:
        case video::reg::ly:
        case video::reg::lyc:
        case video::reg::bgp:
            // case video::reg::obp0:
            // case video::reg::obp1:
            // case video::reg::wx:
            // case video::reg::wy:
            m_ppu.write8(addr, data);
            break;
        case 0xFF50: m_bootrom_disabled = data; break;
        }
        LOG_UNIMPLEMENTED("write to unmapped I/O address {:#04x} with data {:#02x}", addr, data);
    }

} // namespace cb::memory
