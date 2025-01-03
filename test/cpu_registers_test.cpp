#include <gtest/gtest.h>

#include "core/processor.hpp"

using namespace gbcxx;

class Cpu_Registers_Test : public ::testing::Test {
 protected:
  Cpu::Registers reg;
};

TEST_F(Cpu_Registers_Test, HandlesAFRegister) {
  using enum Cpu::Reg16;
  using enum Cpu::Reg8;

  reg.write(af, 0x1234);
  EXPECT_EQ(reg.read(a), 0x12);
  EXPECT_EQ(reg.flags().raw, 0x30);
  EXPECT_EQ(reg.read(af), 0x1230);
}

TEST_F(Cpu_Registers_Test, HandlesCombinedRegisters) {
  using enum Cpu::Reg16;
  using enum Cpu::Reg8;

  reg.write(bc, 0x1234);
  EXPECT_EQ(reg.read(b), 0x12);
  EXPECT_EQ(reg.read(c), 0x34);
  EXPECT_EQ(reg.read(bc), 0x1234);

  reg.write(de, 0x1234);
  EXPECT_EQ(reg.read(d), 0x12);
  EXPECT_EQ(reg.read(e), 0x34);
  EXPECT_EQ(reg.read(de), 0x1234);

  reg.write(hl, 0x1234);
  EXPECT_EQ(reg.read(h), 0x12);
  EXPECT_EQ(reg.read(l), 0x34);
  EXPECT_EQ(reg.read(hl), 0x1234);
}
