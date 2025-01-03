#include "core/constants.hpp"
#include "core/processor.hpp"

// to-do: a lot of duplication in this file can be reduced

namespace gbcxx {
// MARK: 8-bit loads
void Cpu::ld_r_r(Reg8 dst, Reg8 src) {
  write_reg(dst, read_reg(src));
}

void Cpu::ld_r_n(Reg8 dst) {
  write_reg(dst, read_operand());
}

void Cpu::ld_r_mem_hl(Reg8 dst) {
  write_reg(dst, read_mem(read_reg(Reg16::hl)));
}

void Cpu::ld_mem_hl_r(Reg8 src) {
  write_mem(read_reg(Reg16::hl), read_reg(src));
}

void Cpu::ld_mem_hl_n() {
  write_mem(read_reg(Reg16::hl), read_operand());
}

void Cpu::ld_a_mem_rr(Reg16 src) {
  m_reg.write(Reg8::a, read_mem(read_reg(src)));
}

void Cpu::ld_mem_rr_a(Reg16 dst) {
  write_mem(read_reg(dst), read_reg(Reg8::a));
}

void Cpu::ld_a_mem_nn() {
  m_reg.write(Reg8::a, read_mem(read_operands()));
}

void Cpu::ld_mem_nn_a() {
  write_mem(read_operands(), read_reg(Reg8::a));
}

void Cpu::ldh_a_mem_c() {
  write_reg(Reg8::a, read_mem(0xff00 + read_reg(Reg8::c)));
}

void Cpu::ldh_mem_c_a() {
  write_mem(0xff00 + read_reg(Reg8::c), read_reg(Reg8::a));
}

void Cpu::ldh_a_mem_n() {
  write_reg(Reg8::a, read_mem(0xff00 + read_operand()));
}

void Cpu::ldh_mem_n_a() {
  write_mem(0xff00 + read_operand(), read_reg(Reg8::a));
}

void Cpu::ld_a_mem_hl_dec() {
  const uint16_t hl = read_reg(Reg16::hl);
  write_reg(Reg8::a, read_mem(hl));
  write_reg(Reg16::hl, hl - 1);
}

void Cpu::ld_mem_hl_dec_a() {
  const uint16_t hl = read_reg(Reg16::hl);
  write_mem(hl, read_reg(Reg8::a));
  write_reg(Reg16::hl, hl - 1);
}

void Cpu::ld_a_mem_hl_inc() {
  const uint16_t hl = read_reg(Reg16::hl);
  write_reg(Reg8::a, read_mem(hl));
  write_reg(Reg16::hl, hl + 1);
}

void Cpu::ld_mem_hl_inc_a() {
  const uint16_t hl_val = read_reg(Reg16::hl);
  write_mem(hl_val, read_reg(Reg8::a));
  write_reg(Reg16::hl, hl_val + 1);
}

// MARK: 8-bit arithmetic and logical
void Cpu::add_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint16_t result = read_reg(Reg8::a) + r_val;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) + (r_val & 0xf)) > 0xf);
  set_flag(Flag::c, result > 0xff);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::add_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint16_t result = read_reg(Reg8::a) + data;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) + (data & 0xf)) > 0xf);
  set_flag(Flag::c, result > 0xff);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::add_n() {
  const uint8_t n = read_operand();
  const uint16_t result = read_reg(Reg8::a) + n;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) + (n & 0xf)) > 0xf);
  set_flag(Flag::c, result > 0xff);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::adc_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint16_t result = read_reg(Reg8::a) + r_val + flags().c();
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h,
           ((read_reg(Reg8::a) & 0xf) + (r_val & 0xf) + flags().c()) > 0xf);
  set_flag(Flag::c, result > 0xff);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::adc_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint16_t result = read_reg(Reg8::a) + data + flags().c();
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h,
           ((read_reg(Reg8::a) & 0xf) + (data & 0xf) + flags().c()) > 0xf);
  set_flag(Flag::c, result > 0xff);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::adc_n() {
  const uint8_t n = read_operand();
  const uint16_t result = read_reg(Reg8::a) + n + flags().c();
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h,
           ((read_reg(Reg8::a) & 0xf) + (n & 0xf) + flags().c()) > 0xf);
  set_flag(Flag::c, result > 0xff);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::sub_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint8_t result = read_reg(Reg8::a) - r_val;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, true);
  set_flag(Flag::h, (read_reg(Reg8::a) & 0xf) - (r_val & 0xf) < 0);
  set_flag(Flag::c, read_reg(Reg8::a) < r_val);
  write_reg(Reg8::a, result);
}

