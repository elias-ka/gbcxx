#include <gtest/gtest.h>

#include "core/sm83/cpu.hpp"

using namespace gb;
using enum sm83::R8;
using enum sm83::R16;

class CpuRegistersTest : public ::testing::Test
{
protected:
    sm83::Cpu cpu{{}};
};

TEST_F(CpuRegistersTest, HandlesAFRegister)
{
    cpu.SetReg(Af, 0x1234);

    EXPECT_EQ(cpu.GetReg(A), 0x12);
    EXPECT_EQ(cpu.GetReg(F), 0x30);
    EXPECT_EQ(cpu.GetReg(Af), 0x1230);
}

TEST_F(CpuRegistersTest, HandlesCombinedRegisters)
{
    cpu.SetReg(Bc, 0x1234);

    EXPECT_EQ(cpu.GetReg(B), 0x12);
    EXPECT_EQ(cpu.GetReg(C), 0x34);
    EXPECT_EQ(cpu.GetReg(Bc), 0x1234);

    cpu.SetReg(De, 0x1234);
    EXPECT_EQ(cpu.GetReg(D), 0x12);
    EXPECT_EQ(cpu.GetReg(E), 0x34);
    EXPECT_EQ(cpu.GetReg(De), 0x1234);

    cpu.SetReg(Hl, 0x1234);
    EXPECT_EQ(cpu.GetReg(H), 0x12);
    EXPECT_EQ(cpu.GetReg(L), 0x34);
    EXPECT_EQ(cpu.GetReg(Hl), 0x1234);
}
