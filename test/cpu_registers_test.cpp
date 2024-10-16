#include "core/processor.h"
#include <gtest/gtest.h>

using namespace cb;

class CpuRegistersTest : public ::testing::Test
{
protected:
    Registers regs;
};

TEST_F(CpuRegistersTest, HandlesAFRegister)
{
    regs.set(Reg16::af, 0x1234);
    EXPECT_EQ(regs.get(Reg8::a), 0x12);
    EXPECT_EQ(regs.f().raw, 0x30);
    EXPECT_EQ(regs.get(Reg16::af), 0x1230);
}

TEST_F(CpuRegistersTest, HandlesCombinedRegisters)
{
    regs.set(Reg16::bc, 0x1234);
    EXPECT_EQ(regs.get(Reg8::b), 0x12);
    EXPECT_EQ(regs.get(Reg8::c), 0x34);
    EXPECT_EQ(regs.get(Reg16::bc), 0x1234);

    regs.set(Reg16::de, 0x1234);
    EXPECT_EQ(regs.get(Reg8::d), 0x12);
    EXPECT_EQ(regs.get(Reg8::e), 0x34);
    EXPECT_EQ(regs.get(Reg16::de), 0x1234);

    regs.set(Reg16::hl, 0x1234);
    EXPECT_EQ(regs.get(Reg8::h), 0x12);
    EXPECT_EQ(regs.get(Reg8::l), 0x34);
    EXPECT_EQ(regs.get(Reg16::hl), 0x1234);
}
