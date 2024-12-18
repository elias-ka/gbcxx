#include <gtest/gtest.h>

#include "core/processor.hpp"

using namespace gbcxx;

class Cpu_Registers_Test : public ::testing::Test {
protected:
    Cpu::Registers reg;
};

TEST_F(Cpu_Registers_Test, HandlesAFRegister)
{
    using enum Cpu::Reg16;
    using enum Cpu::Reg8;

    reg.set(af, 0x1234);
    EXPECT_EQ(reg.get(a), 0x12);
    EXPECT_EQ(reg.flags().raw, 0x30);
    EXPECT_EQ(reg.get(af), 0x1230);
}

TEST_F(Cpu_Registers_Test, HandlesCombinedRegisters)
{
    using enum Cpu::Reg16;
    using enum Cpu::Reg8;

    reg.set(bc, 0x1234);
    EXPECT_EQ(reg.get(b), 0x12);
    EXPECT_EQ(reg.get(c), 0x34);
    EXPECT_EQ(reg.get(bc), 0x1234);

    reg.set(de, 0x1234);
    EXPECT_EQ(reg.get(d), 0x12);
    EXPECT_EQ(reg.get(e), 0x34);
    EXPECT_EQ(reg.get(de), 0x1234);

    reg.set(hl, 0x1234);
    EXPECT_EQ(reg.get(h), 0x12);
    EXPECT_EQ(reg.get(l), 0x34);
    EXPECT_EQ(reg.get(hl), 0x1234);
}
