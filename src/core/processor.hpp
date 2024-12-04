#pragma once

#include <fmt/base.h>

#include <array>
#include <string>

#include "core/mbc.hpp"
#include "core/memory.hpp"

namespace gbcxx {
enum class Flag { z, n, h, c };

struct Flags {
  uint8_t raw;

  bool operator==(const Flags&) const = default;

  bool z() const;
  bool n() const;
  bool h() const;
  bool c() const;

  void set(Flag f, bool set = true);
};

inline std::string format_as(Flags f) {
  const std::array<char, 4> symbols = {f.z() ? 'Z' : '-', f.n() ? 'N' : '-',
                                       f.h() ? 'H' : '-', f.c() ? 'C' : '-'};
  return {symbols.begin(), symbols.end()};
}

enum class Reg8 { A, B, C, D, E, H, L };
enum class Reg16 { AF, BC, DE, HL, SP };
enum class Condition { NZ, Z, NC, C };

class Registers {
 public:
  Registers() = default;

#ifdef GBCXX_TESTING
  explicit Registers(uint16_t sp, Flags f, uint8_t a, uint8_t b, uint8_t c,
                     uint8_t d, uint8_t e, uint8_t h, uint8_t l)
      : m_sp(sp),
        m_f(f),
        m_a(a),
        m_b(b),
        m_c(c),
        m_d(d),
        m_e(e),
        m_h(h),
        m_l(l) {}
#endif

  Flags flags() const { return m_f; }
  Flags& flags() { return m_f; }

  uint8_t get(Reg8 reg) const;
  uint16_t get(Reg16 reg) const;

  void set(Reg8 reg, uint8_t val);
  void set(Reg16 reg, uint16_t val);

 private:
  uint16_t m_sp{0xFFFE};
  Flags m_f{0xB0};
  uint8_t m_a{0x01};
  uint8_t m_b{0x00};
  uint8_t m_c{0x13};
  uint8_t m_d{0x00};
  uint8_t m_e{0xD8};
  uint8_t m_h{0x01};
  uint8_t m_l{0x4D};
};

class Cpu {
 public:
  explicit Cpu(std::unique_ptr<Mbc> mbc) : m_mmu(std::move(mbc)) {}

#ifdef GBCXX_TESTING
  Cpu(std::unique_ptr<Mbc> mbc, uint16_t pc, uint16_t sp, Flags f, uint8_t a,
      uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t h, uint8_t l)
      : m_mmu(std::move(mbc)), m_reg(sp, f, a, b, c, d, e, h, l), m_pc(pc) {}
#endif

  void step();
  void subtract_cycles(size_t cycles) { m_cycles_elapsed -= cycles; }
  size_t cycles_elapsed() const { return m_cycles_elapsed; }

  uint16_t pc() const { return m_pc; }
  uint16_t sp() const { return m_reg.get(Reg16::SP); }

  Flags flags() const { return m_reg.flags(); }
  void set_flag(Flag f, bool set = true) { m_reg.flags().set(f, set); }

  uint8_t reg(Reg8 reg) const { return m_reg.get(reg); }
  uint16_t reg(Reg16 reg) const { return m_reg.get(reg); }
  void set_reg(Reg8 reg, uint8_t val) { m_reg.set(reg, val); }
  void set_reg(Reg16 reg, uint16_t val) { m_reg.set(reg, val); }

  Mmu& mmu() { return m_mmu; }

 private:
  void log_for_gameboy_doctor();

  void tick();
  void tick4();

  uint8_t cycle_read(uint16_t addr);
  uint16_t cycle_read16(uint16_t addr);
  void cycle_write(uint16_t addr, uint8_t data);
  void cycle_write16(uint16_t addr, uint16_t data);

  uint8_t read_operand();
  uint16_t read_operands();

  void push(uint8_t value);
  void push16(uint16_t value);
  uint16_t pop();

  void handle_interrupt();
  void interpret_instruction(uint8_t opcode);

  bool check_condition(Condition cond) const {
    const auto flags = m_reg.flags();
    switch (cond) {
      case Condition::NZ: return !flags.z();
      case Condition::Z: return flags.z();
      case Condition::NC: return !flags.c();
      case Condition::C: return flags.c();
    }
  }

