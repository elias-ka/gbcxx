#pragma once

#include <utility>

#include "core/mbc.hpp"
#include "core/video.hpp"
#include "util.hpp"

namespace cb
{
    struct Cartridge
    {
        MbcVariant mbc;

        explicit Cartridge(std::vector<u8> cartrom);

        explicit Cartridge(MbcVariant mbc)
            : mbc(std::move(mbc))
        {
        }

        u8 read(u16 address) const;
        void write(u16 address, u8 value);
    };

    class Mmu
    {
    public:
        explicit Mmu(MbcVariant mbc);

        void tick();

        u8 read(u16 address) const;
        u16 read16(u16 address) const;

        void write(u16 address, u8 data);
        void write16(u16 address, u16 data);

        void resize_memory(usz new_size) { m_wram.resize(new_size); }

        Ppu& ppu() { return m_ppu; }

    private:
        bool in_bootrom() const { return m_reg_bootrom == 0; }

    private:
        Ppu m_ppu{};

        std::array<u8, 0x100> m_bootrom{};
        u8 m_reg_bootrom{1};
        std::vector<u8> m_wram;
        std::vector<u8> m_hram;
        Cartridge m_cartridge;
        u8 m_ie{};
        u8 m_if{};
    };

} // namespace cb
