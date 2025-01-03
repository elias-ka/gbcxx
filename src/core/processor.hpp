#pragma once

#include <fmt/base.h>

#include <array>
#include <string>

#include "core/mbc.hpp"
#include "core/memory.hpp"
#include "util.hpp"

namespace gbcxx {

class Cpu {
 public:
  enum class Reg8 : uint8_t { a, b, c, d, e, h, l };
  enum class Reg16 : uint8_t { af, bc, de, hl, sp };
  enum class Condition : uint8_t { nz, z, nc, c };
  enum class Flag : uint8_t { z, n, h, c };

  struct Flags {
    uint8_t raw;

    bool operator==(const Flags&) const = default;

    [[nodiscard]] bool z() const { return raw & (1 << 7); }
    [[nodiscard]] bool n() const { return raw & (1 << 6); }
    [[nodiscard]] bool h() const { return raw & (1 << 5); }
    [[nodiscard]] bool c() const { return raw & (1 << 4); }

    void set(Flag f, bool set = true) {
      const auto mask = static_cast<uint8_t>(1 << (7 - std::to_underlying(f)));
      raw = set ? (raw | mask) : (raw & ~mask);
    }
  };

  class Registers {
   public:
    static Registers make_dmg() {
      Registers r;
      r.m_a = 0x01;
      r.m_b = 0x00;
      r.m_c = 0x13;
      r.m_d = 0x00;
      r.m_e = 0xd8;
      r.m_h = 0x01;
      r.m_l = 0x4d;
      r.m_sp = 0xfffe;
      r.m_f = Flags{0xb0};
      return r;
    }

    [[nodiscard]] auto&& flags(this auto&& self) { return FWD(self).m_f; }

    [[nodiscard]] uint8_t read(Reg8 reg) const;
    [[nodiscard]] uint16_t read(Reg16 reg) const;

    void write(Reg8 reg, uint8_t val);
    void write(Reg16 reg, uint16_t val);

   private:
    uint8_t m_a{};
    uint8_t m_b{};
    uint8_t m_c{};
    uint8_t m_d{};
    uint8_t m_e{};
    uint8_t m_h{};
    uint8_t m_l{};
    uint16_t m_sp{};
    Flags m_f{};
  };

  explicit Cpu(std::unique_ptr<Mbc> mbc) : m_mmu(std::move(mbc)) {}

  void step();

  void subtract_cycles(size_t cycles) { m_cycles -= cycles; }

  [[nodiscard]] size_t cycles_elapsed() const { return m_cycles; }

  [[nodiscard]] uint16_t pc() const { return m_pc; }

  void set_pc(uint16_t pc) { m_pc = pc; }

  [[nodiscard]] uint16_t sp() const { return m_reg.read(Reg16::sp); }

  [[nodiscard]] Flags flags() const { return m_reg.flags(); }
  void set_flag(Flag f, bool set = true) { m_reg.flags().set(f, set); }

  [[nodiscard]] uint8_t read_reg(Reg8 reg) const { return m_reg.read(reg); }

  [[nodiscard]] uint16_t read_reg(Reg16 reg) const { return m_reg.read(reg); }

  void write_reg(Reg8 reg, uint8_t val) { m_reg.write(reg, val); }

  void write_reg(Reg16 reg, uint16_t val) { m_reg.write(reg, val); }

  Mmu& mmu() { return m_mmu; }

 private:
  void log_for_gameboy_doctor();

  void tick();
  void tick4();

  uint8_t read_mem(uint16_t addr);
  uint16_t read_mem16(uint16_t addr);
  void write_mem(uint16_t addr, uint8_t data);
  void write_mem16(uint16_t addr, uint16_t data);

  uint8_t read_operand();
  uint16_t read_operands();

  void push(uint8_t value);
  void push16(uint16_t value);
  uint16_t pop();

  void handle_interrupt();
  void interpret_instruction(uint8_t opcode);

  [[nodiscard]] bool check_condition(Condition cond) const {
    const Flags flags = m_reg.flags();
    switch (cond) {
      case Condition::nz: return !flags.z();
      case Condition::z: return flags.z();
      case Condition::nc: return !flags.c();
      case Condition::c: return flags.c();
    }
  }

 private:
  enum class Mode : uint8_t {
    default_,
    halt,
    stop,
    halt_bug,
    halt_di,
    ime_pending
  };

  Mmu m_mmu;
  Registers m_reg{Registers::make_dmg()};
  size_t m_cycles{};
  uint16_t m_pc{0x0100};
  Mode m_mode{Mode::default_};
  bool m_ime{true};
  std::fstream m_log_file{"gameboy_doctor.log", std::ios::out};

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

inline std::string format_as(Cpu::Flags f) {
  const std::array<char, 4> symbols = {f.z() ? 'Z' : '-', f.n() ? 'N' : '-',
                                       f.h() ? 'H' : '-', f.c() ? 'C' : '-'};
  return {symbols.begin(), symbols.end()};
}

}  // namespace gbcxx
