#include "core/processor.h"
#include "core/memory.h"
#include "util.h"
#include <fmt/base.h>

namespace cb
{
    namespace
    {
        constexpr u8 z_mask = BIT(7);
        constexpr u8 n_mask = BIT(6);
        constexpr u8 h_mask = BIT(5);
        constexpr u8 c_mask = BIT(4);
    } // namespace

    bool flags::z() const { return raw & z_mask; }
    bool flags::n() const { return raw & n_mask; }
    bool flags::h() const { return raw & h_mask; }
    bool flags::c() const { return raw & c_mask; }

    void flags::set(flag f, bool set)
    {
        switch (f)
        {
        case flag::z: set ? (raw |= z_mask) : (raw &= ~z_mask); break;
        case flag::n: set ? (raw |= n_mask) : (raw &= ~n_mask); break;
        case flag::h: set ? (raw |= h_mask) : (raw &= ~h_mask); break;
        case flag::c: set ? (raw |= c_mask) : (raw &= ~c_mask); break;
        }
    }

    u8 processor::reg(reg8 r) const
    {
        switch (r)
        {
        case reg8::a: return m_a;
        case reg8::b: return m_b;
        case reg8::c: return m_c;
        case reg8::d: return m_d;
        case reg8::e: return m_e;
        case reg8::h: return m_h;
        case reg8::l: return m_l;
        }
    }

    u16 processor::reg(reg16 r) const
    {
        switch (r)
        {
        case reg16::af: return static_cast<u16>(m_a << 8) | m_f.raw;
        case reg16::bc: return static_cast<u16>(m_b << 8) | m_c;
        case reg16::de: return static_cast<u16>(m_d << 8) | m_e;
        case reg16::hl: return static_cast<u16>(m_h << 8) | m_l;
        case reg16::sp: return m_sp;
        }
    }

    void processor::set_reg(reg8 reg, u8 val)
    {
        switch (reg)
        {
        case reg8::a: m_a = val; break;
        case reg8::b: m_b = val; break;
        case reg8::c: m_c = val; break;
        case reg8::d: m_d = val; break;
        case reg8::e: m_e = val; break;
        case reg8::h: m_h = val; break;
        case reg8::l: m_l = val; break;
        }
    }

    void processor::set_reg(reg16 reg, u16 val)
    {
        switch (reg)
        {
        case reg16::af:
            m_a = static_cast<u8>(val >> 8);
            m_f.raw = static_cast<u8>(val & 0x00F0);
            break;
        case reg16::bc:
            m_b = static_cast<u8>(val >> 8);
            m_c = static_cast<u8>(val & 0x00FF);
            break;
        case reg16::de:
            m_d = static_cast<u8>(val >> 8);
            m_e = static_cast<u8>(val & 0x00FF);
            break;
        case reg16::hl:
            m_h = static_cast<u8>(val >> 8);
            m_l = static_cast<u8>(val & 0x00FF);
            break;
        case reg16::sp: m_sp = val; break;
        }
    }

    void processor::step()
    {
        m_cycles_elapsed = 0;
        const auto opcode = read_operand();
        execute(opcode);
    }

    void processor::write8(u16 addr, u8 data)
    {
        tick4();
        m_mmu->write8(addr, data);
    }

    void processor::write16(u16 addr, u16 data)
    {
        tick4();
        m_mmu->write16(addr, data);
    }

    u8 processor::read_operand()
    {
        tick4();
        return m_mmu->read8(m_pc++);
    }

    u16 processor::read_operands()
    {
        const auto lo = read_operand();
        const auto hi = read_operand();
        return static_cast<u16>((hi << 8) | lo);
    }

    void processor::push8(u8 value) { write8(--m_sp, value); }

    void processor::push16(u16 value)
    {
        push8(value >> 8);
        push8(static_cast<u8>(value));
    }

    u8 processor::pop8() { return m_mmu->read8(m_sp++); }

