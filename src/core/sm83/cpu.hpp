#pragma once

#include <cstdint>
#include <fstream>
#include <utility>

#include "core/memory/bus.hpp"
#include "core/util.hpp"

namespace gb::sm83
{
enum class R8 : uint8_t
{
    B,
    C,
    D,
    E,
    H,
    L,
    A,
    F
};
enum class R16 : uint8_t
{
    Af,
    Bc,
    De,
    Hl,
    Sp,
    Pc
};
enum class Condition : uint8_t
{
    NotZero,
    Zero,
    NotCarry,
    Carry
};

class Cpu
{
public:
    explicit Cpu(std::vector<uint8_t> rom_data) : bus_(std::move(rom_data))
    {
#ifndef NDEBUG
        log_file_ << "";
#endif
    }

    [[nodiscard]] uint8_t GetReg(R8 r) const;
    [[nodiscard]] uint16_t GetReg(R16 r) const;
    memory::Bus& GetBus() { return bus_; }

    void SetReg(R8 r, uint8_t v);
    void SetReg(R16 r, uint16_t v);

    uint8_t Step();

private:
    void Tick4();
    void LogForGameBoyDoctor();

    uint8_t ReadOperand();
    uint16_t ReadOperands();

    uint8_t ReadByte(uint16_t addr);
    uint16_t ReadWord(uint16_t addr);
    void WriteByte(uint16_t addr, uint8_t val);
    void WriteWord(uint16_t addr, uint16_t val);

    void StackPush(uint16_t val);
    uint16_t StackPop();

    void HandleInterrupts();

    [[nodiscard]] bool EvaluateCondition(Condition cond) const;

    void InterpretInstruction();

    memory::Bus bus_;
    uint8_t cycles_{};

#ifndef NDEBUG
    std::fstream log_file_{"gameboy_doctor.log", std::ios::out};
#endif

    uint16_t pc_{0x0100};
    uint16_t sp_{0xfffe};
    uint8_t a_{0x01};
    uint8_t b_{0x00};
    uint8_t c_{0x13};
    uint8_t d_{0x00};
    uint8_t e_{0xd8};
    uint8_t h_{0x01};
    uint8_t l_{0x4d};

    bool zf_{true};
    bool nf_{false};
    bool hf_{true};
    bool cf_{true};

    bool ime_{true};
    bool ime_next_{};
    bool halt_{};
    bool halt_bug_{};

    // 8-bit loads
    template <R8 Dst, R8 Src>
    void Instr_LD_R_R();
    template <R8 Dst>
    void Instr_LD_R_N();
    template <R8 Dst>
    void Instr_LD_R_MEM_HL();
    template <R8 Src>
    void Instr_LD_MEM_HL_R();
    void Instr_LD_MEM_HL_N();
    template <R16 Src>
    void Instr_LD_A_MEM_RR();
    template <R16 Dst>
    void Instr_LD_MEM_RR_A();
    void Instr_LD_A_MEM_NN();
    void Instr_LD_MEM_NN_A();
    void Instr_LDH_A_MEM_C();
    void Instr_LDH_MEM_C_A();
    void Instr_LDH_A_MEM_N();
    void Instr_LDH_MEM_N_A();
    void Instr_LD_A_MEM_HL_DEC();
    void Instr_LD_MEM_HL_DEC_A();
    void Instr_LD_A_MEM_HL_INC();
    void Instr_LD_MEM_HL_INC_A();

    // 8-bit arithmetic and logical
    template <R8 R>
    void Instr_ADD_R();
    void Instr_ADD_MEM_HL();
    void Instr_ADD_N();
    template <R8 R>
    void Instr_ADC_R();
    void Instr_ADC_MEM_HL();
    void Instr_ADC_N();
    template <R8 R>
    void Instr_SUB_R();
    void Instr_SUB_MEM_HL();
    void Instr_SUB_N();
    template <R8 R>
    void Instr_SBC_R();
    void Instr_SBC_MEM_HL();
    void Instr_SBC_N();
    template <R8 R>
    void Instr_CP_R();
    void Instr_CP_MEM_HL();
    void Instr_CP_N();
    template <R8 R>
    void Instr_INC_R();
    void Instr_INC_MEM_HL();
    template <R8 R>
    void Instr_DEC_R();
    void Instr_DEC_MEM_HL();
    template <R8 R>
    void Instr_AND_R();
    void Instr_AND_MEM_HL();
    void Instr_AND_N();
    template <R8 R>
    void Instr_OR_R();
    void Instr_OR_MEM_HL();
    void Instr_OR_N();
    template <R8 R>
    void Instr_XOR_R();
    void Instr_XOR_MEM_HL();
    void Instr_XOR_N();
    void Instr_CCF();
    void Instr_SCF();
    void Instr_DAA();
    void Instr_CPL();

