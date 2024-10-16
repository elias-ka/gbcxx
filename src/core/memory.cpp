#include "core/memory.hpp"

#include "core/constants.hpp"
#include "util.hpp"
#include <cassert>
#include <utility>

namespace cb
{
    // Extracted from SameBoy's DMG boot ROM:
    // https://github.com/LIJI32/SameBoy/tree/master/BootROMs/dmg_boot.asm
    static const std::array<u8, 0x100> DMG_BOOTROM = {
        0x31, 0xFE, 0xFF, 0x21, 0x00, 0x80, 0xAF, 0x22, 0xCB, 0x6C, 0x28, 0xFB, 0x3E, 0x80, 0xE0,
        0x26, 0xE0, 0x11, 0x3E, 0xF3, 0xE0, 0x12, 0xE0, 0x25, 0x3E, 0x77, 0xE0, 0x24, 0x3E, 0x54,
        0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0x47, 0xCD, 0xA3, 0x00, 0xCD, 0xA3,
        0x00, 0x13, 0x7B, 0xEE, 0x34, 0x20, 0xF2, 0x11, 0xD2, 0x00, 0x0E, 0x08, 0x1A, 0x13, 0x22,
        0x23, 0x0D, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D,
        0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18, 0xF5, 0x3E, 0x1E, 0xE0, 0x42, 0x3E,
        0x91, 0xE0, 0x40, 0x16, 0x89, 0x0E, 0x0F, 0xCD, 0xB8, 0x00, 0x7A, 0xCB, 0x2F, 0xCB, 0x2F,
        0xE0, 0x42, 0x7A, 0x81, 0x57, 0x79, 0xFE, 0x08, 0x20, 0x04, 0x3E, 0xA8, 0xE0, 0x47, 0x0D,
        0x20, 0xE7, 0x3E, 0xFC, 0xE0, 0x47, 0x3E, 0x83, 0xCD, 0xCB, 0x00, 0x06, 0x05, 0xCD, 0xC4,
        0x00, 0x3E, 0xC1, 0xCD, 0xCB, 0x00, 0x06, 0x3C, 0xCD, 0xC4, 0x00, 0x21, 0xB0, 0x01, 0xE5,
        0xF1, 0x21, 0x4D, 0x01, 0x01, 0x13, 0x00, 0x11, 0xD8, 0x00, 0xC3, 0xFE, 0x00, 0x3E, 0x04,
        0x0E, 0x00, 0xCB, 0x20, 0xF5, 0xCB, 0x11, 0xF1, 0xCB, 0x11, 0x3D, 0x20, 0xF5, 0x79, 0x22,
        0x23, 0x22, 0x23, 0xC9, 0xE5, 0x21, 0x0F, 0xFF, 0xCB, 0x86, 0xCB, 0x46, 0x28, 0xFC, 0xE1,
        0xC9, 0xCD, 0xB8, 0x00, 0x05, 0x20, 0xFA, 0xC9, 0xE0, 0x13, 0x3E, 0x87, 0xE0, 0x14, 0xC9,
        0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
        0x50,
    };

    namespace
    {
        constexpr u16 RAM_ENABLE_START = 0x0000;
        constexpr u16 RAM_ENABLE_END = 0x1FFF;
        constexpr u16 SECONDARY_BANK_REGISTER_START = 0x4000;
        constexpr u16 SECONDARY_BANK_REGISTER_END = 0x5FFF;
        constexpr u16 ROM_BANK_START = 0x2000;
        constexpr u16 ROM_BANK_END = 0x3FFF;
        constexpr u16 ROM_SLOT_0_START = 0x0000;
        constexpr u16 ROM_SLOT_0_END = 0x3FFF;
        constexpr u16 ROM_SLOT_1_START = 0x4000;
        constexpr u16 ROM_SLOT_1_END = 0x7FFF;
        constexpr u16 BANKING_MODE_START = 0x6000;
        constexpr u16 BANKING_MODE_END = 0x7FFF;
    } // namespace

