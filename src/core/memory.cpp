#include "core/memory.h"

#include "core/constants.h"
#include "util.h"
#include <cassert>
#include <utility>

namespace cb
{
    const std::array<u8, 256> default_bootrom = {
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

    namespace
    {
        mbc_variant make_mbc(std::vector<u8> cartrom)
        {
            switch (cartrom[0x0147])
            {
            case 0x00:
            {
                LOG_INFO("Using rom only MBC");
                return mbc_rom_only{cartrom};
            }
            case 0x01:
            {
                LOG_INFO("Using MBC1");
                return mbc1{cartrom};
            }
            }
            LOG_UNIMPLEMENTED("cartridge type {:#02x}", cartrom[0x0147]);
            return std::monostate{};
        }
    } // namespace

    cartridge::cartridge(std::vector<u8> cartrom)
        : mbc(make_mbc(std::move(cartrom)))
    {
    }

    u8 cartridge::read(uint16_t addr) const
    {
        return cb::variant_match(
            mbc, [&](const mbc_rom_only& rom_only) { return rom_only.rom.at(addr); },
            [&](const mbc1& mbc1)
            {
                if (interval(rom_slot_0_start, rom_slot_0_end).contains(addr))
                {
                    return mbc1.rom.at(addr);
                }
                if (interval(rom_slot_1_start, rom_slot_1_end).contains(addr))
                {
                    return mbc1.rom.at((addr & rom_slot_1_start) +
                                       (mbc1.rom_bank * rom_slot_1_start));
                }
                if (interval(external_ram_start, external_ram_end).contains(addr))
                {
                    if (mbc1.ram_enabled)
                    {
                        const usz base_addr = addr - external_ram_start;
                        const usz ram_addr = base_addr + (static_cast<usz>(mbc1.ram_bank * 0x2000));
                        return mbc1.ram.at(ram_addr);
                    }
                    else
                    {
                        return u8(0xFF);
                    }
                }
                LOG_ERROR("MBC1: Out of bounds read from address {:#04x}", addr);
                return u8(0xFF);
            },
            [&](std::monostate)
            {
                LOG_ERROR("Uninitialized mbc");
                return u8(0);
            });
    }

    void cartridge::write(uint16_t addr, u8 value)
    {
        cb::variant_match(
            mbc, [&](const mbc_rom_only&) { /* no-op */ },
            [&](mbc1& mbc1)
            {
                if (interval(ram_enable_start, ram_enable_end).contains(addr))
                {
                    mbc1.ram_enabled = (value & 0x0F) == 0x0A;
                }
                else if (interval(rom_bank_start, rom_bank_end).contains(addr))
                {
                    mbc1.rom_bank = static_cast<u16>(value & 0b0001'1111);
                    if (mbc1.rom_bank == 0)
                    {
                        mbc1.rom_bank = 1;
                    }
                }
                else if (interval(secondary_bank_register_start, secondary_bank_register_end)
                             .contains(addr))
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
                else if (interval(banking_mode_start, banking_mode_end).contains(addr))
                {
                    mbc1.banking_mode = (value & 1) == 1;
                }
                else if (interval(external_ram_start, external_ram_end).contains(addr))
                {
                    const usz base_addr = addr - external_ram_start;
                    const usz ram_addr = base_addr + (static_cast<usz>(mbc1.ram_bank * 0x2000));
                    mbc1.ram.at(ram_addr) = value;
                }
            },
            [&](std::monostate) { LOG_ERROR("Uninitialized mbc"); });
    }

    mmu::mmu(cartridge cartridge, bootrom bios)
        : m_bootrom(bios)
        , m_memory(0x10000)
        , m_cartridge(std::move(cartridge))
    {
    }

    u8 mmu::read8(u16 addr) const
    {
        LOG_TRACE("mmu::read8({:#04x})", addr);
#ifdef TESTING
        return m_memory.at(addr);
#endif

        if (interval(0x0000, 0x7FFF).contains(addr)) // Cartridge
        {
            if (in_bootrom() && addr <= 0x00FF)
                return m_bootrom.at(addr);

            return m_cartridge.read(addr);
        }

        if (interval(0xA000, 0xBFFF).contains(addr)) // External RAM
            return m_cartridge.read(addr);

        if (interval(0xE000, 0xFDFF).contains(addr)) // Echo RAM
            return read8(addr - 0x2000);

        if (interval(0xFE00, 0xFE9F).contains(addr)) // OAM
        {
            // LOG_UNIMPLEMENTED("read from OAM");
            return 0xFF;
        }

        if (interval(0xFEA0, 0xFEFF).contains(addr)) // Not usable
            return 0xFF;

        return m_memory.at(addr);
    }

    u16 mmu::read16(u16 addr) const
    {
        const auto lo = static_cast<u16>(read8(addr));
        const auto hi = static_cast<u16>(read8(addr + 1));
        return static_cast<u16>(hi << 8) | lo;
    }

    void mmu::write8(u16 addr, u8 data)
    {
        LOG_TRACE("mmu::write8({:#06x}, {:#04x})", addr, data);
#ifdef TESTING
        m_memory.at(addr) = data;
        return;
#endif

        if (interval(0x0000, 0x7FFF).contains(addr)) // Cartridge
        {
            if (in_bootrom() && addr <= 0x00FF)
            {
                LOG_ERROR("Tried to write to boot ROM");
            }
            else
            {
                m_cartridge.write(addr, data);
            }
        }
        else if (interval(0xA000, 0xBFFF).contains(addr)) // External RAM
        {
            m_cartridge.write(addr, data);
        }
        else if (interval(0xE000, 0xFDFF).contains(addr)) // Echo RAM
        {
            write8(addr - 0x2000, data);
        }
        else if (interval(0xFE00, 0xFE9F).contains(addr)) // OAM
        {
            // LOG_UNIMPLEMENTED("write to OAM");
            return;
        }
        else if (interval(0xFEA0, 0xFEFF).contains(addr)) // Not usable
        {
            return;
        }
        else
        {
            m_memory.at(addr) = data;
        }
    }

    void mmu::write16(u16 addr, u16 data)
    {
        write8(addr, static_cast<u8>(data));
        write8(addr + 1, static_cast<u8>(data >> 8));
    }

    bool mmu::in_bootrom() const { return read8(reg_bootrom) != 0; }
} // namespace cb