    // 16-bit loads
    template <R16 Dst>
    void Instr_LD_RR_NN();
    void Instr_LD_MEM_NN_SP();
    void Instr_LD_SP_HL();
    template <R16 Src>
    void Instr_PUSH_RR();
    template <R16 Dst>
    void Instr_POP_RR();
    void Instr_LD_HL_SP_E();

    // 16-bit arithmetic
    template <R16 RR>
    void Instr_INC_RR();
    template <R16 RR>
    void Instr_DEC_RR();
    template <R16 RR>
    void Instr_ADD_HL_RR();
    void Instr_ADD_SP_E();

    // Control flow
    void Instr_JP_NN();
    void Instr_JP_HL();
    template <Condition CC>
    void Instr_JP_CC_NN();
    void Instr_JR_E();
    template <Condition CC>
    void Instr_JR_CC_E();
    void Instr_CALL_NN();
    template <Condition CC>
    void Instr_CALL_CC_NN();
    void Instr_RET();
    template <Condition CC>
    void Instr_RET_CC();
    void Instr_RETI();
    template <uint8_t Vector>
    void Instr_RST_N();

    // Miscellaneous
    void Instr_HALT();
    void Instr_STOP();
    void Instr_DI();
    void Instr_EI();
    void Instr_NOP();

