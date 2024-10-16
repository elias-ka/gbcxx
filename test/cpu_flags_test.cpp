#include "core/processor.hpp"
#include <gtest/gtest.h>

class CpuFlagsTest : public ::testing::Test
{
protected:
    cb::Flags f;
};

TEST_F(CpuFlagsTest, Zero)
{
    f.raw = 0b1000'0000;
    EXPECT_TRUE(f.z());
    EXPECT_FALSE(f.n());
    EXPECT_FALSE(f.h());
    EXPECT_FALSE(f.c());

    f.raw = 0b0111'1111;
    EXPECT_FALSE(f.z());
}

TEST_F(CpuFlagsTest, Subtraction)
{
    f.raw = 0b0100'0000;
    EXPECT_TRUE(f.n());
    EXPECT_FALSE(f.z());
    EXPECT_FALSE(f.h());
    EXPECT_FALSE(f.c());

    f.raw = 0b1011'1111;
    EXPECT_FALSE(f.n());
}

TEST_F(CpuFlagsTest, HalfCarry)
{
    f.raw = 0b0010'0000;
    EXPECT_TRUE(f.h());
    EXPECT_FALSE(f.z());
    EXPECT_FALSE(f.n());
    EXPECT_FALSE(f.c());

    f.raw = 0b1101'1111;
    EXPECT_FALSE(f.h());
}

TEST_F(CpuFlagsTest, Carry)
{
    f.raw = 0b0001'0000;
    EXPECT_TRUE(f.c());
    EXPECT_FALSE(f.z());
    EXPECT_FALSE(f.n());
    EXPECT_FALSE(f.h());

    f.raw = 0b1110'1111;
    EXPECT_FALSE(f.c());
}