    u16 processor::pop16()
    {
        const auto lo = pop8();
        const auto hi = pop8();
        return static_cast<u16>((hi << 8) | lo);
    }

    void processor::tick()
    {
        m_cycles_elapsed += 1;
        m_mmu->tick_components();
    }

    void processor::tick4()
    {
        tick();
        tick();
        tick();
        tick();
    }

    void processor::execute(u8 opcode)
    {
        LOG_TRACE("processor::execute({:#04x})", opcode);
        // https://gekkio.fi/files/gb-docs/gbctr.pdf
        switch (opcode)
        {
        // 8-bit loads
        case 0x7F: ld_r_r(reg8::a, reg8::a); break;
        case 0x78: ld_r_r(reg8::a, reg8::b); break;
        case 0x79: ld_r_r(reg8::a, reg8::c); break;
        case 0x7A: ld_r_r(reg8::a, reg8::d); break;
        case 0x7B: ld_r_r(reg8::a, reg8::e); break;
        case 0x7C: ld_r_r(reg8::a, reg8::h); break;
        case 0x7D: ld_r_r(reg8::a, reg8::l); break;
        case 0x47: ld_r_r(reg8::b, reg8::a); break;
        case 0x41: ld_r_r(reg8::b, reg8::c); break;
        case 0x42: ld_r_r(reg8::b, reg8::d); break;
        case 0x43: ld_r_r(reg8::b, reg8::e); break;
        case 0x44: ld_r_r(reg8::b, reg8::h); break;
        case 0x45: ld_r_r(reg8::b, reg8::l); break;
        case 0x4F: ld_r_r(reg8::c, reg8::a); break;
        case 0x48: ld_r_r(reg8::c, reg8::b); break;
        case 0x49: ld_r_r(reg8::c, reg8::c); break;
        case 0x4A: ld_r_r(reg8::c, reg8::d); break;
        case 0x4B: ld_r_r(reg8::c, reg8::e); break;
        case 0x4C: ld_r_r(reg8::c, reg8::h); break;
        case 0x4D: ld_r_r(reg8::c, reg8::l); break;
        case 0x57: ld_r_r(reg8::d, reg8::a); break;
        case 0x50: ld_r_r(reg8::d, reg8::b); break;
        case 0x51: ld_r_r(reg8::d, reg8::c); break;
        case 0x52: ld_r_r(reg8::d, reg8::d); break;
        case 0x53: ld_r_r(reg8::d, reg8::e); break;
        case 0x54: ld_r_r(reg8::d, reg8::h); break;
        case 0x55: ld_r_r(reg8::d, reg8::l); break;
        case 0x5F: ld_r_r(reg8::e, reg8::a); break;
        case 0x58: ld_r_r(reg8::e, reg8::b); break;
        case 0x59: ld_r_r(reg8::e, reg8::c); break;
        case 0x5A: ld_r_r(reg8::e, reg8::d); break;
        case 0x5B: ld_r_r(reg8::e, reg8::e); break;
        case 0x5C: ld_r_r(reg8::e, reg8::h); break;
        case 0x5D: ld_r_r(reg8::e, reg8::l); break;
        case 0x67: ld_r_r(reg8::h, reg8::a); break;
        case 0x60: ld_r_r(reg8::h, reg8::b); break;
        case 0x61: ld_r_r(reg8::h, reg8::c); break;
        case 0x62: ld_r_r(reg8::h, reg8::d); break;
        case 0x63: ld_r_r(reg8::h, reg8::e); break;
        case 0x64: ld_r_r(reg8::h, reg8::h); break;
        case 0x65: ld_r_r(reg8::h, reg8::l); break;
        case 0x6F: ld_r_r(reg8::l, reg8::a); break;
        case 0x68: ld_r_r(reg8::l, reg8::b); break;
        case 0x69: ld_r_r(reg8::l, reg8::c); break;
        case 0x6A: ld_r_r(reg8::l, reg8::d); break;
        case 0x6B: ld_r_r(reg8::l, reg8::e); break;
        case 0x6C: ld_r_r(reg8::l, reg8::h); break;
        case 0x6D: ld_r_r(reg8::l, reg8::l); break;
        case 0x3E: ld_r_n(reg8::a); break;
        case 0x06: ld_r_n(reg8::b); break;
        case 0x0E: ld_r_n(reg8::c); break;
        case 0x16: ld_r_n(reg8::d); break;
        case 0x1E: ld_r_n(reg8::e); break;
        case 0x26: ld_r_n(reg8::h); break;
        case 0x2E: ld_r_n(reg8::l); break;
        case 0x7E: ld_r_mem_hl(reg8::a); break;
        case 0x46: ld_r_mem_hl(reg8::b); break;
        case 0x4E: ld_r_mem_hl(reg8::c); break;
        case 0x56: ld_r_mem_hl(reg8::d); break;
        case 0x5E: ld_r_mem_hl(reg8::e); break;
        case 0x66: ld_r_mem_hl(reg8::h); break;
        case 0x6E: ld_r_mem_hl(reg8::l); break;
        case 0x77: ld_mem_hl_r(reg8::a); break;
        case 0x70: ld_mem_hl_r(reg8::b); break;
        case 0x71: ld_mem_hl_r(reg8::c); break;
        case 0x72: ld_mem_hl_r(reg8::d); break;
        case 0x73: ld_mem_hl_r(reg8::e); break;
        case 0x74: ld_mem_hl_r(reg8::h); break;
        case 0x75: ld_mem_hl_r(reg8::l); break;
        case 0x36: ld_mem_hl_n(); break;
        case 0x0A: ld_a_mem_rr(reg16::bc); break;
        case 0x1A: ld_a_mem_rr(reg16::de); break;
        case 0x02: ld_mem_rr_a(reg16::bc); break;
        case 0x12: ld_mem_rr_a(reg16::de); break;
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
        case 0x87: add_r(reg8::a); break;
        case 0x80: add_r(reg8::b); break;
        case 0x81: add_r(reg8::c); break;
        case 0x82: add_r(reg8::d); break;
        case 0x83: add_r(reg8::e); break;
        case 0x84: add_r(reg8::h); break;
        case 0x85: add_r(reg8::l); break;
        case 0x86: add_mem_hl(); break;
        case 0xC6: add_n(); break;
        case 0x8F: adc_r(reg8::a); break;
        case 0x88: adc_r(reg8::b); break;
        case 0x89: adc_r(reg8::c); break;
        case 0x8A: adc_r(reg8::d); break;
        case 0x8B: adc_r(reg8::e); break;
        case 0x8C: adc_r(reg8::h); break;
        case 0x8D: adc_r(reg8::l); break;
        case 0x8E: adc_mem_hl(); break;
        case 0xCE: adc_n(); break;
        case 0x97: sub_r(reg8::a); break;
        case 0x90: sub_r(reg8::b); break;
        case 0x91: sub_r(reg8::c); break;
        case 0x92: sub_r(reg8::d); break;
        case 0x93: sub_r(reg8::e); break;
        case 0x94: sub_r(reg8::h); break;
        case 0x95: sub_r(reg8::l); break;
        case 0x96: sub_mem_hl(); break;
        case 0xD6: sub_n(); break;
        case 0x9F: sbc_r(reg8::a); break;
        case 0x98: sbc_r(reg8::b); break;
        case 0x99: sbc_r(reg8::c); break;
        case 0x9A: sbc_r(reg8::d); break;
        case 0x9B: sbc_r(reg8::e); break;
        case 0x9C: sbc_r(reg8::h); break;
        case 0x9D: sbc_r(reg8::l); break;
        case 0x9E: sbc_mem_hl(); break;
        case 0xDE: sbc_n(); break;
        case 0xBF: cp_r(reg8::a); break;
        case 0xB8: cp_r(reg8::b); break;
        case 0xB9: cp_r(reg8::c); break;
        case 0xBA: cp_r(reg8::d); break;
        case 0xBB: cp_r(reg8::e); break;
        case 0xBC: cp_r(reg8::h); break;
        case 0xBD: cp_r(reg8::l); break;
        case 0xBE: cp_mem_hl(); break;
        case 0xFE: cp_n(); break;
        case 0x3C: inc_r(reg8::a); break;
        case 0x04: inc_r(reg8::b); break;
        case 0x0C: inc_r(reg8::c); break;
        case 0x14: inc_r(reg8::d); break;
        case 0x1C: inc_r(reg8::e); break;
        case 0x24: inc_r(reg8::h); break;
        case 0x2C: inc_r(reg8::l); break;
        case 0x34: inc_mem_hl(); break;
        case 0x3D: dec_r(reg8::a); break;
        case 0x05: dec_r(reg8::b); break;
        case 0x0D: dec_r(reg8::c); break;
        case 0x15: dec_r(reg8::d); break;
        case 0x1D: dec_r(reg8::e); break;
        case 0x25: dec_r(reg8::h); break;
        case 0x2D: dec_r(reg8::l); break;
        case 0x35: dec_mem_hl(); break;
        case 0xA7: and_r(reg8::a); break;
        case 0xA0: and_r(reg8::b); break;
        case 0xA1: and_r(reg8::c); break;
        case 0xA2: and_r(reg8::d); break;
        case 0xA3: and_r(reg8::e); break;
        case 0xA4: and_r(reg8::h); break;
        case 0xA5: and_r(reg8::l); break;
        case 0xA6: and_mem_hl(); break;
        case 0xE6: and_n(); break;
        case 0xB7: or_r(reg8::a); break;
        case 0xB0: or_r(reg8::b); break;
        case 0xB1: or_r(reg8::c); break;
        case 0xB2: or_r(reg8::d); break;
        case 0xB3: or_r(reg8::e); break;
        case 0xB4: or_r(reg8::h); break;
        case 0xB5: or_r(reg8::l); break;
        case 0xB6: or_mem_hl(); break;
        case 0xF6: or_n(); break;
        case 0xAF: xor_r(reg8::a); break;
        case 0xA8: xor_r(reg8::b); break;
        case 0xA9: xor_r(reg8::c); break;
        case 0xAA: xor_r(reg8::d); break;
        case 0xAB: xor_r(reg8::e); break;
        case 0xAC: xor_r(reg8::h); break;
        case 0xAD: xor_r(reg8::l); break;
        case 0xAE: xor_mem_hl(); break;
        case 0xEE: xor_n(); break;
        case 0x3F: ccf(); break;
        case 0x37: scf(); break;
        case 0x2F: cpl(); break;

        // 16-bit loads
        case 0x01: ld_rr_nn(reg16::bc); break;
        case 0x11: ld_rr_nn(reg16::de); break;
        case 0x21: ld_rr_nn(reg16::hl); break;
        case 0x31: ld_rr_nn(reg16::sp); break;
        case 0x08: ld_mem_nn_sp(); break;
        case 0xF8: ld_hl_sp_e(); break;
        case 0xF9: ld_sp_hl(); break;
        case 0xC5: push_rr(reg16::bc); break;
        case 0xD5: push_rr(reg16::de); break;
        case 0xE5: push_rr(reg16::hl); break;
        case 0xF5: push_rr(reg16::af); break;
        case 0xC1: pop_rr(reg16::bc); break;
        case 0xD1: pop_rr(reg16::de); break;
        case 0xE1: pop_rr(reg16::hl); break;
        case 0xF1: pop_rr(reg16::af); break;

        case 0x00: nop(); break;
        default: LOG_UNIMPLEMENTED("opcode {:#04x}", opcode);
        }
    }

} // namespace cb