    // Rotate, shift, and bit operations
    void Instr_RLCA();
    void Instr_RRCA();
    void Instr_RLA();
    void Instr_RRA();
    template <R8 R>
    void Instr_RLC_R();
    void Instr_RLC_MEM_HL();
    template <R8 R>
    void Instr_RRC_R();
    void Instr_RRC_MEM_HL();
    template <R8 R>
    void Instr_RL_R();
    void Instr_RL_MEM_HL();
    template <R8 R>
    void Instr_RR_R();
    void Instr_RR_MEM_HL();
    template <R8 R>
    void Instr_SLA_R();
    void Instr_SLA_MEM_HL();
    template <R8 R>
    void Instr_SRA_R();
    void Instr_SRA_MEM_HL();
    template <R8 R>
    void Instr_SWAP_R();
    void Instr_SWAP_MEM_HL();
    template <R8 R>
    void Instr_SRL_R();
    void Instr_SRL_MEM_HL();
    template <uint8_t Bit, R8 R>
    void Instr_BIT_B_R();
    template <uint8_t Bit>
    void Instr_BIT_B_MEM_HL();
    template <uint8_t Bit, R8 R>
    void Instr_RES_B_R();
    template <uint8_t Bit>
    void Instr_RES_B_MEM_HL();
    template <uint8_t Bit, R8 R>
    void Instr_SET_B_R();
    template <uint8_t Bit>
    void Instr_SET_B_MEM_HL();
};

template <R8 Dst, R8 Src>
ALWAYS_INLINE void Cpu::Instr_LD_R_R()
{
    SetReg(Dst, GetReg(Src));
}

template <R8 Dst>
ALWAYS_INLINE void Cpu::Instr_LD_R_N()
{
    SetReg(Dst, ReadOperand());
}

template <R8 Dst>
ALWAYS_INLINE void Cpu::Instr_LD_R_MEM_HL()
{
    SetReg(Dst, ReadByte(GetReg(R16::Hl)));
}

template <R8 Src>
ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_R()
{
    WriteByte(GetReg(R16::Hl), GetReg(Src));
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_N() { WriteByte(GetReg(R16::Hl), ReadOperand()); }

template <R16 Src>
ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_RR()
{
    a_ = ReadByte(GetReg(Src));
}

template <R16 Dst>
ALWAYS_INLINE void Cpu::Instr_LD_MEM_RR_A()
{
    WriteByte(GetReg(Dst), a_);
}

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_NN() { a_ = ReadByte(ReadOperands()); }

ALWAYS_INLINE void Cpu::Instr_LD_MEM_NN_A() { WriteByte(ReadOperands(), a_); }

ALWAYS_INLINE void Cpu::Instr_LDH_A_MEM_C() { a_ = ReadByte(0xff00 + c_); }

ALWAYS_INLINE void Cpu::Instr_LDH_MEM_C_A() { WriteByte(0xff00 + c_, a_); }

ALWAYS_INLINE void Cpu::Instr_LDH_A_MEM_N() { a_ = ReadByte(0xff00 + ReadOperand()); }

ALWAYS_INLINE void Cpu::Instr_LDH_MEM_N_A() { WriteByte(0xff00 + ReadOperand(), a_); }

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_HL_DEC()
{
    const uint16_t hl = GetReg(R16::Hl);
    a_ = ReadByte(hl);
    SetReg(R16::Hl, hl - 1);
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_DEC_A()
{
    const uint16_t hl = GetReg(R16::Hl);
    WriteByte(hl, a_);
    SetReg(R16::Hl, hl - 1);
}

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_HL_INC()
{
    const uint16_t hl = GetReg(R16::Hl);
    a_ = ReadByte(hl);
    SetReg(R16::Hl, hl + 1);
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_INC_A()
{
    const uint16_t hl = GetReg(R16::Hl);
    WriteByte(hl, a_);
    SetReg(R16::Hl, hl + 1);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_ADD_R()
{
    const uint8_t val = GetReg(R);
    const uint16_t result = a_ + val;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a_ & 0xf) + (val & 0xf)) > 0xf;
    cf_ = result > 0xff;

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_ADD_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint16_t result = a_ + val;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a_ & 0xf) + (val & 0xf)) > 0xf;
    cf_ = result > 0xff;

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_ADD_N()
{
    const uint8_t n = ReadOperand();
    const uint16_t result = a_ + n;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a_ & 0xf) + (n & 0xf)) > 0xf;
    cf_ = result > 0xff;

    a_ = result & 0xff;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_ADC_R()
{
    const uint8_t val = GetReg(R);
    const uint16_t result = a_ + val + cf_;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a_ & 0xf) + (val & 0xf) + cf_) > 0xf;
    cf_ = result > 0xff;

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_ADC_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint16_t result = a_ + val + cf_;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a_ & 0xf) + (val & 0xf) + cf_) > 0xf;
    cf_ = result > 0xff;

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_ADC_N()
{
    const uint8_t n = ReadOperand();
    const uint16_t result = a_ + n + cf_;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a_ & 0xf) + (n & 0xf) + cf_) > 0xf;
    cf_ = result > 0xff;

    a_ = result & 0xff;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_SUB_R()
{
    const uint8_t val = GetReg(R);
    const uint8_t result = a_ - val;

    zf_ = !result;
    nf_ = true;
    hf_ = ((a_ & 0xf) - (val & 0xf)) < 0;
    cf_ = a_ < val;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_SUB_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint16_t result = a_ - val;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a_ & 0xf) - (val & 0xf)) < 0;
    cf_ = a_ < val;

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_SUB_N()
{
    const uint8_t n = ReadOperand();
    const uint16_t result = a_ - n;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a_ & 0xf) - (n & 0xf)) < 0;
    cf_ = a_ < n;

    a_ = result & 0xff;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_SBC_R()
{
    const uint8_t val = GetReg(R);
    const int result = a_ - val - cf_;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = (a_ & 0xf) < ((val & 0xf) + cf_);
    cf_ = a_ < (val + cf_);

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_SBC_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const int result = a_ - val - cf_;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = (a_ & 0xf) < ((val & 0xf) + cf_);
    cf_ = a_ < (val + cf_);

    a_ = result & 0xff;
}