void Cpu::sub_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint16_t result = read_reg(Reg8::a) - data;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, true);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) - (data & 0xf)) < 0);
  set_flag(Flag::c, read_reg(Reg8::a) < data);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::sub_n() {
  const uint8_t n = read_operand();
  const uint16_t result = read_reg(Reg8::a) - n;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, true);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) - (n & 0xf)) < 0);
  set_flag(Flag::c, read_reg(Reg8::a) < n);
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::sbc_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint8_t a = read_reg(Reg8::a);
  const bool carry = flags().c();
  const int result = a - r_val - carry;
  set_flag(Flag::z, !uint8_t(result));
  set_flag(Flag::n, true);
  set_flag(Flag::h, (a & 0xf) < ((r_val & 0xf) + carry));
  set_flag(Flag::c, a < (r_val + carry));
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::sbc_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint8_t a = read_reg(Reg8::a);
  const bool carry = flags().c();
  const int result = a - data - carry;
  set_flag(Flag::z, !uint8_t(result));
  set_flag(Flag::n, true);
  set_flag(Flag::h, (a & 0xf) < ((data & 0xf) + carry));
  set_flag(Flag::c, a < (data + carry));
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::sbc_n() {
  const uint8_t n = read_operand();
  const uint16_t a = read_reg(Reg8::a);
  const bool carry = flags().c();
  const uint16_t result = a - n - carry;
  set_flag(Flag::z, !uint8_t(result));
  set_flag(Flag::n, true);
  set_flag(Flag::h, ((a & 0xf) < ((n & 0xf) + carry)));
  set_flag(Flag::c, a < (n + carry));
  write_reg(Reg8::a, uint8_t(result));
}

void Cpu::cp_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint8_t result = read_reg(Reg8::a) - r_val;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, true);
  set_flag(Flag::h, (read_reg(Reg8::a) & 0xf) - (r_val & 0xf) < 0);
  set_flag(Flag::c, read_reg(Reg8::a) < r_val);
}

void Cpu::cp_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint16_t result = read_reg(Reg8::a) - data;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, true);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) - (data & 0xf)) < 0);
  set_flag(Flag::c, read_reg(Reg8::a) < data);
}

void Cpu::cp_n() {
  const uint8_t n = read_operand();
  const uint16_t result = read_reg(Reg8::a) - n;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, true);
  set_flag(Flag::h, ((read_reg(Reg8::a) & 0xf) - (n & 0xf)) < 0);
  set_flag(Flag::c, read_reg(Reg8::a) < n);
}

void Cpu::inc_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint16_t result = r_val + 1;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h, (r_val & 0xf) == 0xf);
  write_reg(r, uint8_t(result));
}

void Cpu::inc_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const uint16_t result = data + 1;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, false);
  set_flag(Flag::h, (data & 0xf) == 0xf);
  write_mem(addr, uint8_t(result));
}

void Cpu::dec_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const uint8_t result = r_val - 1;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, true);
  set_flag(Flag::h, (result & 0xf) == 0xf);
  write_reg(r, uint8_t(result));
}

void Cpu::dec_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const uint16_t result = data - 1;
  set_flag(Flag::z, !(result & 0xff));
  set_flag(Flag::n, true);
  set_flag(Flag::h, (result & 0xf) == 0xf);
  write_mem(addr, uint8_t(result));
}

