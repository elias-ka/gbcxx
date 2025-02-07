#pragma once

#include <cstdint>
#include <fstream>

#include "core/interrupt.hpp"
#include "core/util.hpp"

namespace gb
{
class Core;

class Cpu
{
public:
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

    struct DisassembledInstruction
    {
        uint16_t addr;
        std::string mnemonic;
    };

    explicit Cpu(Core& gb);

    [[nodiscard]] uint8_t GetReg(R8 r) const;
    [[nodiscard]] uint16_t GetReg(R16 r) const;

    void SetReg(R8 r, uint8_t v);
    void SetReg(R16 r, uint16_t v);

    uint8_t Read(uint16_t addr) const;
    void Write(uint16_t addr, uint8_t val);

    uint16_t Run();
    void Irq(Interrupt interrupt);

private:
    void Tick4x();
    void LogForGameBoyDoctor();

    uint8_t ReadOperand();
    uint16_t ReadOperands();

    uint8_t BusRead8(uint16_t addr);
    uint16_t BusRead16(uint16_t addr);
    void BusWrite8(uint16_t addr, uint8_t val);
    void BusWrite16(uint16_t addr, uint16_t val);

    void StackPush(uint16_t val);
    uint16_t StackPop();

    void HandleInterrupts();

    [[nodiscard]] bool EvaluateCondition(Condition cond) const;

    void InterpretInstruction();
    void InterpretPrefixedInstruction();

    Core& gb_;
    std::fstream log_file_{"gameboy_doctor.log"};

    uint16_t cycles_{};
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

    uint8_t ie_{0x00};
    uint8_t if_{0xe1};

    bool halt_{false};
    bool halt_bug_{false};
    bool ime_{false};
    bool ime_delay_{false};

private:
    // 8-bit loads
    void Instr_LD_R_R(R8 dst, R8 src);
    void Instr_LD_R_N(R8 dst);
    void Instr_LD_R_MEM_HL(R8 dst);
    void Instr_LD_MEM_HL_R(R8 src);
    void Instr_LD_MEM_HL_N();
    void Instr_LD_A_MEM_RR(R16 src);
    void Instr_LD_MEM_RR_A(R16 dst);
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
    void Instr_ADD_R(R8 r);
    void Instr_ADD_MEM_HL();
    void Instr_ADD_N();
    void Instr_ADC_R(R8 r);
    void Instr_ADC_MEM_HL();
    void Instr_ADC_N();
    void Instr_SUB_R(R8 r);
    void Instr_SUB_MEM_HL();
    void Instr_SUB_N();
    void Instr_SBC_R(R8 r);
    void Instr_SBC_MEM_HL();
    void Instr_SBC_N();
    void Instr_CP_R(R8 r);
    void Instr_CP_MEM_HL();
    void Instr_CP_N();
    void Instr_INC_R(R8 r);
    void Instr_INC_MEM_HL();
    void Instr_DEC_R(R8 r);
    void Instr_DEC_MEM_HL();
    void Instr_AND_R(R8 r);
    void Instr_AND_MEM_HL();
    void Instr_AND_N();
    void Instr_OR_R(R8 r);
    void Instr_OR_MEM_HL();
    void Instr_OR_N();
    void Instr_XOR_R(R8 r);
    void Instr_XOR_MEM_HL();
    void Instr_XOR_N();
    void Instr_CCF();
    void Instr_SCF();
    void Instr_DAA();
    void Instr_CPL();

    // 16-bit loads
    void Instr_LD_RR_NN(R16 dst);
    void Instr_LD_MEM_NN_SP();
    void Instr_LD_SP_HL();
    void Instr_PUSH_RR(R16 src);
    void Instr_POP_RR(R16 dst);
    void Instr_LD_HL_SP_E();

    // 16-bit arithmetic
    void Instr_INC_RR(R16 rr);
    void Instr_DEC_RR(R16 rr);
    void Instr_ADD_HL_RR(R16 rr);
    void Instr_ADD_SP_E();