ALWAYS_INLINE void Cpu::Instr_SBC_N()
{
    const uint8_t n = ReadOperand();
    const uint16_t result = a_ - n - cf_;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a_ & 0xf) < ((n & 0xf) + cf_));
    cf_ = a_ < (n + cf_);

    a_ = result & 0xff;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_CP_R()
{
    const uint8_t val = GetReg(R);
    const uint8_t result = a_ - val;

    zf_ = !result;
    nf_ = true;
    hf_ = (a_ & 0xf) - (val & 0xf) < 0;
    cf_ = a_ < val;
}

ALWAYS_INLINE void Cpu::Instr_CP_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint16_t result = a_ - val;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a_ & 0xf) - (val & 0xf)) < 0;
    cf_ = a_ < val;
}

ALWAYS_INLINE void Cpu::Instr_CP_N()
{
    const uint8_t n = ReadOperand();
    const uint16_t result = a_ - n;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a_ & 0xf) - (n & 0xf)) < 0;
    cf_ = a_ < n;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_INC_R()
{
    const uint8_t val = GetReg(R);
    const uint16_t result = val + 1;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = (val & 0xf) == 0xf;

    SetReg(R, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_INC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const uint16_t result = val + 1;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = (val & 0xf) == 0xf;

    WriteByte(addr, result & 0xff);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_DEC_R()
{
    const uint8_t val = GetReg(R);
    const uint8_t result = val - 1;

    zf_ = !result;
    nf_ = true;
    hf_ = (result & 0xf) == 0xf;

    SetReg(R, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_DEC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const uint16_t result = val - 1;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = (result & 0xf) == 0xf;

    WriteByte(addr, result & 0xff);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_AND_R()
{
    const uint8_t val = GetReg(R);
    const uint8_t result = a_ & val;

    zf_ = !result;
    nf_ = false;
    hf_ = true;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_AND_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint8_t result = a_ & val;

    zf_ = !result;
    nf_ = false;
    hf_ = true;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_AND_N()
{
    const uint8_t n = ReadOperand();
    const uint8_t result = a_ & n;

    zf_ = !result;
    nf_ = false;
    hf_ = true;
    cf_ = false;

    a_ = result;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_OR_R()
{
    const uint8_t val = GetReg(R);
    const uint8_t result = a_ | val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_OR_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint8_t result = a_ | val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_OR_N()
{
    const uint8_t n = ReadOperand();
    const uint8_t result = a_ | n;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    a_ = result;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_XOR_R()
{
    const uint8_t val = GetReg(R);
    const uint8_t result = a_ ^ val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_XOR_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));
    const uint8_t result = a_ ^ val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_XOR_N()
{
    const uint8_t n = ReadOperand();
    const uint8_t result = a_ ^ n;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_CCF()
{
    nf_ = false;
    hf_ = false;
    cf_ = !cf_;
}

ALWAYS_INLINE void Cpu::Instr_SCF()
{
    nf_ = false;
    hf_ = false;
    cf_ = true;
}

ALWAYS_INLINE void Cpu::Instr_DAA()
{
    uint8_t result = a_;

    // ref:
    // https://forums.nesdev.org/viewtopic.php?p=196282&sid=38a75719934a07d0ae8ac78a3e1448ad#p196282
    if (!nf_)
    {
        if (cf_ || a_ > 0x99)
        {
            result += 0x60;
            cf_ = true;
        }
        if (hf_ || (a_ & 0x0f) > 0x09) result += 0x6;
    }
    else
    {
        if (cf_) result -= 0x60;
        if (hf_) result -= 0x6;
    }

    zf_ = !result;
    hf_ = false;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_CPL()
{
    a_ = ~a_;
    nf_ = true;
    hf_ = true;
}

template <R16 Dst>
ALWAYS_INLINE void Cpu::Instr_LD_RR_NN()
{
    SetReg(Dst, ReadOperands());
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_NN_SP() { WriteWord(ReadOperands(), sp_); }

ALWAYS_INLINE void Cpu::Instr_LD_SP_HL() { sp_ = GetReg(R16::Hl); }

template <R16 Src>
ALWAYS_INLINE void Cpu::Instr_PUSH_RR()
{
    StackPush(GetReg(Src));
}

template <R16 Dst>
ALWAYS_INLINE void Cpu::Instr_POP_RR()
{
    SetReg(Dst, StackPop());
}

ALWAYS_INLINE void Cpu::Instr_LD_HL_SP_E()
{
    const auto e = static_cast<int8_t>(ReadOperand());

    zf_ = false;
    nf_ = false;
    hf_ = ((sp_ & 0xf) + (e & 0xf)) > 0xf;
    cf_ = ((sp_ & 0xff) + (e & 0xff)) > 0xff;

    SetReg(R16::Hl, sp_ + static_cast<uint16_t>(e));
}

template <R16 RR>
ALWAYS_INLINE void Cpu::Instr_INC_RR()
{
    SetReg(RR, GetReg(RR) + 1);
}

template <R16 RR>
ALWAYS_INLINE void Cpu::Instr_DEC_RR()
{
    SetReg(RR, GetReg(RR) - 1);
}

template <R16 RR>
ALWAYS_INLINE void Cpu::Instr_ADD_HL_RR()
{
    const uint16_t hl = GetReg(R16::Hl);
    const uint16_t val = GetReg(RR);
    const int result = hl + val;

    nf_ = false;
    hf_ = (hl & 0xfff) + (val & 0xfff) > 0xfff;
    cf_ = result > 0xffff;

    SetReg(R16::Hl, static_cast<uint16_t>(result));
}

ALWAYS_INLINE void Cpu::Instr_ADD_SP_E()
{
    const auto e = static_cast<int8_t>(ReadOperand());

    zf_ = false;
    nf_ = false;
    hf_ = ((sp_ & 0xf) + (e & 0xf)) > 0xf;
    cf_ = ((sp_ & 0xff) + (e & 0xff)) > 0xff;

    sp_ += e;
}

ALWAYS_INLINE void Cpu::Instr_JP_NN()
{
    const uint16_t nn = ReadOperands();
    pc_ = nn;
}

ALWAYS_INLINE void Cpu::Instr_JP_HL() { pc_ = GetReg(R16::Hl); }

template <Condition CC>
ALWAYS_INLINE void Cpu::Instr_JP_CC_NN()
{
    const uint16_t nn = ReadOperands();
    if (EvaluateCondition(CC))
    {
        pc_ = nn;
        Tick4();
    }
}

ALWAYS_INLINE void Cpu::Instr_JR_E()
{
    const auto e = static_cast<int8_t>(ReadOperand());
    pc_ += e;
}

template <Condition CC>
ALWAYS_INLINE void Cpu::Instr_JR_CC_E()
{
    const auto e = static_cast<int8_t>(ReadOperand());
    if (EvaluateCondition(CC))
    {
        pc_ += e;
        Tick4();
    }
}

ALWAYS_INLINE void Cpu::Instr_CALL_NN()
{
    const uint16_t nn = ReadOperands();
    StackPush(pc_);
    pc_ = nn;
}

template <Condition CC>
ALWAYS_INLINE void Cpu::Instr_CALL_CC_NN()
{
    const uint16_t nn = ReadOperands();
    if (EvaluateCondition(CC))
    {
        StackPush(pc_);
        pc_ = nn;
        Tick4();
        Tick4();
        Tick4();
    }
}

ALWAYS_INLINE void Cpu::Instr_RET() { pc_ = StackPop(); }

template <Condition CC>
ALWAYS_INLINE void Cpu::Instr_RET_CC()
{
    if (EvaluateCondition(CC))
    {
        pc_ = StackPop();
        Tick4();
        Tick4();
        Tick4();
    }
}

ALWAYS_INLINE void Cpu::Instr_RETI()
{
    pc_ = StackPop();
    ime_ = true;
}

template <uint8_t Vector>
ALWAYS_INLINE void Cpu::Instr_RST_N()
{
    StackPush(pc_);
    pc_ = Vector;
}

ALWAYS_INLINE void Cpu::Instr_HALT()
{
    if (ime_ || !bus_.GetPendingInterrupts()) halt_ = true;
    else halt_bug_ = true;
}

ALWAYS_INLINE void Cpu::Instr_STOP()
{
    // to-do: https://pbs.twimg.com/media/E5jlgW9XIAEKj0t.png:large
}

ALWAYS_INLINE void Cpu::Instr_DI() { ime_ = false; }

ALWAYS_INLINE void Cpu::Instr_EI() { ime_next_ = true; }

ALWAYS_INLINE void Cpu::Instr_NOP() {}

ALWAYS_INLINE void Cpu::Instr_RLCA()
{
    const bool carry = a_ & (1 << 7);
    const auto result = static_cast<uint8_t>((a_ << 1) | carry);

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_RRCA()
{
    const bool carry = a_ & 1;
    const auto result = static_cast<uint8_t>((a_ >> 1) | (carry << 7));

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_RLA()
{
    const auto result = static_cast<uint8_t>((a_ << 1) | cf_);

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = a_ & (1 << 7);

    a_ = result;
}

ALWAYS_INLINE void Cpu::Instr_RRA()
{
    const auto result = static_cast<uint8_t>((a_ >> 1) | (cf_ << 7));

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = a_ & 1;

    a_ = result;
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_RLC_R()
{
    const uint8_t val = GetReg(R);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>((val << 1) | carry);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_RLC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>((val << 1) | carry);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_RRC_R()
{
    const uint8_t val = GetReg(R);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (carry << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_RRC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (carry << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_RL_R()
{
    const uint8_t val = GetReg(R);
    const auto result = static_cast<uint8_t>((val << 1) | cf_);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = val & (1 << 7);

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_RL_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const auto result = static_cast<uint8_t>((val << 1) | cf_);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = GetBit<7>(val);

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_RR_R()
{
    const uint8_t val = GetReg(R);
    const auto result = static_cast<uint8_t>((val >> 1) | (cf_ << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = val & 1;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_RR_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const auto result = static_cast<uint8_t>((val >> 1) | (cf_ << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = val & 1;

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_SLA_R()
{
    const uint8_t val = GetReg(R);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>(val << 1);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_SLA_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>(val << 1);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_SRA_R()
{
    const uint8_t val = GetReg(R);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (val & (1 << 7)));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_SRA_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (val & (1 << 7)));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_SWAP_R()
{
    const uint8_t val = GetReg(R);
    const auto result = static_cast<uint8_t>(((val & 0x0f) << 4) | (((val & 0xf0) >> 4) & 0xf));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_SWAP_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const auto result = static_cast<uint8_t>(((val & 0x0f) << 4) | (((val & 0xf0) >> 4) & 0xf));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    WriteByte(addr, result);
}

template <R8 R>
ALWAYS_INLINE void Cpu::Instr_SRL_R()
{
    const uint8_t val = GetReg(R);
    const bool carry = val & 1;
    const uint8_t result = val >> 1;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R, result);
}

ALWAYS_INLINE void Cpu::Instr_SRL_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);
    const bool carry = val & 1;
    const uint8_t result = val >> 1;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    WriteByte(addr, result);
}

template <uint8_t Bit, R8 R>
ALWAYS_INLINE void Cpu::Instr_BIT_B_R()
{
    zf_ = !GetBit<Bit>(GetReg(R));
    nf_ = false;
    hf_ = true;
}

template <uint8_t Bit>
ALWAYS_INLINE void Cpu::Instr_BIT_B_MEM_HL()
{
    const uint8_t val = ReadByte(GetReg(R16::Hl));

    zf_ = !GetBit<Bit>(val);
    nf_ = false;
    hf_ = true;
}

template <uint8_t Bit, R8 R>
ALWAYS_INLINE void Cpu::Instr_RES_B_R()
{
    SetReg(R, ClearBit<Bit>(GetReg(R)));
}

template <uint8_t Bit>
ALWAYS_INLINE void Cpu::Instr_RES_B_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);

    WriteByte(addr, ClearBit<Bit>(val));
}

template <uint8_t Bit, R8 R>
ALWAYS_INLINE void Cpu::Instr_SET_B_R()
{
    SetReg(R, SetBit<Bit>(GetReg(R)));
}

template <uint8_t Bit>
ALWAYS_INLINE void Cpu::Instr_SET_B_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = ReadByte(addr);

    WriteByte(addr, SetBit<Bit>(val));
}
}  // namespace gb::sm83