void Cpu::and_r(Reg8 r) {
  const uint8_t result = read_reg(Reg8::a) & read_reg(r);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, true);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::and_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint8_t result = read_reg(Reg8::a) & data;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, true);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::and_n() {
  const uint8_t n = read_operand();
  const uint8_t result = read_reg(Reg8::a) & n;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, true);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::or_r(Reg8 r) {
  const uint8_t result = read_reg(Reg8::a) | read_reg(r);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::or_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint8_t result = read_reg(Reg8::a) | data;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::or_n() {
  const uint8_t n = read_operand();
  const uint8_t result = read_reg(Reg8::a) | n;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::xor_r(Reg8 r) {
  const uint8_t result = read_reg(Reg8::a) ^ read_reg(r);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::xor_mem_hl() {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  const uint8_t result = read_reg(Reg8::a) ^ data;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::xor_n() {
  const uint8_t n = read_operand();
  const uint8_t result = read_reg(Reg8::a) ^ n;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(Reg8::a, result);
}

void Cpu::ccf() {
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, !flags().c());
}

void Cpu::scf() {
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, true);
}

void Cpu::daa() {
  // ref:
  // https://forums.nesdev.org/viewtopic.php?p=196282&sid=38a75719934a07d0ae8ac78a3e1448ad#p196282
  if (!flags().n()) {
    if (flags().c() || read_reg(Reg8::a) > 0x99) {
      write_reg(Reg8::a, read_reg(Reg8::a) + 0x60);
      set_flag(Flag::c, true);
    }
    if (flags().h() || (read_reg(Reg8::a) & 0x0f) > 0x09) {
      write_reg(Reg8::a, read_reg(Reg8::a) + 0x6);
    }
  } else {
    if (flags().c()) {
      write_reg(Reg8::a, read_reg(Reg8::a) - 0x60);
    }
    if (flags().h()) {
      write_reg(Reg8::a, read_reg(Reg8::a) - 0x6);
    }
  }

  set_flag(Flag::z, !read_reg(Reg8::a));
  set_flag(Flag::h, false);
}

void Cpu::cpl() {
  write_reg(Reg8::a, ~read_reg(Reg8::a));
  set_flag(Flag::n, true);
  set_flag(Flag::h, true);
}

// MARK: 16-bit loads
void Cpu::ld_rr_nn(Reg16 dst) {
  write_reg(dst, read_operands());
}

void Cpu::ld_mem_nn_sp() {
  write_mem16(read_operands(), read_reg(Reg16::sp));
}

void Cpu::ld_sp_hl() {
  write_reg(Reg16::sp, read_reg(Reg16::hl));
}

void Cpu::push_rr(Reg16 src) {
  push16(read_reg(src));
}

void Cpu::pop_rr(Reg16 dst) {
  write_reg(dst, pop());
}

void Cpu::ld_hl_sp_e() {
  const auto e = static_cast<int8_t>(read_operand());
  set_flag(Flag::z, false);
  set_flag(Flag::n, false);
  set_flag(Flag::h, ((read_reg(Reg16::sp) & 0xf) + (e & 0xf)) > 0xf);
  set_flag(Flag::c, ((read_reg(Reg16::sp) & 0xff) + (e & 0xff)) > 0xff);
  write_reg(Reg16::hl, read_reg(Reg16::sp) + uint16_t(e));
}

// MARK: 16-bit arithmetic
void Cpu::inc_rr(Reg16 rr) {
  write_reg(rr, read_reg(rr) + 1);
}

void Cpu::dec_rr(Reg16 rr) {
  write_reg(rr, read_reg(rr) - 1);
}

void Cpu::add_hl_rr(Reg16 rr) {
  const uint16_t rr_val = read_reg(rr);
  const uint16_t hl_val = read_reg(Reg16::hl);
  const int result = hl_val + rr_val;
  set_flag(Flag::n, false);
  set_flag(Flag::h, (rr_val & 0xffF) + (hl_val & 0xffF) > 0xffF);
  set_flag(Flag::c, result > 0xffFF);
  write_reg(Reg16::hl, uint16_t(result));
}

void Cpu::add_sp_e() {
  const auto e = static_cast<int8_t>(read_operand());
  set_flag(Flag::z, false);
  set_flag(Flag::n, false);
  set_flag(Flag::h, ((read_reg(Reg16::sp) & 0xf) + (e & 0xf)) > 0xf);
  set_flag(Flag::c, ((read_reg(Reg16::sp) & 0xff) + (e & 0xff)) > 0xff);
  write_reg(Reg16::sp, read_reg(Reg16::sp) + uint16_t(e));
}

// MARK: Control flow
void Cpu::jp_nn() {
  const uint16_t nn = read_operands();
  m_pc = nn;
}

void Cpu::jp_hl() {
  m_pc = read_reg(Reg16::hl);
}

void Cpu::jp_cc_nn(Condition cc) {
  const uint16_t nn = read_operands();
  if (check_condition(cc)) {
    m_pc = nn;
    tick4();
  }
}

void Cpu::jr_e() {
  const auto e = static_cast<int8_t>(read_operand());
  m_pc += e;
}

void Cpu::jr_cc_e(Condition cc) {
  const auto e = static_cast<int8_t>(read_operand());
  if (check_condition(cc)) {
    m_pc += e;
    tick4();
  }
}

void Cpu::call_nn() {
  const uint16_t nn = read_operands();
  push16(m_pc);
  m_pc = nn;
}

void Cpu::call_cc_nn(Condition cc) {
  const uint16_t nn = read_operands();
  if (check_condition(cc)) {
    push16(m_pc);
    m_pc = nn;
  }
}

void Cpu::ret() {
  m_pc = pop();
}

void Cpu::ret_cc(Condition cc) {
  if (check_condition(cc)) {
    m_pc = pop();
  }
}

void Cpu::reti() {
  m_pc = pop();
  m_ime = true;
}

void Cpu::rst_n(uint8_t vec) {
  push16(m_pc);
  m_pc = vec;
}

// MARK: Miscellaneous
void Cpu::halt() {
  if (m_ime) {
    m_mode = Mode::halt;
  } else {
    const uint8_t intf = m_mmu.read(reg_if);
    const uint8_t inte = m_mmu.read(reg_ie);
    if (intf & inte & 0x1F) {
      m_mode = Mode::halt_bug;
    } else {
      m_mode = Mode::halt_di;
    }
  }
}
void Cpu::stop() {
  m_mode = Mode::stop;
}
void Cpu::di() {
  m_ime = false;
}
void Cpu::ei() {
  m_mode = Mode::ime_pending;
}
void Cpu::nop() {}

// MARK: Rotate, shift, and bit operations
void Cpu::rlca() {
  const bool carry = read_reg(Reg8::a) & (1 << 7);
  const auto result = uint8_t((read_reg(Reg8::a) << 1) | carry);
  set_flag(Flag::z, false);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(Reg8::a, result);
}

void Cpu::rrca() {
  const bool carry = read_reg(Reg8::a) & 1;
  const auto result = uint8_t((read_reg(Reg8::a) >> 1) | (carry << 7));
  set_flag(Flag::z, false);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(Reg8::a, result);
}

void Cpu::rla() {
  const auto result = uint8_t((read_reg(Reg8::a) << 1) | flags().c());
  set_flag(Flag::z, false);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, read_reg(Reg8::a) & (1 << 7));
  write_reg(Reg8::a, result);
}

void Cpu::rra() {
  const auto result = uint8_t((read_reg(Reg8::a) >> 1) | (flags().c() << 7));
  set_flag(Flag::z, false);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, read_reg(Reg8::a) & 1);
  write_reg(Reg8::a, result);
}

