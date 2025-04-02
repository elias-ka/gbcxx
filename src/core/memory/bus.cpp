#include "core/memory/bus.hpp"

#include "core/constants.hpp"

namespace gb::memory
{
void Bus::Tick(uint8_t tcycles)
{
    timer.Tick(tcycles);
    interrupt_flag |= timer.ConsumeInterrupts();

    ppu.Tick(tcycles);
    interrupt_flag |= ppu.ConsumeInterrupts();
}

uint8_t Bus::ReadByte(uint16_t addr) const
{
#ifdef GBCXX_TESTS
    return wram[addr];
#endif

    if ((addr >= kCartridgeStart && addr <= kCartridgeEnd) ||
        (addr >= kExternalRamStart && addr <= kExternalRamEnd))
    {
        return cartridge.ReadByte(addr);
    }
    if ((addr >= kVramStart && addr <= kVramEnd) || (addr >= kOamStart && addr <= kOamEnd) ||
        (addr >= kRegLcdc && addr <= kRegLyc) || (addr >= kRegBgp && addr <= kRegWx))
    {
        return ppu.ReadByte(addr);
    }
    if (addr >= kWorkRamStart && addr <= kWorkRamEnd) { return wram[addr - kWorkRamStart]; }
    if (addr >= kEchoRamStart && addr <= kEchoRamEnd) { return wram[addr - kEchoRamStart]; }
    if (addr >= kRegDiv && addr <= kRegTac) { return timer.ReadByte(addr); }
    if (addr >= kNotUsableStart && addr <= kNotUsableEnd) { return 0; }
    if (addr >= kHighRamStart && addr <= kHighRamEnd) { return hram[addr - kHighRamStart]; }
    if (addr == kRegJoyp) { return joypad.ReadButtons(); }
    if (addr == kRegIf) { return interrupt_flag; }
    if (addr == kRegIe) { return interrupt_enable; }
    if (addr == kRegBootrom) { return 0xff; }

    LOG_ERROR("Bus: Unmapped read {:X}", addr);
    return 0xff;
}

void Bus::WriteByte(uint16_t addr, uint8_t val)
{
#ifdef GBCXX_TESTS
    wram[addr] = val;
    return;
#endif

    if ((addr >= kCartridgeStart && addr <= kCartridgeEnd) ||
        (addr >= kExternalRamStart && addr <= kExternalRamEnd))
    {
        cartridge.WriteByte(addr, val);
    }
    else if ((addr >= kVramStart && addr <= kVramEnd) || (addr >= kOamStart && addr <= kOamEnd) ||
             (addr >= kRegLcdc && addr <= kRegLyc) || (addr >= kRegBgp && addr <= kRegWx))
    {
        ppu.WriteByte(addr, val);
    }
    else if (addr >= kWorkRamStart && addr <= kWorkRamEnd) { wram[addr - kWorkRamStart] = val; }
    else if (addr >= kEchoRamStart && addr <= kEchoRamEnd) { wram[addr - kEchoRamStart] = val; }
    else if (addr >= kRegDiv && addr <= kRegTac) { timer.WriteByte(addr, val); }
    else if (addr >= kNotUsableStart && addr <= kNotUsableEnd || addr == kRegBootrom) { return; }
    else if (addr == kRegJoyp) { joypad.Write(val); }
    else if (addr >= kHighRamStart && addr <= kHighRamEnd) { hram[addr - kHighRamStart] = val; }
    else if (addr == kRegOamDma)
    {
        // to-do: OAM blocking?
        const uint16_t source_address = val * 0x100;
        constexpr auto kOamSize = 0xa0;
        for (uint8_t i = 0; i < kOamSize; ++i)
        {
            WriteByte(kOamStart + i, ReadByte(source_address + i));
        }
    }
    else if (addr == kRegIf) { interrupt_flag = val; }
    else if (addr == kRegIe) { interrupt_enable = val; }
    else { LOG_ERROR("Bus: Unmapped write {:X} <- {:X}", addr, val); }
}

}  // namespace gb::memory