    u8 Cartridge::read(u16 address) const
    {
        return cb::variant_match(
            mbc, [&](const MbcRomOnly& rom_only) { return rom_only.rom.at(address); },
            [&](const Mbc1& mbc1)
            {
                if (ROM_SLOT_0_START <= address && address <= ROM_SLOT_0_END)
                {
                    return mbc1.rom.at(address);
                }
                if (ROM_SLOT_1_START <= address && address <= ROM_SLOT_1_END)
                {
                    return mbc1.rom.at((address & ROM_SLOT_1_START) +
                                       (mbc1.rom_bank * ROM_SLOT_1_END));
                }
                if (EXTERNAL_RAM_START <= address && address <= EXTERNAL_RAM_END)
                {
                    if (mbc1.ram_enabled)
                    {
                        const usz base_addr = address - EXTERNAL_RAM_START;
                        const usz ram_addr = base_addr + (static_cast<usz>(mbc1.ram_bank * 0x2000));
                        return mbc1.ram.at(ram_addr);
                    }
                    else
                    {
                        return u8(0xFF);
                    }
                }
                LOG_ERROR("MBC1: Out of bounds read from address {:#04x}", address);
                return u8(0xFF);
            },
            [&](std::monostate)
            {
                LOG_ERROR("Uninitialized mbc");
                return u8(0);
            });
    }