    // Control flow
    void Instr_JP_NN();
    void Instr_JP_HL();
    void Instr_JP_CC_NN(Condition cc);
    void Instr_JR_E();
    void Instr_JR_CC_E(Condition cc);
    void Instr_CALL_NN();
    void Instr_CALL_CC_NN(Condition cc);
    void Instr_RET();
    void Instr_RET_CC(Condition cc);
    void Instr_RETI();
    void Instr_RST_N(uint8_t vec);

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
    void Instr_RLC_R(R8 r);
    void Instr_RLC_MEM_HL();
    void Instr_RRC_R(R8 r);
    void Instr_RRC_MEM_HL();
    void Instr_RL_R(R8 r);
    void Instr_RL_MEM_HL();
    void Instr_RR_R(R8 r);
    void Instr_RR_MEM_HL();
    void Instr_SLA_R(R8 r);
    void Instr_SLA_MEM_HL();
    void Instr_SRA_R(R8 r);
    void Instr_SRA_MEM_HL();
    void Instr_SWAP_R(R8 r);
    void Instr_SWAP_MEM_HL();
    void Instr_SRL_R(R8 r);
    void Instr_SRL_MEM_HL();
    void Instr_BIT_B_R(uint8_t b, R8 r);
    void Instr_BIT_B_MEM_HL(uint8_t b);
    void Instr_RES_B_R(uint8_t b, R8 r);
    void Instr_RES_B_MEM_HL(uint8_t b);
    void Instr_SET_B_R(uint8_t b, R8 r);
    void Instr_SET_B_MEM_HL(uint8_t b);
};

ALWAYS_INLINE void Cpu::Instr_LD_R_R(R8 dst, R8 src)
{
    SetReg(dst, GetReg(src));
}

ALWAYS_INLINE void Cpu::Instr_LD_R_N(R8 dst)
{
    SetReg(dst, ReadOperand());
}

ALWAYS_INLINE void Cpu::Instr_LD_R_MEM_HL(R8 dst)
{
    SetReg(dst, BusRead8(GetReg(R16::Hl)));
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_R(R8 src)
{
    BusWrite8(GetReg(R16::Hl), GetReg(src));
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_N()
{
    BusWrite8(GetReg(R16::Hl), ReadOperand());
}

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_RR(R16 src)
{
    SetReg(R8::A, BusRead8(GetReg(src)));
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_RR_A(R16 dst)
{
    BusWrite8(GetReg(dst), GetReg(R8::A));
}

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_NN()
{
    SetReg(R8::A, BusRead8(ReadOperands()));
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_NN_A()
{
    BusWrite16(ReadOperands(), GetReg(R8::A));
}

ALWAYS_INLINE void Cpu::Instr_LDH_A_MEM_C()
{
    SetReg(R8::A, BusRead8(0xff00 + GetReg(R8::C)));
}

ALWAYS_INLINE void Cpu::Instr_LDH_MEM_C_A()
{
    BusWrite8(0xff00 + GetReg(R8::C), GetReg(R8::A));
}

ALWAYS_INLINE void Cpu::Instr_LDH_A_MEM_N()
{
    SetReg(R8::A, BusRead8(0xff00 + ReadOperand()));
}

ALWAYS_INLINE void Cpu::Instr_LDH_MEM_N_A()
{
    BusWrite8(0xff00 + ReadOperand(), GetReg(R8::A));
}

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_HL_DEC()
{
    const uint16_t hl = GetReg(R16::Hl);
    SetReg(R8::A, BusRead8(hl));
    SetReg(R16::Hl, hl - 1);
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_DEC_A()
{
    const uint16_t hl = GetReg(R16::Hl);
    BusWrite8(hl, GetReg(R8::A));
    SetReg(R16::Hl, hl - 1);
}

ALWAYS_INLINE void Cpu::Instr_LD_A_MEM_HL_INC()
{
    const uint16_t hl = GetReg(R16::Hl);
    SetReg(R8::A, BusRead8(hl));
    SetReg(R16::Hl, hl + 1);
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_HL_INC_A()
{
    const uint16_t hl = GetReg(R16::Hl);
    BusWrite8(hl, GetReg(R8::A));
    SetReg(R16::Hl, hl + 1);
}

ALWAYS_INLINE void Cpu::Instr_ADD_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint16_t result = a + val;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a & 0xf) + (val & 0xf)) > 0xf;
    cf_ = result > 0xff;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_ADD_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint16_t result = a + val;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a & 0xf) + (val & 0xf)) > 0xf;
    cf_ = result > 0xff;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_ADD_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint16_t result = a + n;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a & 0xf) + (n & 0xf)) > 0xf;
    cf_ = result > 0xff;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_ADC_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint16_t result = a + val + cf_;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a & 0xf) + (val & 0xf) + cf_) > 0xf;
    cf_ = result > 0xff;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_ADC_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint16_t result = a + val + cf_;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a & 0xf) + (val & 0xf) + cf_) > 0xf;
    cf_ = result > 0xff;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_ADC_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint16_t result = a + n + cf_;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = ((a & 0xf) + (n & 0xf) + cf_) > 0xf;
    cf_ = result > 0xff;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_SUB_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint8_t result = a - val;

    zf_ = !result;
    nf_ = true;
    hf_ = ((a & 0xf) - (val & 0xf)) < 0;
    cf_ = a < val;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_SUB_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint16_t result = a - val;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a & 0xf) - (val & 0xf)) < 0;
    cf_ = a < val;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_SUB_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint16_t result = a - n;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a & 0xf) - (n & 0xf)) < 0;
    cf_ = a < n;

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_SBC_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const int result = a - val - cf_;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = (a & 0xf) < ((val & 0xf) + cf_);
    cf_ = a < (val + cf_);

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_SBC_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const int result = a - val - cf_;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = (a & 0xf) < ((val & 0xf) + cf_);
    cf_ = a < (val + cf_);

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_SBC_N()
{
    const uint16_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint16_t result = a - n - cf_;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a & 0xf) < ((n & 0xf) + cf_));
    cf_ = a < (n + cf_);

    SetReg(R8::A, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_CP_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint8_t result = a - val;

    zf_ = !result;
    nf_ = true;
    hf_ = (a & 0xf) - (val & 0xf) < 0;
    cf_ = a < val;
}

