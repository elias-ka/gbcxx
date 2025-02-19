#include "core/core.hpp"

#include "core/constants.hpp"

namespace gb
{
using namespace literals;

void Core::RunFrame()
{
    constexpr int kCyclesPerFrame = 70224;

    int this_frame_cycles{};
    while (this_frame_cycles < kCyclesPerFrame)
    {
        const int cycles_executed = cpu_.Step();
        ppu_.Step(cycles_executed);
        this_frame_cycles += cycles_executed;
    }
}

uint8_t Core::BusRead8(uint16_t addr) const
{
#ifdef GBCXX_TESTS
    return wram_[addr];
#endif

    if (kCartridgeStart <= addr && addr <= kCartridgeEnd)
        return mbc_->ReadRom(addr);

    if ((kVramStart <= addr && addr <= kVramEnd) ||
        (kOamStart <= addr && addr <= kOamEnd))
        return ppu_.Read(addr);

    if (kExternalRamStart <= addr && addr <= kExternalRamEnd)
        return mbc_->ReadRam(addr);

    if (kWorkRamStart <= addr && addr <= kWorkRamEnd)
        return wram_[addr - kWorkRamStart];

    if (kEchoRamStart <= addr && addr <= kEchoRamEnd)
        return wram_[addr - 0x2000];

    if (kNotUsableStart <= addr && addr <= kNotUsableEnd)
        return 0xff;

    if (kHighRamStart <= addr && addr <= kHighRamEnd)
        return hram_[addr & 0x7f];

    switch (addr)
    {
    case kRegJoyp: return joyp_ | 0xcf;  // todo

    case kRegSb: return sb_;  // todo

    case kRegSc: return sc_;  // todo

    case kRegDiv:
    case kRegTima:
    case kRegTma:
    case kRegTac: return timer_.Read(addr);

    case kRegIe:
    case kRegIf: return cpu_.Read(addr);

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
    case 0xff30:
    case 0xff31:
    case 0xff32:
    case 0xff33:
    case 0xff34:
    case 0xff35:
    case 0xff36:
    case 0xff37:
    case 0xff38:
    case 0xff39:
    case 0xff3a:
    case 0xff3b:
    case 0xff3c:
    case 0xff3d:
    case 0xff3e:
    case 0xff3f: return 0xff;

    case kRegLcdc:
    case kRegStat:
    case kRegLy:
    case kRegLyc:
    case kRegScy:
    case kRegScx:
    case kRegWy:
    case kRegWx:
    case kRegBgp:
    case kRegObp0:
    case kRegObp1: return ppu_.Read(addr);

    default: break;
    }

    DIE("Core: Unmapped read from {:X}", addr);
}

void Core::BusWrite8(uint16_t addr, uint8_t val)
{
#ifdef GBCXX_TESTS
    wram_[addr] = val;
    return;
#endif

    if (kCartridgeStart <= addr && addr <= kCartridgeEnd)
    {
        mbc_->WriteRom(addr, val);
    }
    else if ((kVramStart <= addr && addr <= kVramEnd) ||
             (kOamStart <= addr && addr <= kOamEnd))
    {
        ppu_.Write(addr, val);
    }
    else if (kExternalRamStart <= addr && addr <= kExternalRamEnd)
    {
        mbc_->WriteRam(addr, val);
    }
    else if (kWorkRamStart <= addr && addr <= kWorkRamEnd)
    {
        wram_[addr - kWorkRamStart] = val;
    }
    else if (kEchoRamStart <= addr && addr <= kEchoRamEnd)
    {
        wram_[addr - 0x2000] = val;
    }
    else if (kNotUsableStart <= addr && addr <= kNotUsableEnd)
    {
        return;
    }
    else if (kHighRamStart <= addr && addr <= kHighRamEnd)
    {
        hram_[addr & 0x7f] = val;
    }
    else
    {
        switch (addr)
        {
        case kRegJoyp:
            joyp_ = val;  // todo
            return;

        case kRegSb:
            sb_ = val;  // todo
            return;

        case kRegSc:
            sc_ = val;  // todo
            return;

        case kRegDiv:
        case kRegTima:
        case kRegTma:
        case kRegTac: timer_.Write(addr, val); return;

        case kRegIe:
        case kRegIf: cpu_.Write(addr, val); return;

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
        case 0xff30:
        case 0xff31:
        case 0xff32:
        case 0xff33:
        case 0xff34:
        case 0xff35:
        case 0xff36:
        case 0xff37:
        case 0xff38:
        case 0xff39:
        case 0xff3a:
        case 0xff3b:
        case 0xff3c:
        case 0xff3d:
        case 0xff3e:
        case 0xff3f: return;  // todo

        case 0xff46:  // todo oam dma
            return;

        case kRegLcdc:
        case kRegStat:
        case kRegLy:
        case kRegLyc:
        case kRegScy:
        case kRegScx:
        case kRegWy:
        case kRegWx:
        case kRegBgp:
        case kRegObp0:
        case kRegObp1: ppu_.Write(addr, val); return;

        default: break;
        }

        LOG_ERROR("Core: Unmapped write {:X} <- {:X}", addr, val);
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
