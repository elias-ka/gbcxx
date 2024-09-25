#pragma once

#include "core/video.h"
#include "util.h"

namespace cb::memory
{
    using bios_t = std::array<u8, 0x100>;
    extern const bios_t default_bios;

    struct mbc_rom_only
    {
        std::array<u8, 32_KiB> rom;
        std::array<u8, 8_KiB> ram;
    };

    struct cartridge
    {
        std::variant<mbc_rom_only> mbc;

        u8 read(uint16_t addr) const;
        u8 read_external_ram(uint16_t addr) const;
        void write_external_ram(uint16_t addr, u8 value);
    };

    class mmu
    {
    public:
        mmu(bios_t bios = default_bios);

        u8 read8(u16 addr) const;
        u16 read16(u16 addr) const;

        void write8(u16 addr, u8 data);
        void write16(u16 addr, u16 data);

        void tick_components() { m_ppu.tick(); }

        video::ppu& ppu() { return m_ppu; }

    private:
        u8 read8_io(u16 addr) const;
        void write8_io(u16 addr, u8 data);

        bool in_bootrom() const;

        std::array<u8, 0x2000> m_wram{};
        std::array<u8, 256> m_bios{};
        std::array<u8, 0x80> m_hram{};
        memory::cartridge m_cartridge{};
        video::ppu m_ppu{};
        u8 m_wram_bank{};
        u8 m_bootrom_disabled{};
    };

} // namespace cb::memory
