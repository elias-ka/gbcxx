#include "core/core.hpp"

#include "core/constants.hpp"

namespace gb
{
using namespace literals;

// Extracted from SameBoy's DMG boot ROM:
// https://github.com/LIJI32/SameBoy/tree/master/BootROMs/dmg_boot.asm
static constexpr std::array<uint8_t, 0x100> kDmgBootrom = {
    0x31, 0xfe, 0xff, 0x21, 0x00, 0x80, 0xaf, 0x22, 0xcb, 0x6c, 0x28, 0xfb, 0x3e, 0x80, 0xe0, 0x26,
    0xe0, 0x11, 0x3e, 0xf3, 0xe0, 0x12, 0xe0, 0x25, 0x3e, 0x77, 0xe0, 0x24, 0x3e, 0x54, 0xe0, 0x47,
    0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0x47, 0xcd, 0xa3, 0x00, 0xcd, 0xa3, 0x00, 0x13, 0x7b,
    0xee, 0x34, 0x20, 0xf2, 0x11, 0xd2, 0x00, 0x0e, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x0d, 0x20, 0xf9,
    0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20,
    0xf9, 0x2e, 0x0f, 0x18, 0xf5, 0x3e, 0x1e, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x16, 0x89, 0x0e,
    0x0f, 0xcd, 0xb8, 0x00, 0x7a, 0xcb, 0x2f, 0xcb, 0x2f, 0xe0, 0x42, 0x7a, 0x81, 0x57, 0x79, 0xfe,
    0x08, 0x20, 0x04, 0x3e, 0xa8, 0xe0, 0x47, 0x0d, 0x20, 0xe7, 0x3e, 0xfc, 0xe0, 0x47, 0x3e, 0x83,
    0xcd, 0xcb, 0x00, 0x06, 0x05, 0xcd, 0xc4, 0x00, 0x3e, 0xc1, 0xcd, 0xcb, 0x00, 0x06, 0x3c, 0xcd,
    0xc4, 0x00, 0x21, 0xb0, 0x01, 0xe5, 0xf1, 0x21, 0x4d, 0x01, 0x01, 0x13, 0x00, 0x11, 0xd8, 0x00,
    0xc3, 0xfe, 0x00, 0x3e, 0x04, 0x0e, 0x00, 0xcb, 0x20, 0xf5, 0xcb, 0x11, 0xf1, 0xcb, 0x11, 0x3d,
    0x20, 0xf5, 0x79, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xe5, 0x21, 0x0f, 0xff, 0xcb, 0x86, 0xcb, 0x46,
    0x28, 0xfc, 0xe1, 0xc9, 0xcd, 0xb8, 0x00, 0x05, 0x20, 0xfa, 0xc9, 0xe0, 0x13, 0x3e, 0x87, 0xe0,
    0x14, 0xc9, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x50,
};

void Core::RunFrame()
{
    ppu_.SetFrameReady(false);

    while (!ppu_.IsFrameReady())
    {
        cpu_.Run();
    }
}

void Core::TickComponents()
{
    ppu_.Tick();
    timer_.Tick();
}

uint8_t Core::BusRead8(uint16_t addr) const
{
#ifdef GBCXX_TESTS
    return wram_.at(addr);
#endif

    if (kCartridgeStart <= addr && addr <= kCartridgeEnd)
    {
        if (bootrom_enabled_ && addr < kDmgBootrom.size())
        {
            return kDmgBootrom[addr];
        }
        return mbc_->ReadRom(addr);
    }
    if ((kVramStart <= addr && addr <= kVramEnd) || (kOamStart <= addr && addr <= kOamEnd) ||
        (kRegLcdc <= addr && addr <= kRegVbk))
    {
        return ppu_.Read(addr);
    }
    if (kExternalRamStart <= addr && addr <= kExternalRamEnd)
    {
        return mbc_->ReadRam(addr);
    }
    if (kWorkRamStart <= addr && addr <= kWorkRamEnd)
    {
        return wram_.at(addr - kWorkRamStart);
    }
    if (kEchoRamStart <= addr && addr <= kEchoRamEnd)
    {
        return wram_.at(addr - 0x1fff);
    }
    if (kNotUsableStart <= addr && addr <= kNotUsableEnd)
    {
        return 0x00;
    }
    if (kHighRamStart <= addr && addr <= kHighRamEnd)
    {
        return hram_.at(addr & 0x7f);
    }

    switch (addr)
    {
        case kRegIe:
        case kRegIf: return cpu_.Read(addr);
        case kRegBootrom: return bootrom_enabled_;
        case kRegJoyp:
        case kRegSb:
        case kRegSc: return 0xff;
        case kRegDiv:
        case kRegTima:
        case kRegTma:
        case kRegTac: return timer_.Read(addr);
        case kRegNr10:
        case kRegNr11:
        case kRegNr12:
        case kRegNr13:
        case kRegNr14:
        case kRegNr21:
        case kRegNr22:
        case kRegNr23:
        case kRegNr24:
        case kRegNr30:
        case kRegNr31:
        case kRegNr32:
        case kRegNr33:
        case kRegNr34:
        case kRegNr41:
        case kRegNr42:
        case kRegNr43:
        case kRegNr44:
        case kRegNr50:
        case kRegNr51:
        case kRegNr52:
        case kWavePatternStart:
        case kWavePatternEnd: return 0xff;
        default: break;
    }

    LOG_ERROR("MMU: Unmapped read {:X}", addr);
    return 0xff;
}

void Core::BusWrite8(uint16_t addr, uint8_t val)
{
#ifdef GBCXX_TESTS
    wram_.at(addr) = val;
    return;
#endif

    LOG_TRACE("core::write8({:X}, {:X})", addr, val);

    if (kCartridgeStart <= addr && addr <= kCartridgeEnd)
    {
        if (bootrom_enabled_ && addr < kDmgBootrom.size())
        {
            LOG_ERROR("MMU: Tried writing to boot ROM address {:X}", addr);
            return;
        }
        mbc_->WriteRom(addr, val);
    }
    else if ((kVramStart <= addr && addr <= kVramEnd) || (kOamStart <= addr && addr <= kOamEnd) ||
             (kRegLcdc <= addr && addr <= kRegVbk))
    {
        ppu_.Write(addr, val);
    }
    else if (kExternalRamStart <= addr && addr <= kExternalRamEnd)
    {
        mbc_->WriteRam(addr, val);
    }
    else if (kWorkRamStart <= addr && addr <= kWorkRamEnd)
    {
        wram_.at(addr - kWorkRamStart) = val;
    }
    else if (kEchoRamStart <= addr && addr <= kEchoRamEnd)
    {
        wram_.at(addr & 0x1fff) = val;
    }
    else if (kNotUsableStart <= addr && addr <= kNotUsableEnd)
    {
        return;
    }
    else if (kHighRamStart <= addr && addr <= kHighRamEnd)
    {
        hram_.at(addr & 0x7f) = val;
    }
    else
    {
        switch (addr)
        {
            case kRegBootrom: bootrom_enabled_ = val; return;
            case kRegIe:
            case kRegIf: cpu_.Write(addr, val); return;
            case kRegJoyp: return;
            case kRegSb:
            {
                serial_buffer_.push_back(val);
                if (val == '\n')
                {
                    LOG_DEBUG("Serial: {}",
                              std::string{serial_buffer_.begin(), serial_buffer_.end()});
                    serial_buffer_.clear();
                }
                break;
            }
            case kRegSc: return;
            case kRegDiv:
            case kRegTima:
            case kRegTma:
            case kRegTac: timer_.Write(addr, val); return;
            case kRegNr10:
            case kRegNr11:
            case kRegNr12:
            case kRegNr13:
            case kRegNr14:
            case kRegNr21:
            case kRegNr22:
            case kRegNr23:
            case kRegNr24:
            case kRegNr30:
            case kRegNr31:
            case kRegNr32:
            case kRegNr33:
            case kRegNr34:
            case kRegNr41:
            case kRegNr42:
            case kRegNr43:
            case kRegNr44:
            case kRegNr50:
            case kRegNr51:
            case kRegNr52:
            case kWavePatternStart:
            case kWavePatternEnd: return;
            default: LOG_ERROR("MMU: Unmapped write {:X} <- {:X}", addr, val);
        }
    }
}

uint16_t Core::BusRead16(uint16_t addr) const
{
    const uint8_t lo = BusRead8(addr);
    const uint8_t hi = BusRead8(addr + 1);
    return static_cast<uint16_t>(hi << 8) | lo;
}

void Core::BusWrite16(uint16_t addr, uint16_t val)
{
    BusWrite8(addr, val & 0xff);
    BusWrite8(addr + 1, val >> 8);
}

void Core::Irq(Interrupt interrupt)
{
    cpu_.Irq(interrupt);
}

void Core::LoadRom(const std::filesystem::path& path)
{
    std::vector<uint8_t> contents = fs::ReadFile(path);
    if (contents.size() > 8_MiB)
    {
        DIE("Core: Exceeded maximum ROM size of 8 MiB");
    }
    else if (!contents.size())
    {
        DIE("Core: Empty ROM file");
    }
    cart_title_ = {contents.begin() + 0x134, contents.begin() + 0x143};
    mbc_ = Mbc::MakeFromRom(std::move(contents));
}

}  // namespace gb