    void Cartridge::write(u16 address, u8 value)
    {
        cb::variant_match(
            mbc, [&](const MbcRomOnly&) { /* no-op */ },
            [&](Mbc1& mbc1)
            {
                if (RAM_ENABLE_START <= address && address <= RAM_ENABLE_END)
                {
                    mbc1.ram_enabled = (value & 0x0F) == 0x0A;
                }
                else if (ROM_BANK_START <= address && address <= ROM_BANK_END)
                {
                    mbc1.rom_bank = static_cast<u16>(value & 0b0001'1111);
                    if (mbc1.rom_bank == 0)
                    {
                        mbc1.rom_bank = 1;
                    }
                }
                else if (SECONDARY_BANK_REGISTER_START <= address &&
                         address <= SECONDARY_BANK_REGISTER_END)
                {
                    if (mbc1.banking_mode && mbc1.supports_advanced_banking)
                    {
                        mbc1.rom_bank =
                            static_cast<u16>(((static_cast<u8>(mbc1.rom_bank) & 0b0001'1111) |
                                              ((value & 0b11) << 5)));
                    }
                    else
                    {
                        mbc1.ram_bank = value & 0b11;
                    }
                }
                else if (BANKING_MODE_START <= address && address <= BANKING_MODE_END)
                {
                    mbc1.banking_mode = (value & 1) == 1;
                }
                else if (EXTERNAL_RAM_START <= address && address <= EXTERNAL_RAM_END)
                {
                    const usz base_addr = address - EXTERNAL_RAM_START;
                    const usz ram_addr = base_addr + (static_cast<usz>(mbc1.ram_bank * 0x2000));
                    mbc1.ram.at(ram_addr) = value;
                }
            },
            [&](std::monostate) { DIE("Uninitialized MBC"); });
    }

    Mmu::Mmu(MbcVariant mbc)
        : m_bootrom(DMG_BOOTROM)
        , m_wram(8_KiB)
        , m_hram(0x7F)
        , m_cartridge(std::move(mbc))
    {
    }

    void Mmu::tick() { m_ppu.tick(); }

    u8 Mmu::read(u16 address) const
    {
        LOG_TRACE("mmu::read({:#04x})", address);
#ifdef TESTING
        return m_wram.at(address);
#endif

        if (address == REG_BOOTROM)
            return m_reg_bootrom;

        if (CARTRIDGE_START <= address && address <= CARTRIDGE_END)
        {
            if (in_bootrom() && address <= 0xFF)
                return m_bootrom.at(address);

            return m_cartridge.read(address);
        }

        if ((VRAM_START <= address && address <= VRAM_END) ||
            (OAM_START <= address && address <= OAM_END))
        {
            return m_ppu.read(address);
        }

        if (EXTERNAL_RAM_START <= address && address <= EXTERNAL_RAM_END)
            return m_cartridge.read(address);

        if (WORK_RAM_START <= address && address <= WORK_RAM_END)
            return m_wram.at(address - WORK_RAM_START);

        if (ECHO_RAM_START <= address && address <= ECHO_RAM_END)
            return m_wram.at(address - ECHO_RAM_START);

        if (NOT_USABLE_START <= address && address <= NOT_USABLE_END)
            return 0;

        if (HIGH_RAM_START <= address && address <= HIGH_RAM_END)
            return m_hram.at(address - HIGH_RAM_START);

        if (address == REG_IE)
            return m_ie;

        if (IO_START <= address && address <= IO_END)
        {
            switch (address)
            {
            case REG_JOYP:
            {
                return 0;
            }
            case REG_SB:
            case REG_SC:
            {
                return 0;
            }
            case REG_DIV:
            {
                return 0;
            }
            case REG_TIMA:
            case REG_TMA:
            case REG_TAC:
            {
                return 0;
            }
            case REG_IF: return m_if;
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
            case WAVE_PATTERN_END:
            {
                return 0;
            }
            case REG_LCDC:
            case REG_STAT:
            case REG_SCY:
            case REG_SCX:
            case REG_LY:
            case REG_LYC:
            case REG_BGP:
            case REG_OBP0:
            case REG_OBP1:
            case REG_WX:
            case REG_WY: return m_ppu.read(address);
            default:
            {
                return 0;
            }
            }
        }
        return 0;
    }

    u16 Mmu::read16(u16 address) const
    {
        const auto lo = static_cast<u16>(read(address));
        const auto hi = static_cast<u16>(read(address + 1));
        return static_cast<u16>(hi << 8) | lo;
    }

    void Mmu::write(u16 address, u8 data)
    {
        LOG_TRACE("mmu::write({:#06x}, {:#04x})", address, data);
#ifdef TESTING
        m_wram.at(address) = data;
        return;
#endif

        if (address == REG_BOOTROM)
        {
            m_reg_bootrom = data;
            return;
        }

        if (CARTRIDGE_START <= address && address <= CARTRIDGE_END)
        {
            if (in_bootrom() && address <= 0xFF)
            {
                LOG_ERROR("Attempted to write into bootrom");
                return;
            }
            m_cartridge.write(address, data);
            return;
        }

        if ((VRAM_START <= address && address <= VRAM_END) ||
            (OAM_START <= address && address <= OAM_END))
        {
            m_ppu.write(address, data);
            return;
        }

        if (EXTERNAL_RAM_START <= address && address <= EXTERNAL_RAM_END)
        {
            m_cartridge.write(address, data);
            return;
        }

        if (WORK_RAM_START <= address && address <= WORK_RAM_END)
        {
            m_wram.at(address - WORK_RAM_START) = data;
            return;
        }

        if (ECHO_RAM_START <= address && address <= ECHO_RAM_END)
        {
            m_wram.at(address - 0x2000) = data;
            return;
        }

        if (NOT_USABLE_START <= address && address <= NOT_USABLE_END)
        {
            return;
        }

        if (HIGH_RAM_START <= address && address <= HIGH_RAM_END)
        {
            m_hram.at(address - HIGH_RAM_START) = data;
            return;
        }

        if (address == REG_IE)
        {
            m_ie = data;
            return;
        }

        if (IO_START <= address && address <= IO_END)
        {
            switch (address)
            {
            case REG_JOYP:
            {
                return;
            }
            case REG_SB:
            case REG_SC:
            {
                return;
            }
            case REG_DIV:
            {
                return;
            }
            case REG_TIMA:
            case REG_TMA:
            case REG_TAC:
            {
                return;
            }
            case REG_IF: m_if = data; return;
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
            case WAVE_PATTERN_END:
            {
                return;
            }
            case REG_LCDC:
            case REG_STAT:
            case REG_SCY:
            case REG_SCX:
            case REG_LY:
            case REG_LYC:
            case REG_BGP:
            case REG_OBP0:
            case REG_OBP1:
            case REG_WX:
            case REG_WY:
            {
                m_ppu.write(address, data);
                return;
            }
            }
        }
    }

    void Mmu::write16(u16 addr, u16 data)
    {
        write(addr, static_cast<u8>(data));
        write(addr + 1, static_cast<u8>(data >> 8));
    }

} // namespace cb
