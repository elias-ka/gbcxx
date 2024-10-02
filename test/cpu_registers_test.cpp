#include "core/processor.h"
#include <gtest/gtest.h>

using namespace cb;

class CpuRegistersTest : public ::testing::Test
{
protected:
    mmu mmu{cartridge{mbc_rom_only{{}}}};
    processor cpu{&mmu};
};

TEST_F(CpuRegistersTest, HandlesAFRegister)
{
    cpu.set_reg(reg16::af, 0x1234);
    EXPECT_EQ(cpu.reg(reg8::a), 0x12);
    EXPECT_EQ(cpu.f().raw, 0x30);
    EXPECT_EQ(cpu.reg(reg16::af), 0x1230);
}

TEST_F(CpuRegistersTest, HandlesCombinedRegisters)
{
    cpu.set_reg(reg16::bc, 0x1234);
    EXPECT_EQ(cpu.reg(reg8::b), 0x12);
    EXPECT_EQ(cpu.reg(reg8::c), 0x34);
    EXPECT_EQ(cpu.reg(reg16::bc), 0x1234);

    cpu.set_reg(reg16::de, 0x1234);
    EXPECT_EQ(cpu.reg(reg8::d), 0x12);
    EXPECT_EQ(cpu.reg(reg8::e), 0x34);
    EXPECT_EQ(cpu.reg(reg16::de), 0x1234);

    cpu.set_reg(reg16::hl, 0x1234);
    EXPECT_EQ(cpu.reg(reg8::h), 0x12);
    EXPECT_EQ(cpu.reg(reg8::l), 0x34);
    EXPECT_EQ(cpu.reg(reg16::hl), 0x1234);
}