ALWAYS_INLINE void Cpu::Instr_CP_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint16_t result = a - val;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a & 0xf) - (val & 0xf)) < 0;
    cf_ = a < val;
}

ALWAYS_INLINE void Cpu::Instr_CP_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint16_t result = a - n;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = ((a & 0xf) - (n & 0xf)) < 0;
    cf_ = a < n;
}

ALWAYS_INLINE void Cpu::Instr_INC_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const uint16_t result = val + 1;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = (val & 0xf) == 0xf;

    SetReg(r, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_INC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const uint16_t result = val + 1;

    zf_ = !(result & 0xff);
    nf_ = false;
    hf_ = (val & 0xf) == 0xf;

    BusWrite8(addr, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_DEC_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const uint8_t result = val - 1;

    zf_ = !result;
    nf_ = true;
    hf_ = (result & 0xf) == 0xf;

    SetReg(r, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_DEC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const uint16_t result = val - 1;

    zf_ = !(result & 0xff);
    nf_ = true;
    hf_ = (result & 0xf) == 0xf;

    BusWrite8(addr, result & 0xff);
}

ALWAYS_INLINE void Cpu::Instr_AND_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint8_t result = a & val;

    zf_ = !result;
    nf_ = false;
    hf_ = true;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_AND_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint8_t result = a & val;

    zf_ = !result;
    nf_ = false;
    hf_ = true;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_AND_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint8_t result = a & n;

    zf_ = !result;
    nf_ = false;
    hf_ = true;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_OR_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint8_t result = a | val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_OR_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint8_t result = a | val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_OR_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint8_t result = a | n;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_XOR_R(R8 r)
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = GetReg(r);
    const uint8_t result = a ^ val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_XOR_MEM_HL()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t val = BusRead8(GetReg(R16::Hl));
    const uint8_t result = a ^ val;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_XOR_N()
{
    const uint8_t a = GetReg(R8::A);
    const uint8_t n = ReadOperand();
    const uint8_t result = a ^ n;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(R8::A, result);
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
    const uint8_t a = GetReg(R8::A);
    uint8_t result = a;

    // ref:
    // https://forums.nesdev.org/viewtopic.php?p=196282&sid=38a75719934a07d0ae8ac78a3e1448ad#p196282
    if (!nf_)
    {
        if (cf_ || a > 0x99)
        {
            result += 0x60;
            cf_ = true;
        }
        if (hf_ || (a & 0x0f) > 0x09)
        {
            result += 0x6;
        }
    }
    else
    {
        if (cf_)
        {
            result -= 0x60;
        }
        if (hf_)
        {
            result -= 0x6;
        }
    }

    zf_ = !result;
    hf_ = false;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_CPL()
{
    SetReg(R8::A, ~GetReg(R8::A));
    nf_ = true;
    hf_ = true;
}

ALWAYS_INLINE void Cpu::Instr_LD_RR_NN(R16 dst)
{
    SetReg(dst, ReadOperands());
}

ALWAYS_INLINE void Cpu::Instr_LD_MEM_NN_SP()
{
    BusWrite16(ReadOperands(), GetReg(R16::Sp));
}

ALWAYS_INLINE void Cpu::Instr_LD_SP_HL()
{
    SetReg(R16::Sp, GetReg(R16::Hl));
}

ALWAYS_INLINE void Cpu::Instr_PUSH_RR(R16 src)
{
    StackPush(GetReg(src));
}

ALWAYS_INLINE void Cpu::Instr_POP_RR(R16 dst)
{
    SetReg(dst, StackPop());
}

ALWAYS_INLINE void Cpu::Instr_LD_HL_SP_E()
{
    const uint16_t sp = GetReg(R16::Sp);
    const auto e = static_cast<int8_t>(ReadOperand());

    zf_ = false;
    nf_ = false;
    hf_ = ((sp & 0xf) + (e & 0xf)) > 0xf;
    cf_ = ((sp & 0xff) + (e & 0xff)) > 0xff;

    SetReg(R16::Hl, sp + static_cast<uint16_t>(e));
}

ALWAYS_INLINE void Cpu::Instr_INC_RR(R16 rr)
{
    SetReg(rr, GetReg(rr) + 1);
}

ALWAYS_INLINE void Cpu::Instr_DEC_RR(R16 rr)
{
    SetReg(rr, GetReg(rr) - 1);
}

ALWAYS_INLINE void Cpu::Instr_ADD_HL_RR(R16 rr)
{
    const uint16_t hl = GetReg(R16::Hl);
    const uint16_t val = GetReg(rr);
    const int result = hl + val;

    nf_ = false;
    hf_ = (hl & 0xfff) + (val & 0xfff) > 0xfff;
    cf_ = result > 0xffff;

    SetReg(R16::Hl, static_cast<uint16_t>(result));
}

ALWAYS_INLINE void Cpu::Instr_ADD_SP_E()
{
    const uint16_t sp = GetReg(R16::Sp);
    const auto e = static_cast<int8_t>(ReadOperand());

    zf_ = false;
    nf_ = false;
    hf_ = ((sp & 0xf) + (e & 0xf)) > 0xf;
    cf_ = ((sp & 0xff) + (e & 0xff)) > 0xff;

    SetReg(R16::Sp, sp + static_cast<uint16_t>(e));
}

ALWAYS_INLINE void Cpu::Instr_JP_NN()
{
    const uint16_t nn = ReadOperands();
    pc_ = nn;
}

ALWAYS_INLINE void Cpu::Instr_JP_HL()
{
    pc_ = GetReg(R16::Hl);
}

ALWAYS_INLINE void Cpu::Instr_JP_CC_NN(Condition cc)
{
    const uint16_t nn = ReadOperands();
    if (EvaluateCondition(cc))
    {
        pc_ = nn;
        Tick4x();
    }
}

ALWAYS_INLINE void Cpu::Instr_JR_E()
{
    const auto e = static_cast<int8_t>(ReadOperand());
    pc_ += e;
}

ALWAYS_INLINE void Cpu::Instr_JR_CC_E(Condition cc)
{
    const auto e = static_cast<int8_t>(ReadOperand());
    if (EvaluateCondition(cc))
    {
        pc_ += e;
        Tick4x();
    }
}

ALWAYS_INLINE void Cpu::Instr_CALL_NN()
{
    const uint16_t nn = ReadOperands();
    StackPush(pc_);
    pc_ = nn;
}

ALWAYS_INLINE void Cpu::Instr_CALL_CC_NN(Condition cc)
{
    const uint16_t nn = ReadOperands();
    if (EvaluateCondition(cc))
    {
        StackPush(pc_);
        pc_ = nn;
        Tick4x();
        Tick4x();
        Tick4x();
    }
}

ALWAYS_INLINE void Cpu::Instr_RET()
{
    pc_ = StackPop();
}

ALWAYS_INLINE void Cpu::Instr_RET_CC(Condition cc)
{
    if (EvaluateCondition(cc))
    {
        pc_ = StackPop();
        Tick4x();
        Tick4x();
        Tick4x();
    }
}

ALWAYS_INLINE void Cpu::Instr_RETI()
{
    pc_ = StackPop();
    ime_ = true;
}

ALWAYS_INLINE void Cpu::Instr_RST_N(uint8_t vec)
{
    StackPush(pc_);
    pc_ = vec;
}

ALWAYS_INLINE void Cpu::Instr_HALT()
{
    if (ime_ || !(if_ & ie_ & 0x1f))
    {
        halt_ = true;
    }
    else
    {
        halt_bug_ = true;
    }
}

ALWAYS_INLINE void Cpu::Instr_STOP()
{
    // to-do: https://pbs.twimg.com/media/E5jlgW9XIAEKj0t.png:large
}

ALWAYS_INLINE void Cpu::Instr_DI()
{
    ime_ = false;
}

ALWAYS_INLINE void Cpu::Instr_EI()
{
    ime_delay_ = true;
}

ALWAYS_INLINE void Cpu::Instr_NOP() {}

ALWAYS_INLINE void Cpu::Instr_RLCA()
{
    const uint8_t a = GetReg(R8::A);
    const bool carry = a & (1 << 7);
    const auto result = static_cast<uint8_t>((a << 1) | carry);

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_RRCA()
{
    const uint8_t a = GetReg(R8::A);
    const bool carry = a & 1;
    const auto result = static_cast<uint8_t>((a >> 1) | (carry << 7));

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_RLA()
{
    const uint8_t a = GetReg(R8::A);
    const auto result = static_cast<uint8_t>((a << 1) | cf_);

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = a & (1 << 7);

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_RRA()
{
    const uint8_t a = GetReg(R8::A);
    const auto result = static_cast<uint8_t>((a >> 1) | (cf_ << 7));

    zf_ = false;
    nf_ = false;
    hf_ = false;
    cf_ = a & 1;

    SetReg(R8::A, result);
}

ALWAYS_INLINE void Cpu::Instr_RLC_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>((val << 1) | carry);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_RLC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>((val << 1) | carry);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_RRC_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (carry << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_RRC_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (carry << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_RL_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const auto result = static_cast<uint8_t>((val << 1) | cf_);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = val & (1 << 7);

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_RL_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const auto result = static_cast<uint8_t>((val << 1) | cf_);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = (val >> 7) & 1;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_RR_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const auto result = static_cast<uint8_t>((val >> 1) | (cf_ << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = val & 1;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_RR_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const auto result = static_cast<uint8_t>((val >> 1) | (cf_ << 7));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = val & 1;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_SLA_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>(val << 1);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_SLA_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const bool carry = val & (1 << 7);
    const auto result = static_cast<uint8_t>(val << 1);

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_SRA_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (val & (1 << 7)));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_SRA_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const bool carry = val & 1;
    const auto result = static_cast<uint8_t>((val >> 1) | (val & (1 << 7)));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_SWAP_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const auto result = static_cast<uint8_t>(((val & 0x0F) << 4) | (((val & 0xf0) >> 4) & 0xf));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_SWAP_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const auto result = static_cast<uint8_t>(((val & 0x0F) << 4) | (((val & 0xf0) >> 4) & 0xf));

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = false;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_SRL_R(R8 r)
{
    const uint8_t val = GetReg(r);
    const bool carry = val & 1;
    const uint8_t result = val >> 1;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    SetReg(r, result);
}

ALWAYS_INLINE void Cpu::Instr_SRL_MEM_HL()
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);
    const bool carry = val & 1;
    const uint8_t result = val >> 1;

    zf_ = !result;
    nf_ = false;
    hf_ = false;
    cf_ = carry;

    BusWrite8(addr, result);
}

ALWAYS_INLINE void Cpu::Instr_BIT_B_R(uint8_t b, R8 r)
{
    zf_ = !(GetReg(r) & (1 << b));
    nf_ = false;
    hf_ = true;
}

ALWAYS_INLINE void Cpu::Instr_BIT_B_MEM_HL(uint8_t b)
{
    const uint8_t val = BusRead8(GetReg(R16::Hl));

    zf_ = !(val & (1 << b));
    nf_ = false;
    hf_ = true;
}

ALWAYS_INLINE void Cpu::Instr_RES_B_R(uint8_t b, R8 r)
{
    SetReg(r, (GetReg(r) & ~(1 << b)));
}

ALWAYS_INLINE void Cpu::Instr_RES_B_MEM_HL(uint8_t b)
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);

    BusWrite8(addr, val & ~(1 << b));
}

ALWAYS_INLINE void Cpu::Instr_SET_B_R(uint8_t b, R8 r)
{
    SetReg(r, static_cast<uint8_t>(GetReg(r) | (1 << b)));
}

ALWAYS_INLINE void Cpu::Instr_SET_B_MEM_HL(uint8_t b)
{
    const uint16_t addr = GetReg(R16::Hl);
    const uint8_t val = BusRead8(addr);

    BusWrite8(addr, static_cast<uint8_t>(val | (1 << b)));
}
}  // namespace gb