void Cpu::rlc_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const bool carry = r_val & (1 << 7);
  const auto result = uint8_t((r_val << 1) | carry);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(r, result);
}

void Cpu::rlc_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const bool carry = data & (1 << 7);
  const auto result = uint8_t((data << 1) | carry);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_mem(addr, result);
}

void Cpu::rrc_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const bool carry = r_val & 1;
  const auto result = uint8_t((r_val >> 1) | (carry << 7));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(r, result);
}

void Cpu::rrc_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const bool carry = data & 1;
  const auto result = uint8_t((data >> 1) | (carry << 7));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_mem(addr, result);
}

void Cpu::rl_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const auto result = uint8_t((r_val << 1) | flags().c());
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, r_val & (1 << 7));
  write_reg(r, result);
}

void Cpu::rl_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const auto result = uint8_t((data << 1) | flags().c());
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, data & (1 << 7));
  write_mem(addr, result);
}

void Cpu::rr_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const auto result = uint8_t((r_val >> 1) | (flags().c() << 7));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, r_val & 1);
  write_reg(r, result);
}

void Cpu::rr_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const auto result = uint8_t((data >> 1) | (flags().c() << 7));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, data & 1);
  write_mem(addr, result);
}

void Cpu::sla_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const bool carry = r_val & (1 << 7);
  const auto result = uint8_t(r_val << 1);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(r, result);
}