 private:
  enum class Mode { Default, Halt, Stop, HaltBug, HaltDI, IMEPending };

  Mmu m_mmu;
  Registers m_reg;
  size_t m_cycles_elapsed{};
  uint16_t m_pc{0x100};
  Mode m_mode{Mode::Default};
  bool m_ime{true};
  std::fstream m_log_file{"log.txt", std::ios::out};

 private:
  // 8-bit loads
  void ld_r_r(Reg8 dst, Reg8 src);
  void ld_r_n(Reg8 dst);
  void ld_r_mem_hl(Reg8 dst);
  void ld_mem_hl_r(Reg8 src);
  void ld_mem_hl_n();
  void ld_a_mem_rr(Reg16 src);
  void ld_mem_rr_a(Reg16 dst);
  void ld_a_mem_nn();
  void ld_mem_nn_a();
  void ldh_a_mem_c();
  void ldh_mem_c_a();
  void ldh_a_mem_n();
  void ldh_mem_n_a();
  void ld_a_mem_hl_dec();
  void ld_mem_hl_dec_a();
  void ld_a_mem_hl_inc();
  void ld_mem_hl_inc_a();

  // 8-bit arithmetic and logical
  void add_r(Reg8 r);
  void add_mem_hl();
  void add_n();
  void adc_r(Reg8 r);
  void adc_mem_hl();
  void adc_n();
  void sub_r(Reg8 r);
  void sub_mem_hl();
  void sub_n();
  void sbc_r(Reg8 r);
  void sbc_mem_hl();
  void sbc_n();
  void cp_r(Reg8 r);
  void cp_mem_hl();
  void cp_n();
  void inc_r(Reg8 r);
  void inc_mem_hl();
  void dec_r(Reg8 r);
  void dec_mem_hl();
  void and_r(Reg8 r);
  void and_mem_hl();
  void and_n();
  void or_r(Reg8 r);
  void or_mem_hl();
  void or_n();
  void xor_r(Reg8 r);
  void xor_mem_hl();
  void xor_n();
  void ccf();
  void scf();
  void daa();
  void cpl();

  // 16-bit loads
  void ld_rr_nn(Reg16 dst);
  void ld_mem_nn_sp();
  void ld_sp_hl();
  void push_rr(Reg16 src);
  void pop_rr(Reg16 dst);
  void ld_hl_sp_e();

  // 16-bit arithmetic
  void inc_rr(Reg16 rr);
  void dec_rr(Reg16 rr);
  void add_hl_rr(Reg16 rr);
  void add_sp_e();

  // Control flow
  void jp_nn();
  void jp_hl();
  void jp_cc_nn(Condition cc);
  void jr_e();
  void jr_cc_e(Condition cc);
  void call_nn();
  void call_cc_nn(Condition cc);
  void ret();
  void ret_cc(Condition cc);
  void reti();
  void rst_n(uint8_t vec);

  // Miscellaneous
  void halt();
  void stop();
  void di();
  void ei();
  void nop();

  // Rotate, shift, and bit operations
  void rlca();
  void rrca();
  void rla();
  void rra();
  void rlc_r(Reg8 r);
  void rlc_mem_hl();
  void rrc_r(Reg8 r);
  void rrc_mem_hl();
  void rl_r(Reg8 r);
  void rl_mem_hl();
  void rr_r(Reg8 r);
  void rr_mem_hl();
  void sla_r(Reg8 r);
  void sla_mem_hl();
  void sra_r(Reg8 r);
  void sra_mem_hl();
  void swap_r(Reg8 r);
  void swap_mem_hl();
  void srl_r(Reg8 r);
  void srl_mem_hl();
  void bit_b_r(uint8_t b, Reg8 r);
  void bit_b_mem_hl(uint8_t b);
  void res_b_r(uint8_t b, Reg8 r);
  void res_b_mem_hl(uint8_t b);
  void set_b_r(uint8_t b, Reg8 r);
  void set_b_mem_hl(uint8_t b);
};
}  // namespace gbcxx
