#include "core/processor.h"
#include <gtest/gtest.h>

namespace cpu = cb::cpu;

class CpuRegistersTest : public ::testing::Test
{
protected:
    cpu::registers regs;
};

TEST_F(CpuRegistersTest, HandlesAFRegister)
{
    regs.write16(cpu::reg16::af, 0x1234);
    EXPECT_EQ(regs.a, 0x12);
    EXPECT_EQ(regs.f.raw, 0x30);
    EXPECT_EQ(regs.read16(cpu::reg16::af), 0x1230);
}

TEST_F(CpuRegistersTest, HandlesCombinedRegisters)
{
    regs.write16(cpu::reg16::bc, 0x1234);
    EXPECT_EQ(regs.b, 0x12);
    EXPECT_EQ(regs.c, 0x34);
    EXPECT_EQ(regs.read16(cpu::reg16::bc), 0x1234);

    regs.write16(cpu::reg16::de, 0x1234);
    EXPECT_EQ(regs.d, 0x12);
    EXPECT_EQ(regs.e, 0x34);
    EXPECT_EQ(regs.read16(cpu::reg16::de), 0x1234);

    regs.write16(cpu::reg16::hl, 0x1234);
    EXPECT_EQ(regs.h, 0x12);
    EXPECT_EQ(regs.l, 0x34);
    EXPECT_EQ(regs.read16(cpu::reg16::hl), 0x1234);
}