void Cpu::sla_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const bool carry = data & (1 << 7);
  const auto result = uint8_t(data << 1);
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_mem(addr, result);
}

void Cpu::sra_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const bool carry = r_val & 1;
  const auto result = uint8_t((r_val >> 1) | (r_val & (1 << 7)));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(r, result);
}

void Cpu::sra_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const bool carry = data & 1;
  const auto result = uint8_t((data >> 1) | (data & (1 << 7)));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_mem(addr, result);
}

void Cpu::swap_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const auto result =
      uint8_t(((r_val & 0x0F) << 4) | (((r_val & 0xf0) >> 4) & 0xf));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_reg(r, result);
}

void Cpu::swap_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const auto result =
      uint8_t(((data & 0x0F) << 4) | (((data & 0xf0) >> 4) & 0xf));
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, false);
  write_mem(addr, result);
}

void Cpu::srl_r(Reg8 r) {
  const uint8_t r_val = read_reg(r);
  const bool carry = r_val & 1;
  const uint8_t result = r_val >> 1;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_reg(r, result);
}

void Cpu::srl_mem_hl() {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  const bool carry = data & 1;
  const uint8_t result = data >> 1;
  set_flag(Flag::z, !result);
  set_flag(Flag::n, false);
  set_flag(Flag::h, false);
  set_flag(Flag::c, carry);
  write_mem(addr, result);
}

void Cpu::bit_b_r(uint8_t b, Reg8 r) {
  set_flag(Flag::z, !(read_reg(r) & (1 << b)));
  set_flag(Flag::n, false);
  set_flag(Flag::h, true);
}

void Cpu::bit_b_mem_hl(uint8_t b) {
  const uint8_t data = read_mem(read_reg(Reg16::hl));
  set_flag(Flag::z, !(data & (1 << b)));
  set_flag(Flag::n, false);
  set_flag(Flag::h, true);
}

void Cpu::res_b_r(uint8_t b, Reg8 r) {
  write_reg(r, (read_reg(r) & ~(1 << b)));
}

void Cpu::res_b_mem_hl(uint8_t b) {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  write_mem(addr, data & ~(1 << b));
}

void Cpu::set_b_r(uint8_t b, Reg8 r) {
  write_reg(r, uint8_t(read_reg(r) | (1 << b)));
}

void Cpu::set_b_mem_hl(uint8_t b) {
  const uint16_t addr = read_reg(Reg16::hl);
  const uint8_t data = read_mem(addr);
  write_mem(addr, uint8_t(data | (1 << b)));
}

}  // namespace gbcxx
