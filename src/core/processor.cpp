#include "core/processor.hpp"

#include <fmt/base.h>

#include "core/interrupt.hpp"
#include "core/memory.hpp"
#include "util.hpp"

namespace gbcxx {
namespace {
constexpr u8 Z_MASK = 1 << 7;
constexpr u8 N_MASK = 1 << 6;
constexpr u8 H_MASK = 1 << 5;
constexpr u8 C_MASK = 1 << 4;
}  // namespace

bool Flags::z() const { return raw & Z_MASK; }
bool Flags::n() const { return raw & N_MASK; }
bool Flags::h() const { return raw & H_MASK; }
bool Flags::c() const { return raw & C_MASK; }

void Flags::set(Flag f, bool set) {
  switch (f) {
    case Flag::z: set ? (raw |= Z_MASK) : (raw &= ~Z_MASK); break;
    case Flag::n: set ? (raw |= N_MASK) : (raw &= ~N_MASK); break;
    case Flag::h: set ? (raw |= H_MASK) : (raw &= ~H_MASK); break;
    case Flag::c: set ? (raw |= C_MASK) : (raw &= ~C_MASK); break;
  }
}

u8 Registers::get(Reg8 r) const {
  switch (r) {
    case Reg8::a: return m_a;
    case Reg8::b: return m_b;
    case Reg8::c: return m_c;
    case Reg8::d: return m_d;
    case Reg8::e: return m_e;
    case Reg8::h: return m_h;
    case Reg8::l: return m_l;
  }
}

u16 Registers::get(Reg16 r) const {
  switch (r) {
    case Reg16::af: return static_cast<u16>(m_a << 8) | m_f.raw;
    case Reg16::bc: return static_cast<u16>(m_b << 8) | m_c;
    case Reg16::de: return static_cast<u16>(m_d << 8) | m_e;
    case Reg16::hl: return static_cast<u16>(m_h << 8) | m_l;
    case Reg16::sp: return m_sp;
  }
}

void Registers::set(Reg8 reg, u8 val) {
  switch (reg) {
    case Reg8::a: m_a = val; break;
    case Reg8::b: m_b = val; break;
    case Reg8::c: m_c = val; break;
    case Reg8::d: m_d = val; break;
    case Reg8::e: m_e = val; break;
    case Reg8::h: m_h = val; break;
    case Reg8::l: m_l = val; break;
  }
}

void Registers::set(Reg16 reg, u16 val) {
  switch (reg) {
    case Reg16::af:
      m_a = static_cast<u8>(val >> 8);
      m_f.raw = static_cast<u8>(val & 0x00F0);
      break;
    case Reg16::bc:
      m_b = static_cast<u8>(val >> 8);
      m_c = static_cast<u8>(val & 0x00FF);
      break;
    case Reg16::de:
      m_d = static_cast<u8>(val >> 8);
      m_e = static_cast<u8>(val & 0x00FF);
      break;
    case Reg16::hl:
      m_h = static_cast<u8>(val >> 8);
      m_l = static_cast<u8>(val & 0x00FF);
      break;
    case Reg16::sp: m_sp = val; break;
  }
}

void Cpu::step() {
  handle_interrupts();

  if (m_halted) {
    tick();
  } else {
    m_log_file << fmt::format(
        "A:{:02X} F:{:02X} B:{:02X} C:{:02X} D:{:02X} E:{:02X} "
        "H:{:02X} L:{:02X} SP:{:04X} PC:{:04X} "
        "PCMEM:{:02X},{:02X},{:02X},{:02X}\n",
        reg(Reg8::a), m_reg.flags().raw, reg(Reg8::b), reg(Reg8::c),
        reg(Reg8::d), reg(Reg8::e), reg(Reg8::h), reg(Reg8::l), reg(Reg16::sp),
        m_pc, m_mmu.read(m_pc), m_mmu.read(m_pc + 1), m_mmu.read(m_pc + 2),
        m_mmu.read(m_pc + 3));
    const auto instr = read_operand();
    execute(instr);
  }
}

u8 Cpu::cycle_read(u16 addr) {
  tick4();
  return m_mmu.read(addr);
}

u16 Cpu::cycle_read16(u16 addr) {
  const auto lo = cycle_read(addr);
  const auto hi = cycle_read(addr + 1);
  return static_cast<u16>((hi << 8) | lo);
}

void Cpu::cycle_write(u16 addr, u8 data) {
  tick4();
  m_mmu.write(addr, data);
}

void Cpu::cycle_write16(u16 addr, u16 data) {
  cycle_write(addr, static_cast<u8>(data));
  cycle_write(addr + 1, static_cast<u8>(data >> 8));
}

u8 Cpu::read_operand() {
  const auto operand = cycle_read(m_pc++);
  LOG_TRACE("Read operand {:#04x} from {:#06x}", operand, m_pc);
  return operand;
}

u16 Cpu::read_operands() {
  const auto lo = read_operand();
  const auto hi = read_operand();
  return static_cast<u16>((hi << 8) | lo);
}

void Cpu::push(u16 value) {
  set_reg(Reg16::sp, reg(Reg16::sp) - 1);
  cycle_write(reg(Reg16::sp), value >> 8);
  set_reg(Reg16::sp, reg(Reg16::sp) - 1);
  cycle_write(reg(Reg16::sp), static_cast<u8>(value));
}

u16 Cpu::pop() {
  const auto lo = cycle_read(reg(Reg16::sp));
  set_reg(Reg16::sp, reg(Reg16::sp) + 1);
  const auto hi = cycle_read(reg(Reg16::sp));
  set_reg(Reg16::sp, reg(Reg16::sp) + 1);
  return static_cast<u16>((hi << 8) | lo);
}

void Cpu::tick() {
  m_mmu.tick();
  m_cycles_elapsed += 1;
}

void Cpu::tick4() {
  tick();
  tick();
  tick();
  tick();
}

void Cpu::handle_interrupts() {
  if (m_ime_pending) {
    LOG_TRACE("IME was pending, enabling it.");
    m_ime = true;
    m_ime_pending = false;
  }

  if (!m_ime && !m_halted) {
    return;
  }

  const u8 intf = m_mmu.read(REG_IF);
  const u8 inte = m_mmu.read(REG_IE);
  const u8 enabled_interrupts = intf & inte;

  if (!enabled_interrupts) {
    return;
  }

  LOG_TRACE("Enabled interrupts: {:#04x}", enabled_interrupts);
  m_ime = false;
  m_halted = false;

  const auto n = std::countr_zero(enabled_interrupts);
  if (n > 4) {
    LOG_ERROR("Invalid interrupt number: {}", n);
    return;
  }

  const auto priority_int = static_cast<Interrupt>(1 << n);
  m_mmu.write(REG_IF, intf & ~to_underlying(priority_int));

  push(m_pc);

  const u16 handler = get_interrupt_handler_address(priority_int);
  LOG_TRACE("Jumping to interrupt handler {:#06x}", handler);
  m_pc = handler;
}

void Cpu::execute(u8 opcode) {
  // LOG_TRACE("processor::execute({:#04x})", opcode);
  // https://gekkio.fi/files/gb-docs/gbctr.pdf
  switch (opcode) {
    // 8-bit loads
    case 0x7F: ld_r_r(Reg8::a, Reg8::a); break;
    case 0x78: ld_r_r(Reg8::a, Reg8::b); break;
    case 0x79: ld_r_r(Reg8::a, Reg8::c); break;
    case 0x7A: ld_r_r(Reg8::a, Reg8::d); break;
    case 0x7B: ld_r_r(Reg8::a, Reg8::e); break;
    case 0x7C: ld_r_r(Reg8::a, Reg8::h); break;
    case 0x7D: ld_r_r(Reg8::a, Reg8::l); break;
    case 0x47: ld_r_r(Reg8::b, Reg8::a); break;
    case 0x40: ld_r_r(Reg8::b, Reg8::b); break;
    case 0x41: ld_r_r(Reg8::b, Reg8::c); break;
    case 0x42: ld_r_r(Reg8::b, Reg8::d); break;
    case 0x43: ld_r_r(Reg8::b, Reg8::e); break;
    case 0x44: ld_r_r(Reg8::b, Reg8::h); break;
    case 0x45: ld_r_r(Reg8::b, Reg8::l); break;
    case 0x4F: ld_r_r(Reg8::c, Reg8::a); break;
    case 0x48: ld_r_r(Reg8::c, Reg8::b); break;
    case 0x49: ld_r_r(Reg8::c, Reg8::c); break;
    case 0x4A: ld_r_r(Reg8::c, Reg8::d); break;
    case 0x4B: ld_r_r(Reg8::c, Reg8::e); break;
    case 0x4C: ld_r_r(Reg8::c, Reg8::h); break;
    case 0x4D: ld_r_r(Reg8::c, Reg8::l); break;
    case 0x57: ld_r_r(Reg8::d, Reg8::a); break;
    case 0x50: ld_r_r(Reg8::d, Reg8::b); break;
    case 0x51: ld_r_r(Reg8::d, Reg8::c); break;
    case 0x52: ld_r_r(Reg8::d, Reg8::d); break;
    case 0x53: ld_r_r(Reg8::d, Reg8::e); break;
    case 0x54: ld_r_r(Reg8::d, Reg8::h); break;
    case 0x55: ld_r_r(Reg8::d, Reg8::l); break;
    case 0x5F: ld_r_r(Reg8::e, Reg8::a); break;
    case 0x58: ld_r_r(Reg8::e, Reg8::b); break;
    case 0x59: ld_r_r(Reg8::e, Reg8::c); break;
    case 0x5A: ld_r_r(Reg8::e, Reg8::d); break;
    case 0x5B: ld_r_r(Reg8::e, Reg8::e); break;
    case 0x5C: ld_r_r(Reg8::e, Reg8::h); break;
    case 0x5D: ld_r_r(Reg8::e, Reg8::l); break;
    case 0x67: ld_r_r(Reg8::h, Reg8::a); break;
    case 0x60: ld_r_r(Reg8::h, Reg8::b); break;
    case 0x61: ld_r_r(Reg8::h, Reg8::c); break;
    case 0x62: ld_r_r(Reg8::h, Reg8::d); break;
    case 0x63: ld_r_r(Reg8::h, Reg8::e); break;
    case 0x64: ld_r_r(Reg8::h, Reg8::h); break;
    case 0x65: ld_r_r(Reg8::h, Reg8::l); break;
    case 0x6F: ld_r_r(Reg8::l, Reg8::a); break;
    case 0x68: ld_r_r(Reg8::l, Reg8::b); break;
    case 0x69: ld_r_r(Reg8::l, Reg8::c); break;
    case 0x6A: ld_r_r(Reg8::l, Reg8::d); break;
    case 0x6B: ld_r_r(Reg8::l, Reg8::e); break;
    case 0x6C: ld_r_r(Reg8::l, Reg8::h); break;
    case 0x6D: ld_r_r(Reg8::l, Reg8::l); break;
    case 0x3E: ld_r_n(Reg8::a); break;
    case 0x06: ld_r_n(Reg8::b); break;
    case 0x0E: ld_r_n(Reg8::c); break;
    case 0x16: ld_r_n(Reg8::d); break;
    case 0x1E: ld_r_n(Reg8::e); break;
    case 0x26: ld_r_n(Reg8::h); break;
    case 0x2E: ld_r_n(Reg8::l); break;
    case 0x7E: ld_r_mem_hl(Reg8::a); break;
    case 0x46: ld_r_mem_hl(Reg8::b); break;
    case 0x4E: ld_r_mem_hl(Reg8::c); break;
    case 0x56: ld_r_mem_hl(Reg8::d); break;
    case 0x5E: ld_r_mem_hl(Reg8::e); break;
    case 0x66: ld_r_mem_hl(Reg8::h); break;
    case 0x6E: ld_r_mem_hl(Reg8::l); break;
    case 0x77: ld_mem_hl_r(Reg8::a); break;
    case 0x70: ld_mem_hl_r(Reg8::b); break;
    case 0x71: ld_mem_hl_r(Reg8::c); break;
    case 0x72: ld_mem_hl_r(Reg8::d); break;
    case 0x73: ld_mem_hl_r(Reg8::e); break;
    case 0x74: ld_mem_hl_r(Reg8::h); break;
    case 0x75: ld_mem_hl_r(Reg8::l); break;
    case 0x36: ld_mem_hl_n(); break;
    case 0x0A: ld_a_mem_rr(Reg16::bc); break;
    case 0x1A: ld_a_mem_rr(Reg16::de); break;
    case 0x02: ld_mem_rr_a(Reg16::bc); break;
    case 0x12: ld_mem_rr_a(Reg16::de); break;
    case 0xFA: ld_a_mem_nn(); break;
    case 0xEA: ld_mem_nn_a(); break;
    case 0xF2: ldh_a_mem_c(); break;
    case 0xE2: ldh_mem_c_a(); break;
    case 0xF0: ldh_a_mem_n(); break;
    case 0xE0: ldh_mem_n_a(); break;
    case 0x3A: ld_a_mem_hl_dec(); break;
    case 0x32: ld_mem_hl_dec_a(); break;
    case 0x2A: ld_a_mem_hl_inc(); break;
    case 0x22: ld_mem_hl_inc_a(); break;

    // 8-bit arithmetic and logical
    case 0x87: add_r(Reg8::a); break;
    case 0x80: add_r(Reg8::b); break;
    case 0x81: add_r(Reg8::c); break;
    case 0x82: add_r(Reg8::d); break;
    case 0x83: add_r(Reg8::e); break;
    case 0x84: add_r(Reg8::h); break;
    case 0x85: add_r(Reg8::l); break;
    case 0x86: add_mem_hl(); break;
    case 0xC6: add_n(); break;
    case 0x8F: adc_r(Reg8::a); break;
    case 0x88: adc_r(Reg8::b); break;
    case 0x89: adc_r(Reg8::c); break;
    case 0x8A: adc_r(Reg8::d); break;
    case 0x8B: adc_r(Reg8::e); break;
    case 0x8C: adc_r(Reg8::h); break;
    case 0x8D: adc_r(Reg8::l); break;
    case 0x8E: adc_mem_hl(); break;
    case 0xCE: adc_n(); break;
    case 0x97: sub_r(Reg8::a); break;
    case 0x90: sub_r(Reg8::b); break;
    case 0x91: sub_r(Reg8::c); break;
    case 0x92: sub_r(Reg8::d); break;
    case 0x93: sub_r(Reg8::e); break;
    case 0x94: sub_r(Reg8::h); break;
    case 0x95: sub_r(Reg8::l); break;
    case 0x96: sub_mem_hl(); break;
    case 0xD6: sub_n(); break;
    case 0x9F: sbc_r(Reg8::a); break;
    case 0x98: sbc_r(Reg8::b); break;
    case 0x99: sbc_r(Reg8::c); break;
    case 0x9A: sbc_r(Reg8::d); break;
    case 0x9B: sbc_r(Reg8::e); break;
    case 0x9C: sbc_r(Reg8::h); break;
    case 0x9D: sbc_r(Reg8::l); break;
    case 0x9E: sbc_mem_hl(); break;
    case 0xDE: sbc_n(); break;
    case 0xBF: cp_r(Reg8::a); break;
    case 0xB8: cp_r(Reg8::b); break;
    case 0xB9: cp_r(Reg8::c); break;
    case 0xBA: cp_r(Reg8::d); break;
    case 0xBB: cp_r(Reg8::e); break;
    case 0xBC: cp_r(Reg8::h); break;
    case 0xBD: cp_r(Reg8::l); break;
    case 0xBE: cp_mem_hl(); break;
    case 0xFE: cp_n(); break;
    case 0x3C: inc_r(Reg8::a); break;
    case 0x04: inc_r(Reg8::b); break;
    case 0x0C: inc_r(Reg8::c); break;
    case 0x14: inc_r(Reg8::d); break;
    case 0x1C: inc_r(Reg8::e); break;
    case 0x24: inc_r(Reg8::h); break;
    case 0x2C: inc_r(Reg8::l); break;
    case 0x34: inc_mem_hl(); break;
    case 0x3D: dec_r(Reg8::a); break;
    case 0x05: dec_r(Reg8::b); break;
    case 0x0D: dec_r(Reg8::c); break;
    case 0x15: dec_r(Reg8::d); break;
    case 0x1D: dec_r(Reg8::e); break;
    case 0x25: dec_r(Reg8::h); break;
    case 0x2D: dec_r(Reg8::l); break;
    case 0x35: dec_mem_hl(); break;
    case 0xA7: and_r(Reg8::a); break;
    case 0xA0: and_r(Reg8::b); break;
    case 0xA1: and_r(Reg8::c); break;
    case 0xA2: and_r(Reg8::d); break;
    case 0xA3: and_r(Reg8::e); break;
    case 0xA4: and_r(Reg8::h); break;
    case 0xA5: and_r(Reg8::l); break;
    case 0xA6: and_mem_hl(); break;
    case 0xE6: and_n(); break;
    case 0xB7: or_r(Reg8::a); break;
    case 0xB0: or_r(Reg8::b); break;
    case 0xB1: or_r(Reg8::c); break;
    case 0xB2: or_r(Reg8::d); break;
    case 0xB3: or_r(Reg8::e); break;
    case 0xB4: or_r(Reg8::h); break;
    case 0xB5: or_r(Reg8::l); break;
    case 0xB6: or_mem_hl(); break;
    case 0xF6: or_n(); break;
    case 0xAF: xor_r(Reg8::a); break;
    case 0xA8: xor_r(Reg8::b); break;
    case 0xA9: xor_r(Reg8::c); break;
    case 0xAA: xor_r(Reg8::d); break;
    case 0xAB: xor_r(Reg8::e); break;
    case 0xAC: xor_r(Reg8::h); break;
    case 0xAD: xor_r(Reg8::l); break;
    case 0xAE: xor_mem_hl(); break;
    case 0xEE: xor_n(); break;
    case 0x3F: ccf(); break;
    case 0x37: scf(); break;
    case 0x27: daa(); break;
    case 0x2F: cpl(); break;

    // 16-bit loads
    case 0x01: ld_rr_nn(Reg16::bc); break;
    case 0x11: ld_rr_nn(Reg16::de); break;
    case 0x21: ld_rr_nn(Reg16::hl); break;
    case 0x31: ld_rr_nn(Reg16::sp); break;
    case 0x08: ld_mem_nn_sp(); break;
    case 0xF8: ld_hl_sp_e(); break;
    case 0xF9: ld_sp_hl(); break;
    case 0xC5: push_rr(Reg16::bc); break;
    case 0xD5: push_rr(Reg16::de); break;
    case 0xE5: push_rr(Reg16::hl); break;
    case 0xF5: push_rr(Reg16::af); break;
    case 0xC1: pop_rr(Reg16::bc); break;
    case 0xD1: pop_rr(Reg16::de); break;
    case 0xE1: pop_rr(Reg16::hl); break;
    case 0xF1: pop_rr(Reg16::af); break;

    // 16-bit arithmetic
    case 0x09: add_hl_rr(Reg16::bc); break;
    case 0x19: add_hl_rr(Reg16::de); break;
    case 0x29: add_hl_rr(Reg16::hl); break;
    case 0x39: add_hl_rr(Reg16::sp); break;
    case 0xE8: add_sp_e(); break;
    case 0x03: inc_rr(Reg16::bc); break;
    case 0x13: inc_rr(Reg16::de); break;
    case 0x23: inc_rr(Reg16::hl); break;
    case 0x33: inc_rr(Reg16::sp); break;
    case 0x0B: dec_rr(Reg16::bc); break;
    case 0x1B: dec_rr(Reg16::de); break;
    case 0x2B: dec_rr(Reg16::hl); break;
    case 0x3B: dec_rr(Reg16::sp); break;

    // Control flow
    case 0xC3: jp_nn(); break;
    case 0xE9: jp_hl(); break;
    case 0xC2: jp_cc_nn(Condition::nz); break;
    case 0xCA: jp_cc_nn(Condition::z); break;
    case 0xD2: jp_cc_nn(Condition::nc); break;
    case 0xDA: jp_cc_nn(Condition::c); break;
    case 0x18: jr_e(); break;
    case 0x20: jr_cc_e(Condition::nz); break;
    case 0x28: jr_cc_e(Condition::z); break;
    case 0x30: jr_cc_e(Condition::nc); break;
    case 0x38: jr_cc_e(Condition::c); break;
    case 0xCD: call_nn(); break;
    case 0xC4: call_cc_nn(Condition::nz); break;
    case 0xCC: call_cc_nn(Condition::z); break;
    case 0xD4: call_cc_nn(Condition::nc); break;
    case 0xDC: call_cc_nn(Condition::c); break;
    case 0xC9: ret(); break;
    case 0xC0: ret_cc(Condition::nz); break;
    case 0xC8: ret_cc(Condition::z); break;
    case 0xD0: ret_cc(Condition::nc); break;
    case 0xD8: ret_cc(Condition::c); break;
    case 0xD9: reti(); break;
    case 0xC7: rst_n(0x00); break;
    case 0xCF: rst_n(0x08); break;
    case 0xD7: rst_n(0x10); break;
    case 0xDF: rst_n(0x18); break;
    case 0xE7: rst_n(0x20); break;
    case 0xEF: rst_n(0x28); break;
    case 0xF7: rst_n(0x30); break;
    case 0xFF: rst_n(0x38); break;

    // Miscellaneous
    case 0x76: halt(); break;
    case 0x10: stop(); break;
    case 0xF3: di(); break;
    case 0xFB: ei(); break;
    case 0x00: nop(); break;

    // Rotate accumulator
    case 0x07: rlca(); break;
    case 0x0F: rrca(); break;
    case 0x17: rla(); break;
    case 0x1F: rra(); break;

    // Unusable opcodes
    case 0xD3:
    case 0xDB:
    case 0xDD:
    case 0xE3:
    case 0xE4:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xF4:
    case 0xFC:
    case 0xFD: break;

    // CB prefixed
    case 0xCB: {
      // Eat the prefix
      opcode = read_operand();
      switch (opcode) {
        // Rotate, shift, and bit operations
        case 0x07: rlc_r(Reg8::a); break;
        case 0x00: rlc_r(Reg8::b); break;
        case 0x01: rlc_r(Reg8::c); break;
        case 0x02: rlc_r(Reg8::d); break;
        case 0x03: rlc_r(Reg8::e); break;
        case 0x04: rlc_r(Reg8::h); break;
        case 0x05: rlc_r(Reg8::l); break;
        case 0x06: rlc_mem_hl(); break;
        case 0x0F: rrc_r(Reg8::a); break;
        case 0x08: rrc_r(Reg8::b); break;
        case 0x09: rrc_r(Reg8::c); break;
        case 0x0A: rrc_r(Reg8::d); break;
        case 0x0B: rrc_r(Reg8::e); break;
        case 0x0C: rrc_r(Reg8::h); break;
        case 0x0D: rrc_r(Reg8::l); break;
        case 0x0E: rrc_mem_hl(); break;
        case 0x17: rl_r(Reg8::a); break;
        case 0x10: rl_r(Reg8::b); break;
        case 0x11: rl_r(Reg8::c); break;
        case 0x12: rl_r(Reg8::d); break;
        case 0x13: rl_r(Reg8::e); break;
        case 0x14: rl_r(Reg8::h); break;
        case 0x15: rl_r(Reg8::l); break;
        case 0x16: rl_mem_hl(); break;
        case 0x1F: rr_r(Reg8::a); break;
        case 0x18: rr_r(Reg8::b); break;
        case 0x19: rr_r(Reg8::c); break;
        case 0x1A: rr_r(Reg8::d); break;
        case 0x1B: rr_r(Reg8::e); break;
        case 0x1C: rr_r(Reg8::h); break;
        case 0x1D: rr_r(Reg8::l); break;
        case 0x1E: rr_mem_hl(); break;
        case 0x27: sla_r(Reg8::a); break;
        case 0x20: sla_r(Reg8::b); break;
        case 0x21: sla_r(Reg8::c); break;
        case 0x22: sla_r(Reg8::d); break;
        case 0x23: sla_r(Reg8::e); break;
        case 0x24: sla_r(Reg8::h); break;
        case 0x25: sla_r(Reg8::l); break;
        case 0x26: sla_mem_hl(); break;
        case 0x2F: sra_r(Reg8::a); break;
        case 0x28: sra_r(Reg8::b); break;
        case 0x29: sra_r(Reg8::c); break;
        case 0x2A: sra_r(Reg8::d); break;
        case 0x2B: sra_r(Reg8::e); break;
        case 0x2C: sra_r(Reg8::h); break;
        case 0x2D: sra_r(Reg8::l); break;
        case 0x2E: sra_mem_hl(); break;
        case 0x37: swap_r(Reg8::a); break;
        case 0x30: swap_r(Reg8::b); break;
        case 0x31: swap_r(Reg8::c); break;
        case 0x32: swap_r(Reg8::d); break;
        case 0x33: swap_r(Reg8::e); break;
        case 0x34: swap_r(Reg8::h); break;
        case 0x35: swap_r(Reg8::l); break;
        case 0x36: swap_mem_hl(); break;
        case 0x3F: srl_r(Reg8::a); break;
        case 0x38: srl_r(Reg8::b); break;
        case 0x39: srl_r(Reg8::c); break;
        case 0x3A: srl_r(Reg8::d); break;
        case 0x3B: srl_r(Reg8::e); break;
        case 0x3C: srl_r(Reg8::h); break;
        case 0x3D: srl_r(Reg8::l); break;
        case 0x3E: srl_mem_hl(); break;
        case 0x47: bit_b_r(0, Reg8::a); break;
        case 0x4F: bit_b_r(1, Reg8::a); break;
        case 0x57: bit_b_r(2, Reg8::a); break;
        case 0x5F: bit_b_r(3, Reg8::a); break;
        case 0x67: bit_b_r(4, Reg8::a); break;
        case 0x6F: bit_b_r(5, Reg8::a); break;
        case 0x77: bit_b_r(6, Reg8::a); break;
        case 0x7F: bit_b_r(7, Reg8::a); break;
        case 0x40: bit_b_r(0, Reg8::b); break;
        case 0x48: bit_b_r(1, Reg8::b); break;
        case 0x50: bit_b_r(2, Reg8::b); break;
        case 0x58: bit_b_r(3, Reg8::b); break;
        case 0x60: bit_b_r(4, Reg8::b); break;
        case 0x68: bit_b_r(5, Reg8::b); break;
        case 0x70: bit_b_r(6, Reg8::b); break;
        case 0x78: bit_b_r(7, Reg8::b); break;
        case 0x41: bit_b_r(0, Reg8::c); break;
        case 0x49: bit_b_r(1, Reg8::c); break;
        case 0x51: bit_b_r(2, Reg8::c); break;
        case 0x59: bit_b_r(3, Reg8::c); break;
        case 0x61: bit_b_r(4, Reg8::c); break;
        case 0x69: bit_b_r(5, Reg8::c); break;
        case 0x71: bit_b_r(6, Reg8::c); break;
        case 0x79: bit_b_r(7, Reg8::c); break;
        case 0x42: bit_b_r(0, Reg8::d); break;
        case 0x4A: bit_b_r(1, Reg8::d); break;
        case 0x52: bit_b_r(2, Reg8::d); break;
        case 0x5A: bit_b_r(3, Reg8::d); break;
        case 0x62: bit_b_r(4, Reg8::d); break;
        case 0x6A: bit_b_r(5, Reg8::d); break;
        case 0x72: bit_b_r(6, Reg8::d); break;
        case 0x7A: bit_b_r(7, Reg8::d); break;
        case 0x43: bit_b_r(0, Reg8::e); break;
        case 0x4B: bit_b_r(1, Reg8::e); break;
        case 0x53: bit_b_r(2, Reg8::e); break;
        case 0x5B: bit_b_r(3, Reg8::e); break;
        case 0x63: bit_b_r(4, Reg8::e); break;
        case 0x6B: bit_b_r(5, Reg8::e); break;
        case 0x73: bit_b_r(6, Reg8::e); break;
        case 0x7B: bit_b_r(7, Reg8::e); break;
        case 0x44: bit_b_r(0, Reg8::h); break;
        case 0x4C: bit_b_r(1, Reg8::h); break;
        case 0x54: bit_b_r(2, Reg8::h); break;
        case 0x5C: bit_b_r(3, Reg8::h); break;
        case 0x64: bit_b_r(4, Reg8::h); break;
        case 0x6C: bit_b_r(5, Reg8::h); break;
        case 0x74: bit_b_r(6, Reg8::h); break;
        case 0x7C: bit_b_r(7, Reg8::h); break;
        case 0x45: bit_b_r(0, Reg8::l); break;
        case 0x4D: bit_b_r(1, Reg8::l); break;
        case 0x55: bit_b_r(2, Reg8::l); break;
        case 0x5D: bit_b_r(3, Reg8::l); break;
        case 0x65: bit_b_r(4, Reg8::l); break;
        case 0x6D: bit_b_r(5, Reg8::l); break;
        case 0x75: bit_b_r(6, Reg8::l); break;
        case 0x7D: bit_b_r(7, Reg8::l); break;
        case 0x46: bit_b_mem_hl(0); break;
        case 0x4E: bit_b_mem_hl(1); break;
        case 0x56: bit_b_mem_hl(2); break;
        case 0x5E: bit_b_mem_hl(3); break;
        case 0x66: bit_b_mem_hl(4); break;
        case 0x6E: bit_b_mem_hl(5); break;
        case 0x76: bit_b_mem_hl(6); break;
        case 0x7E: bit_b_mem_hl(7); break;
        case 0x87: res_b_r(0, Reg8::a); break;
        case 0x8F: res_b_r(1, Reg8::a); break;
        case 0x97: res_b_r(2, Reg8::a); break;
        case 0x9F: res_b_r(3, Reg8::a); break;
        case 0xA7: res_b_r(4, Reg8::a); break;
        case 0xAF: res_b_r(5, Reg8::a); break;
        case 0xB7: res_b_r(6, Reg8::a); break;
        case 0xBF: res_b_r(7, Reg8::a); break;
        case 0x80: res_b_r(0, Reg8::b); break;
        case 0x88: res_b_r(1, Reg8::b); break;
        case 0x90: res_b_r(2, Reg8::b); break;
        case 0x98: res_b_r(3, Reg8::b); break;
        case 0xA0: res_b_r(4, Reg8::b); break;
        case 0xA8: res_b_r(5, Reg8::b); break;
        case 0xB0: res_b_r(6, Reg8::b); break;
        case 0xB8: res_b_r(7, Reg8::b); break;
        case 0x81: res_b_r(0, Reg8::c); break;
        case 0x89: res_b_r(1, Reg8::c); break;
        case 0x91: res_b_r(2, Reg8::c); break;
        case 0x99: res_b_r(3, Reg8::c); break;
        case 0xA1: res_b_r(4, Reg8::c); break;
        case 0xA9: res_b_r(5, Reg8::c); break;
        case 0xB1: res_b_r(6, Reg8::c); break;
        case 0xB9: res_b_r(7, Reg8::c); break;
        case 0x82: res_b_r(0, Reg8::d); break;
        case 0x8A: res_b_r(1, Reg8::d); break;
        case 0x92: res_b_r(2, Reg8::d); break;
        case 0x9A: res_b_r(3, Reg8::d); break;
        case 0xA2: res_b_r(4, Reg8::d); break;
        case 0xAA: res_b_r(5, Reg8::d); break;
        case 0xB2: res_b_r(6, Reg8::d); break;
        case 0xBA: res_b_r(7, Reg8::d); break;
        case 0x83: res_b_r(0, Reg8::e); break;
        case 0x8B: res_b_r(1, Reg8::e); break;
        case 0x93: res_b_r(2, Reg8::e); break;
        case 0x9B: res_b_r(3, Reg8::e); break;
        case 0xA3: res_b_r(4, Reg8::e); break;
        case 0xAB: res_b_r(5, Reg8::e); break;
        case 0xB3: res_b_r(6, Reg8::e); break;
        case 0xBB: res_b_r(7, Reg8::e); break;
        case 0x84: res_b_r(0, Reg8::h); break;
        case 0x8C: res_b_r(1, Reg8::h); break;
        case 0x94: res_b_r(2, Reg8::h); break;
        case 0x9C: res_b_r(3, Reg8::h); break;
        case 0xA4: res_b_r(4, Reg8::h); break;
        case 0xAC: res_b_r(5, Reg8::h); break;
        case 0xB4: res_b_r(6, Reg8::h); break;
        case 0xBC: res_b_r(7, Reg8::h); break;
        case 0x85: res_b_r(0, Reg8::l); break;
        case 0x8D: res_b_r(1, Reg8::l); break;
        case 0x95: res_b_r(2, Reg8::l); break;
        case 0x9D: res_b_r(3, Reg8::l); break;
        case 0xA5: res_b_r(4, Reg8::l); break;
        case 0xAD: res_b_r(5, Reg8::l); break;
        case 0xB5: res_b_r(6, Reg8::l); break;
        case 0xBD: res_b_r(7, Reg8::l); break;
        case 0x86: res_b_mem_hl(0); break;
        case 0x8E: res_b_mem_hl(1); break;
        case 0x96: res_b_mem_hl(2); break;
        case 0x9E: res_b_mem_hl(3); break;
        case 0xA6: res_b_mem_hl(4); break;
        case 0xAE: res_b_mem_hl(5); break;
        case 0xB6: res_b_mem_hl(6); break;
        case 0xBE: res_b_mem_hl(7); break;
        case 0xC7: set_b_r(0, Reg8::a); break;
        case 0xCF: set_b_r(1, Reg8::a); break;
        case 0xD7: set_b_r(2, Reg8::a); break;
        case 0xDF: set_b_r(3, Reg8::a); break;
        case 0xE7: set_b_r(4, Reg8::a); break;
        case 0xEF: set_b_r(5, Reg8::a); break;
        case 0xF7: set_b_r(6, Reg8::a); break;
        case 0xFF: set_b_r(7, Reg8::a); break;
        case 0xC0: set_b_r(0, Reg8::b); break;
        case 0xC8: set_b_r(1, Reg8::b); break;
        case 0xD0: set_b_r(2, Reg8::b); break;
        case 0xD8: set_b_r(3, Reg8::b); break;
        case 0xE0: set_b_r(4, Reg8::b); break;
        case 0xE8: set_b_r(5, Reg8::b); break;
        case 0xF0: set_b_r(6, Reg8::b); break;
        case 0xF8: set_b_r(7, Reg8::b); break;
        case 0xC1: set_b_r(0, Reg8::c); break;
        case 0xC9: set_b_r(1, Reg8::c); break;
        case 0xD1: set_b_r(2, Reg8::c); break;
        case 0xD9: set_b_r(3, Reg8::c); break;
        case 0xE1: set_b_r(4, Reg8::c); break;
        case 0xE9: set_b_r(5, Reg8::c); break;
        case 0xF1: set_b_r(6, Reg8::c); break;
        case 0xF9: set_b_r(7, Reg8::c); break;
        case 0xC2: set_b_r(0, Reg8::d); break;
        case 0xCA: set_b_r(1, Reg8::d); break;
        case 0xD2: set_b_r(2, Reg8::d); break;
        case 0xDA: set_b_r(3, Reg8::d); break;
        case 0xE2: set_b_r(4, Reg8::d); break;
        case 0xEA: set_b_r(5, Reg8::d); break;
        case 0xF2: set_b_r(6, Reg8::d); break;
        case 0xFA: set_b_r(7, Reg8::d); break;
        case 0xC3: set_b_r(0, Reg8::e); break;
        case 0xCB: set_b_r(1, Reg8::e); break;
        case 0xD3: set_b_r(2, Reg8::e); break;
        case 0xDB: set_b_r(3, Reg8::e); break;
        case 0xE3: set_b_r(4, Reg8::e); break;
        case 0xEB: set_b_r(5, Reg8::e); break;
        case 0xF3: set_b_r(6, Reg8::e); break;
        case 0xFB: set_b_r(7, Reg8::e); break;
        case 0xC4: set_b_r(0, Reg8::h); break;
        case 0xCC: set_b_r(1, Reg8::h); break;
        case 0xD4: set_b_r(2, Reg8::h); break;
        case 0xDC: set_b_r(3, Reg8::h); break;
        case 0xE4: set_b_r(4, Reg8::h); break;
        case 0xEC: set_b_r(5, Reg8::h); break;
        case 0xF4: set_b_r(6, Reg8::h); break;
        case 0xFC: set_b_r(7, Reg8::h); break;
        case 0xC5: set_b_r(0, Reg8::l); break;
        case 0xCD: set_b_r(1, Reg8::l); break;
        case 0xD5: set_b_r(2, Reg8::l); break;
        case 0xDD: set_b_r(3, Reg8::l); break;
        case 0xE5: set_b_r(4, Reg8::l); break;
        case 0xED: set_b_r(5, Reg8::l); break;
        case 0xF5: set_b_r(6, Reg8::l); break;
        case 0xFD: set_b_r(7, Reg8::l); break;
        case 0xC6: set_b_mem_hl(0); break;
        case 0xCE: set_b_mem_hl(1); break;
        case 0xD6: set_b_mem_hl(2); break;
        case 0xDE: set_b_mem_hl(3); break;
        case 0xE6: set_b_mem_hl(4); break;
        case 0xEE: set_b_mem_hl(5); break;
        case 0xF6: set_b_mem_hl(6); break;
        case 0xFE: set_b_mem_hl(7); break;
      }
      break;
    }
    default: LOG_ERROR("Unimplemented opcode {:#04x}", opcode);
  }
}

}  // namespace gbcxx
