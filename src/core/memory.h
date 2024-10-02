#pragma once

#include "core/video.h"
#include "util.h"

namespace cb
{
    using bootrom = std::array<u8, 0x100>;
    extern const bootrom default_bootrom;

    struct mbc_rom_only
    {
        explicit mbc_rom_only(std::vector<u8> cartrom)
            : rom(std::move(cartrom))
            , ram(8_KiB)
        {
        }

        std::vector<u8> rom;
        std::vector<u8> ram;
    };

    struct mbc1
    {
        std::vector<u8> rom;
        std::vector<u8> ram;
        u16 rom_bank{1};
        u16 ram_bank{0};
        bool ram_enabled{};
        bool banking_mode{};
        bool supports_advanced_banking;

        explicit mbc1(std::vector<u8> cartrom)
            : rom(std::move(cartrom))
            , ram(32_KiB)
            , supports_advanced_banking(rom.size() > 512_KiB)
        {
        }
    };

    using memory_bank_controller = std::variant<std::monostate, mbc_rom_only, mbc1>;

    struct cartridge
    {
        memory_bank_controller mbc;

        explicit cartridge(std::vector<u8> cartrom);

        explicit cartridge(memory_bank_controller mbc)
            : mbc(std::move(mbc))
        {
        }

        u8 read(uint16_t addr) const;
        void write(uint16_t addr, u8 value);
    };

    class mmu
    {
    public:
        mmu(cartridge cartridge, bootrom bootrom = default_bootrom);

        u8 read8(u16 addr) const;
        u16 read16(u16 addr) const;

        void write8(u16 addr, u8 data);
        void write16(u16 addr, u16 data);

        void tick_components() { m_ppu.tick(); }

        ppu& ppu() { return m_ppu; }

        void resize_memory(usz new_size) { m_memory.resize(new_size); }

    private:
        u8 read8_io(u16 addr) const;
        void write8_io(u16 addr, u8 data);

        bool in_bootrom() const;
        std::array<u8, 256> m_bootrom{};
        cartridge m_cartridge;
        std::vector<u8> m_memory{};
        class ppu m_ppu = {};
        u8 m_bootrom_disabled{};
    };

} // namespace cb
