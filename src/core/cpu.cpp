#include "core/cpu.hpp"

namespace gb
{
uint8_t Cpu::Step()
{
    cycles_ = 0;

    HandleInterrupts();

    if (ime_next_)
    {
        ime_ = true;
        ime_next_ = false;
    }

    if (halt_)
        return 4;

#ifndef NDEBUG
    LogForGameBoyDoctor();
#endif

    InterpretInstruction();

    assert(cycles_ != 0);
    return cycles_;
}

void Cpu::Tick4()
{
    cycles_ += 4;
}

void Cpu::LogForGameBoyDoctor()
{
    log_file_ << fmt::format(
        "A:{:02X} F:{:02X} B:{:02X} C:{:02X} D:{:02X} E:{:02X} "
        "H:{:02X} L:{:02X} SP:{:04X} PC:{:04X} "
        "PCMEM:{:02X},{:02X},{:02X},{:02X}\n",
        a_, GetReg(R8::F), b_, c_, d_, e_, h_, l_, sp_, pc_, bus_.ReadByte(pc_),
        bus_.ReadByte(pc_ + 1), bus_.ReadByte(pc_ + 2), bus_.ReadByte(pc_ + 3));
}

// void Cpu::LogForGameBoyDoctor()
// {
//     log_file_ << fmt::format(
//         "A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} "
//         "H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} "
//         "{:02X})\n",
//         a_, GetReg(R8::F), b_, c_, d_, e_, h_, l_, sp_, pc_, bus_.ReadByte(pc_),
//         bus_.ReadByte(pc_ + 1), bus_.ReadByte(pc_ + 2), bus_.ReadByte(pc_ + 3));
// }

uint8_t Cpu::GetReg(R8 r) const
{
    switch (r)
    {
    case R8::B: return b_;
    case R8::C: return c_;
    case R8::D: return d_;
    case R8::E: return e_;
    case R8::H: return h_;
    case R8::L: return l_;
    case R8::A: return a_;
    case R8::F: return static_cast<uint8_t>((zf_ << 7) | (nf_ << 6) | (hf_ << 5) | (cf_ << 4));
    }
}

uint16_t Cpu::GetReg(R16 r) const
{
    switch (r)
    {
    case R16::Af: return static_cast<uint16_t>(GetReg(R8::A) << 8) | GetReg(R8::F);
    case R16::Bc: return static_cast<uint16_t>(GetReg(R8::B) << 8) | GetReg(R8::C);
    case R16::De: return static_cast<uint16_t>(GetReg(R8::D) << 8) | GetReg(R8::E);
    case R16::Hl: return static_cast<uint16_t>(GetReg(R8::H) << 8) | GetReg(R8::L);
    case R16::Sp: return sp_;
    case R16::Pc: return pc_;
    }
}

void Cpu::SetReg(R8 r, uint8_t v)
{
    switch (r)
    {
    case R8::B: b_ = v; break;
    case R8::C: c_ = v; break;
    case R8::D: d_ = v; break;
    case R8::E: e_ = v; break;
    case R8::H: h_ = v; break;
    case R8::L: l_ = v; break;
    case R8::A: a_ = v; break;
    case R8::F:
        zf_ = (v >> 7) & 1;
        nf_ = (v >> 6) & 1;
        hf_ = (v >> 5) & 1;
        cf_ = (v >> 4) & 1;
        break;
    }
}

void Cpu::SetReg(R16 r, uint16_t v)
{
    switch (r)
    {
    case R16::Af:
        SetReg(R8::A, v >> 8);
        SetReg(R8::F, v & 0xf0);
        break;
    case R16::Bc:
        SetReg(R8::B, v >> 8);
        SetReg(R8::C, v & 0xff);
        break;
    case R16::De:
        SetReg(R8::D, v >> 8);
        SetReg(R8::E, v & 0xff);
        break;
    case R16::Hl:
        SetReg(R8::H, v >> 8);
        SetReg(R8::L, v & 0xff);
        break;
    case R16::Sp: sp_ = v; break;
    case R16::Pc: pc_ = v; break;
    }
}

uint8_t Cpu::ReadOperand()
{
    Tick4();
    return ReadByte(pc_++);
}

uint16_t Cpu::ReadOperands()
{
    const auto lo = ReadOperand();
    const auto hi = ReadOperand();
    return static_cast<uint16_t>((hi << 8) | lo);
}

uint8_t Cpu::ReadByte(uint16_t addr)
{
    Tick4();
    return bus_.ReadByte(addr);
}

uint16_t Cpu::ReadWord(uint16_t addr)
{
    const uint8_t lo = ReadByte(addr);
    const uint8_t hi = ReadByte(addr + 1);
    return static_cast<uint16_t>((hi << 8) | lo);
}

void Cpu::WriteByte(uint16_t addr, uint8_t val)
{
    Tick4();
    bus_.WriteByte(addr, val);
}

void Cpu::WriteWord(uint16_t addr, uint16_t val)
{
    WriteByte(addr, val & 0xff);
    WriteByte(addr + 1, val >> 8);
}

void Cpu::StackPush(uint16_t val)
{
    sp_ -= 2;
    WriteWord(sp_, val);
}

uint16_t Cpu::StackPop()
{
    const uint16_t ret = ReadWord(sp_);
    sp_ += 2;
    return ret;
}

bool Cpu::HavePendingInterrupts() const
{
    return (bus_.interrupt_enable & bus_.interrupt_flag & 0x1f) != 0;
}

void Cpu::HandleInterrupts()
{
    if (!ime_ && !halt_)
        return;

    const uint8_t interrupts = bus_.interrupt_enable & bus_.interrupt_flag & 0x1f;
    if (!interrupts)
        return;

    if (halt_)
    {
        cycles_ += 4;
        halt_ = false;
    }

    if (!ime_)
        return;

    ime_ = false;
    if (interrupts & std::to_underlying(Interrupt::VBlank))
    {
        LOG_TRACE("CPU: Handling vblank interrupt");
        Instr_RST_N(0x40);
        bus_.interrupt_flag &= ~std::to_underlying(Interrupt::VBlank);
    }
    else if (interrupts & std::to_underlying(Interrupt::Lcd))
    {
        LOG_TRACE("CPU: Handling lcd interrupt");
        Instr_RST_N(0x48);
        bus_.interrupt_flag &= ~std::to_underlying(Interrupt::Lcd);
    }
    else if (interrupts & std::to_underlying(Interrupt::Timer))
    {
        LOG_TRACE("CPU: Handling timer interrupt");
        Instr_RST_N(0x50);
        bus_.interrupt_flag &= ~std::to_underlying(Interrupt::Timer);
    }
    else if (interrupts & std::to_underlying(Interrupt::Serial))
    {
        LOG_TRACE("CPU: Handling serial interrupt");
        Instr_RST_N(0x58);
        bus_.interrupt_flag &= ~std::to_underlying(Interrupt::Serial);
    }
    else if (interrupts & std::to_underlying(Interrupt::Joypad))
    {
        LOG_TRACE("CPU: Handling joypad interrupt");
        Instr_RST_N(0x60);
        bus_.interrupt_flag &= ~std::to_underlying(Interrupt::Joypad);
    }

    cycles_ += 20;
}

bool Cpu::EvaluateCondition(Condition cond) const
{
    switch (cond)
    {
    case Condition::NotZero: return !zf_;
    case Condition::Zero: return zf_;
    case Condition::NotCarry: return !cf_;
    case Condition::Carry: return cf_;
    }
}

void Cpu::InterpretInstruction()
{
    uint8_t opcode = ReadOperand();
    if (opcode != 0xcb)
        LOG_TRACE("CPU: opcode {:X}", opcode);

    switch (opcode)
    {
    // 8-bit loads
    case 0x7F: Instr_LD_R_R(R8::A, R8::A); break;
    case 0x78: Instr_LD_R_R(R8::A, R8::B); break;
    case 0x79: Instr_LD_R_R(R8::A, R8::C); break;
    case 0x7a: Instr_LD_R_R(R8::A, R8::D); break;
    case 0x7b: Instr_LD_R_R(R8::A, R8::E); break;
    case 0x7c: Instr_LD_R_R(R8::A, R8::H); break;
    case 0x7d: Instr_LD_R_R(R8::A, R8::L); break;
    case 0x47: Instr_LD_R_R(R8::B, R8::A); break;
    case 0x40: Instr_LD_R_R(R8::B, R8::B); break;
    case 0x41: Instr_LD_R_R(R8::B, R8::C); break;
    case 0x42: Instr_LD_R_R(R8::B, R8::D); break;
    case 0x43: Instr_LD_R_R(R8::B, R8::E); break;
    case 0x44: Instr_LD_R_R(R8::B, R8::H); break;
    case 0x45: Instr_LD_R_R(R8::B, R8::L); break;
    case 0x4f: Instr_LD_R_R(R8::C, R8::A); break;
    case 0x48: Instr_LD_R_R(R8::C, R8::B); break;
    case 0x49: Instr_LD_R_R(R8::C, R8::C); break;
    case 0x4a: Instr_LD_R_R(R8::C, R8::D); break;
    case 0x4b: Instr_LD_R_R(R8::C, R8::E); break;
    case 0x4c: Instr_LD_R_R(R8::C, R8::H); break;
    case 0x4d: Instr_LD_R_R(R8::C, R8::L); break;
    case 0x57: Instr_LD_R_R(R8::D, R8::A); break;
    case 0x50: Instr_LD_R_R(R8::D, R8::B); break;
    case 0x51: Instr_LD_R_R(R8::D, R8::C); break;
    case 0x52: Instr_LD_R_R(R8::D, R8::D); break;
    case 0x53: Instr_LD_R_R(R8::D, R8::E); break;
    case 0x54: Instr_LD_R_R(R8::D, R8::H); break;
    case 0x55: Instr_LD_R_R(R8::D, R8::L); break;
    case 0x5f: Instr_LD_R_R(R8::E, R8::A); break;
    case 0x58: Instr_LD_R_R(R8::E, R8::B); break;
    case 0x59: Instr_LD_R_R(R8::E, R8::C); break;
    case 0x5a: Instr_LD_R_R(R8::E, R8::D); break;
    case 0x5b: Instr_LD_R_R(R8::E, R8::E); break;
    case 0x5c: Instr_LD_R_R(R8::E, R8::H); break;
    case 0x5d: Instr_LD_R_R(R8::E, R8::L); break;
    case 0x67: Instr_LD_R_R(R8::H, R8::A); break;
    case 0x60: Instr_LD_R_R(R8::H, R8::B); break;
    case 0x61: Instr_LD_R_R(R8::H, R8::C); break;
    case 0x62: Instr_LD_R_R(R8::H, R8::D); break;
    case 0x63: Instr_LD_R_R(R8::H, R8::E); break;
    case 0x64: Instr_LD_R_R(R8::H, R8::H); break;
    case 0x65: Instr_LD_R_R(R8::H, R8::L); break;
    case 0x6f: Instr_LD_R_R(R8::L, R8::A); break;
    case 0x68: Instr_LD_R_R(R8::L, R8::B); break;
    case 0x69: Instr_LD_R_R(R8::L, R8::C); break;
    case 0x6a: Instr_LD_R_R(R8::L, R8::D); break;
    case 0x6b: Instr_LD_R_R(R8::L, R8::E); break;
    case 0x6c: Instr_LD_R_R(R8::L, R8::H); break;
    case 0x6d: Instr_LD_R_R(R8::L, R8::L); break;
    case 0x3e: Instr_LD_R_N(R8::A); break;
    case 0x06: Instr_LD_R_N(R8::B); break;
    case 0x0e: Instr_LD_R_N(R8::C); break;
    case 0x16: Instr_LD_R_N(R8::D); break;
    case 0x1e: Instr_LD_R_N(R8::E); break;
    case 0x26: Instr_LD_R_N(R8::H); break;
    case 0x2e: Instr_LD_R_N(R8::L); break;
    case 0x7e: Instr_LD_R_MEM_HL(R8::A); break;
    case 0x46: Instr_LD_R_MEM_HL(R8::B); break;
    case 0x4e: Instr_LD_R_MEM_HL(R8::C); break;
    case 0x56: Instr_LD_R_MEM_HL(R8::D); break;
    case 0x5e: Instr_LD_R_MEM_HL(R8::E); break;
    case 0x66: Instr_LD_R_MEM_HL(R8::H); break;
    case 0x6e: Instr_LD_R_MEM_HL(R8::L); break;
    case 0x77: Instr_LD_MEM_HL_R(R8::A); break;
    case 0x70: Instr_LD_MEM_HL_R(R8::B); break;
    case 0x71: Instr_LD_MEM_HL_R(R8::C); break;
    case 0x72: Instr_LD_MEM_HL_R(R8::D); break;
    case 0x73: Instr_LD_MEM_HL_R(R8::E); break;
    case 0x74: Instr_LD_MEM_HL_R(R8::H); break;
    case 0x75: Instr_LD_MEM_HL_R(R8::L); break;
    case 0x36: Instr_LD_MEM_HL_N(); break;
    case 0x0a: Instr_LD_A_MEM_RR(R16::Bc); break;
    case 0x1a: Instr_LD_A_MEM_RR(R16::De); break;
    case 0x02: Instr_LD_MEM_RR_A(R16::Bc); break;
    case 0x12: Instr_LD_MEM_RR_A(R16::De); break;
    case 0xfa: Instr_LD_A_MEM_NN(); break;
    case 0xea: Instr_LD_MEM_NN_A(); break;
    case 0xf2: Instr_LDH_A_MEM_C(); break;
    case 0xe2: Instr_LDH_MEM_C_A(); break;
    case 0xf0: Instr_LDH_A_MEM_N(); break;
    case 0xe0: Instr_LDH_MEM_N_A(); break;
    case 0x3a: Instr_LD_A_MEM_HL_DEC(); break;
    case 0x32: Instr_LD_MEM_HL_DEC_A(); break;
    case 0x2a: Instr_LD_A_MEM_HL_INC(); break;
    case 0x22: Instr_LD_MEM_HL_INC_A(); break;

    // 8-bit arithmetic and logical
    case 0x87: Instr_ADD_R(R8::A); break;
    case 0x80: Instr_ADD_R(R8::B); break;
    case 0x81: Instr_ADD_R(R8::C); break;
    case 0x82: Instr_ADD_R(R8::D); break;
    case 0x83: Instr_ADD_R(R8::E); break;
    case 0x84: Instr_ADD_R(R8::H); break;
    case 0x85: Instr_ADD_R(R8::L); break;
    case 0x86: Instr_ADD_MEM_HL(); break;
    case 0xc6: Instr_ADD_N(); break;
    case 0x8f: Instr_ADC_R(R8::A); break;
    case 0x88: Instr_ADC_R(R8::B); break;
    case 0x89: Instr_ADC_R(R8::C); break;
    case 0x8a: Instr_ADC_R(R8::D); break;
    case 0x8b: Instr_ADC_R(R8::E); break;
    case 0x8c: Instr_ADC_R(R8::H); break;
    case 0x8d: Instr_ADC_R(R8::L); break;
    case 0x8e: Instr_ADC_MEM_HL(); break;
    case 0xce: Instr_ADC_N(); break;
    case 0x97: Instr_SUB_R(R8::A); break;
    case 0x90: Instr_SUB_R(R8::B); break;
    case 0x91: Instr_SUB_R(R8::C); break;
    case 0x92: Instr_SUB_R(R8::D); break;
    case 0x93: Instr_SUB_R(R8::E); break;
    case 0x94: Instr_SUB_R(R8::H); break;
    case 0x95: Instr_SUB_R(R8::L); break;
    case 0x96: Instr_SUB_MEM_HL(); break;
    case 0xd6: Instr_SUB_N(); break;
    case 0x9f: Instr_SBC_R(R8::A); break;
    case 0x98: Instr_SBC_R(R8::B); break;
    case 0x99: Instr_SBC_R(R8::C); break;
    case 0x9a: Instr_SBC_R(R8::D); break;
    case 0x9b: Instr_SBC_R(R8::E); break;
    case 0x9c: Instr_SBC_R(R8::H); break;
    case 0x9d: Instr_SBC_R(R8::L); break;
    case 0x9e: Instr_SBC_MEM_HL(); break;
    case 0xde: Instr_SBC_N(); break;
    case 0xbf: Instr_CP_R(R8::A); break;
    case 0xb8: Instr_CP_R(R8::B); break;
    case 0xb9: Instr_CP_R(R8::C); break;
    case 0xba: Instr_CP_R(R8::D); break;
    case 0xbb: Instr_CP_R(R8::E); break;
    case 0xbc: Instr_CP_R(R8::H); break;
    case 0xbd: Instr_CP_R(R8::L); break;
    case 0xbe: Instr_CP_MEM_HL(); break;
    case 0xfe: Instr_CP_N(); break;
    case 0x3c: Instr_INC_R(R8::A); break;
    case 0x04: Instr_INC_R(R8::B); break;
    case 0x0c: Instr_INC_R(R8::C); break;
    case 0x14: Instr_INC_R(R8::D); break;
    case 0x1c: Instr_INC_R(R8::E); break;
    case 0x24: Instr_INC_R(R8::H); break;
    case 0x2c: Instr_INC_R(R8::L); break;
    case 0x34: Instr_INC_MEM_HL(); break;
    case 0x3d: Instr_DEC_R(R8::A); break;
    case 0x05: Instr_DEC_R(R8::B); break;
    case 0x0d: Instr_DEC_R(R8::C); break;
    case 0x15: Instr_DEC_R(R8::D); break;
    case 0x1d: Instr_DEC_R(R8::E); break;
    case 0x25: Instr_DEC_R(R8::H); break;
    case 0x2d: Instr_DEC_R(R8::L); break;
    case 0x35: Instr_DEC_MEM_HL(); break;
    case 0xa7: Instr_AND_R(R8::A); break;
    case 0xa0: Instr_AND_R(R8::B); break;
    case 0xa1: Instr_AND_R(R8::C); break;
    case 0xa2: Instr_AND_R(R8::D); break;
    case 0xa3: Instr_AND_R(R8::E); break;
    case 0xa4: Instr_AND_R(R8::H); break;
    case 0xa5: Instr_AND_R(R8::L); break;
    case 0xa6: Instr_AND_MEM_HL(); break;
    case 0xe6: Instr_AND_N(); break;
    case 0xb7: Instr_OR_R(R8::A); break;
    case 0xb0: Instr_OR_R(R8::B); break;
    case 0xb1: Instr_OR_R(R8::C); break;
    case 0xb2: Instr_OR_R(R8::D); break;
    case 0xb3: Instr_OR_R(R8::E); break;
    case 0xb4: Instr_OR_R(R8::H); break;
    case 0xb5: Instr_OR_R(R8::L); break;
    case 0xb6: Instr_OR_MEM_HL(); break;
    case 0xf6: Instr_OR_N(); break;
    case 0xaf: Instr_XOR_R(R8::A); break;
    case 0xa8: Instr_XOR_R(R8::B); break;
    case 0xa9: Instr_XOR_R(R8::C); break;
    case 0xaa: Instr_XOR_R(R8::D); break;
    case 0xab: Instr_XOR_R(R8::E); break;
    case 0xac: Instr_XOR_R(R8::H); break;
    case 0xad: Instr_XOR_R(R8::L); break;
    case 0xae: Instr_XOR_MEM_HL(); break;
    case 0xee: Instr_XOR_N(); break;
    case 0x3f: Instr_CCF(); break;
    case 0x37: Instr_SCF(); break;
    case 0x27: Instr_DAA(); break;
    case 0x2f: Instr_CPL(); break;

    // 16-bit loads
    case 0x01: Instr_LD_RR_NN(R16::Bc); break;
    case 0x11: Instr_LD_RR_NN(R16::De); break;
    case 0x21: Instr_LD_RR_NN(R16::Hl); break;
    case 0x31: Instr_LD_RR_NN(R16::Sp); break;
    case 0x08: Instr_LD_MEM_NN_SP(); break;
    case 0xf8: Instr_LD_HL_SP_E(); break;
    case 0xf9: Instr_LD_SP_HL(); break;
    case 0xc5: Instr_PUSH_RR(R16::Bc); break;
    case 0xd5: Instr_PUSH_RR(R16::De); break;
    case 0xe5: Instr_PUSH_RR(R16::Hl); break;
    case 0xf5: Instr_PUSH_RR(R16::Af); break;
    case 0xc1: Instr_POP_RR(R16::Bc); break;
    case 0xd1: Instr_POP_RR(R16::De); break;
    case 0xe1: Instr_POP_RR(R16::Hl); break;
    case 0xf1: Instr_POP_RR(R16::Af); break;

    // 16-bit arithmetic
    case 0x09: Instr_ADD_HL_RR(R16::Bc); break;
    case 0x19: Instr_ADD_HL_RR(R16::De); break;
    case 0x29: Instr_ADD_HL_RR(R16::Hl); break;
    case 0x39: Instr_ADD_HL_RR(R16::Sp); break;
    case 0xe8: Instr_ADD_SP_E(); break;
    case 0x03: Instr_INC_RR(R16::Bc); break;
    case 0x13: Instr_INC_RR(R16::De); break;
    case 0x23: Instr_INC_RR(R16::Hl); break;
    case 0x33: Instr_INC_RR(R16::Sp); break;
    case 0x0b: Instr_DEC_RR(R16::Bc); break;
    case 0x1b: Instr_DEC_RR(R16::De); break;
    case 0x2b: Instr_DEC_RR(R16::Hl); break;
    case 0x3b: Instr_DEC_RR(R16::Sp); break;

    // Control flow
    case 0xc3: Instr_JP_NN(); break;
    case 0xe9: Instr_JP_HL(); break;
    case 0xc2: Instr_JP_CC_NN(Condition::NotZero); break;
    case 0xca: Instr_JP_CC_NN(Condition::Zero); break;
    case 0xd2: Instr_JP_CC_NN(Condition::NotCarry); break;
    case 0xda: Instr_JP_CC_NN(Condition::Carry); break;
    case 0x18: Instr_JR_E(); break;
    case 0x20: Instr_JR_CC_E(Condition::NotZero); break;
    case 0x28: Instr_JR_CC_E(Condition::Zero); break;
    case 0x30: Instr_JR_CC_E(Condition::NotCarry); break;
    case 0x38: Instr_JR_CC_E(Condition::Carry); break;
    case 0xcd: Instr_CALL_NN(); break;
    case 0xc4: Instr_CALL_CC_NN(Condition::NotZero); break;
    case 0xcc: Instr_CALL_CC_NN(Condition::Zero); break;
    case 0xd4: Instr_CALL_CC_NN(Condition::NotCarry); break;
    case 0xdc: Instr_CALL_CC_NN(Condition::Carry); break;
    case 0xc9: Instr_RET(); break;
    case 0xc0: Instr_RET_CC(Condition::NotZero); break;
    case 0xc8: Instr_RET_CC(Condition::Zero); break;
    case 0xd0: Instr_RET_CC(Condition::NotCarry); break;
    case 0xd8: Instr_RET_CC(Condition::Carry); break;
    case 0xd9: Instr_RETI(); break;
    case 0xc7: Instr_RST_N(0x00); break;
    case 0xcf: Instr_RST_N(0x08); break;
    case 0xd7: Instr_RST_N(0x10); break;
    case 0xdf: Instr_RST_N(0x18); break;
    case 0xe7: Instr_RST_N(0x20); break;
    case 0xef: Instr_RST_N(0x28); break;
    case 0xf7: Instr_RST_N(0x30); break;
    case 0xff: Instr_RST_N(0x38); break;

    // Miscellaneous
    case 0x76: Instr_HALT(); break;
    case 0x10: Instr_STOP(); break;
    case 0xf3: Instr_DI(); break;
    case 0xfb: Instr_EI(); break;
    case 0x00: Instr_NOP(); break;

    // Rotate accumulator
    case 0x07: Instr_RLCA(); break;
    case 0x0f: Instr_RRCA(); break;
    case 0x17: Instr_RLA(); break;
    case 0x1f: Instr_RRA(); break;

    // Unusable opcodes
    case 0xd3:
    case 0xdb:
    case 0xdd:
    case 0xe3:
    case 0xe4:
    case 0xeb:
    case 0xec:
    case 0xed:
    case 0xf4:
    case 0xfc:
    case 0xfd: return;

    // CB prefixed
    case 0xcb:
    {
        opcode = ReadOperand();
        LOG_TRACE("CPU: opcode 0xCB{:X}", opcode);
        switch (opcode)
        {
        case 0x07: Instr_RLC_R(R8::A); break;
        case 0x00: Instr_RLC_R(R8::B); break;
        case 0x01: Instr_RLC_R(R8::C); break;
        case 0x02: Instr_RLC_R(R8::D); break;
        case 0x03: Instr_RLC_R(R8::E); break;
        case 0x04: Instr_RLC_R(R8::H); break;
        case 0x05: Instr_RLC_R(R8::L); break;
        case 0x06: Instr_RLC_MEM_HL(); break;

        case 0x0f: Instr_RRC_R(R8::A); break;
        case 0x08: Instr_RRC_R(R8::B); break;
        case 0x09: Instr_RRC_R(R8::C); break;
        case 0x0a: Instr_RRC_R(R8::D); break;
        case 0x0b: Instr_RRC_R(R8::E); break;
        case 0x0c: Instr_RRC_R(R8::H); break;
        case 0x0d: Instr_RRC_R(R8::L); break;
        case 0x0e: Instr_RRC_MEM_HL(); break;

        case 0x17: Instr_RL_R(R8::A); break;
        case 0x10: Instr_RL_R(R8::B); break;
        case 0x11: Instr_RL_R(R8::C); break;
        case 0x12: Instr_RL_R(R8::D); break;
        case 0x13: Instr_RL_R(R8::E); break;
        case 0x14: Instr_RL_R(R8::H); break;
        case 0x15: Instr_RL_R(R8::L); break;
        case 0x16: Instr_RL_MEM_HL(); break;

        case 0x1f: Instr_RR_R(R8::A); break;
        case 0x18: Instr_RR_R(R8::B); break;
        case 0x19: Instr_RR_R(R8::C); break;
        case 0x1a: Instr_RR_R(R8::D); break;
        case 0x1b: Instr_RR_R(R8::E); break;
        case 0x1c: Instr_RR_R(R8::H); break;
        case 0x1d: Instr_RR_R(R8::L); break;
        case 0x1e: Instr_RR_MEM_HL(); break;

        case 0x27: Instr_SLA_R(R8::A); break;
        case 0x20: Instr_SLA_R(R8::B); break;
        case 0x21: Instr_SLA_R(R8::C); break;
        case 0x22: Instr_SLA_R(R8::D); break;
        case 0x23: Instr_SLA_R(R8::E); break;
        case 0x24: Instr_SLA_R(R8::H); break;
        case 0x25: Instr_SLA_R(R8::L); break;
        case 0x26: Instr_SLA_MEM_HL(); break;

        case 0x2f: Instr_SRA_R(R8::A); break;
        case 0x28: Instr_SRA_R(R8::B); break;
        case 0x29: Instr_SRA_R(R8::C); break;
        case 0x2a: Instr_SRA_R(R8::D); break;
        case 0x2b: Instr_SRA_R(R8::E); break;
        case 0x2c: Instr_SRA_R(R8::H); break;
        case 0x2d: Instr_SRA_R(R8::L); break;
        case 0x2e: Instr_SRA_MEM_HL(); break;

        case 0x37: Instr_SWAP_R(R8::A); break;
        case 0x30: Instr_SWAP_R(R8::B); break;
        case 0x31: Instr_SWAP_R(R8::C); break;
        case 0x32: Instr_SWAP_R(R8::D); break;
        case 0x33: Instr_SWAP_R(R8::E); break;
        case 0x34: Instr_SWAP_R(R8::H); break;
        case 0x35: Instr_SWAP_R(R8::L); break;
        case 0x36: Instr_SWAP_MEM_HL(); break;

        case 0x3f: Instr_SRL_R(R8::A); break;
        case 0x38: Instr_SRL_R(R8::B); break;
        case 0x39: Instr_SRL_R(R8::C); break;
        case 0x3a: Instr_SRL_R(R8::D); break;
        case 0x3b: Instr_SRL_R(R8::E); break;
        case 0x3c: Instr_SRL_R(R8::H); break;
        case 0x3d: Instr_SRL_R(R8::L); break;
        case 0x3e: Instr_SRL_MEM_HL(); break;

        case 0x47: Instr_BIT_B_R(0, R8::A); break;
        case 0x4f: Instr_BIT_B_R(1, R8::A); break;
        case 0x57: Instr_BIT_B_R(2, R8::A); break;
        case 0x5f: Instr_BIT_B_R(3, R8::A); break;
        case 0x67: Instr_BIT_B_R(4, R8::A); break;
        case 0x6f: Instr_BIT_B_R(5, R8::A); break;
        case 0x77: Instr_BIT_B_R(6, R8::A); break;
        case 0x7f: Instr_BIT_B_R(7, R8::A); break;
        case 0x40: Instr_BIT_B_R(0, R8::B); break;
        case 0x48: Instr_BIT_B_R(1, R8::B); break;
        case 0x50: Instr_BIT_B_R(2, R8::B); break;
        case 0x58: Instr_BIT_B_R(3, R8::B); break;
        case 0x60: Instr_BIT_B_R(4, R8::B); break;
        case 0x68: Instr_BIT_B_R(5, R8::B); break;
        case 0x70: Instr_BIT_B_R(6, R8::B); break;
        case 0x78: Instr_BIT_B_R(7, R8::B); break;
        case 0x41: Instr_BIT_B_R(0, R8::C); break;
        case 0x49: Instr_BIT_B_R(1, R8::C); break;
        case 0x51: Instr_BIT_B_R(2, R8::C); break;
        case 0x59: Instr_BIT_B_R(3, R8::C); break;
        case 0x61: Instr_BIT_B_R(4, R8::C); break;
        case 0x69: Instr_BIT_B_R(5, R8::C); break;
        case 0x71: Instr_BIT_B_R(6, R8::C); break;
        case 0x79: Instr_BIT_B_R(7, R8::C); break;
        case 0x42: Instr_BIT_B_R(0, R8::D); break;
        case 0x4a: Instr_BIT_B_R(1, R8::D); break;
        case 0x52: Instr_BIT_B_R(2, R8::D); break;
        case 0x5a: Instr_BIT_B_R(3, R8::D); break;
        case 0x62: Instr_BIT_B_R(4, R8::D); break;
        case 0x6a: Instr_BIT_B_R(5, R8::D); break;
        case 0x72: Instr_BIT_B_R(6, R8::D); break;
        case 0x7a: Instr_BIT_B_R(7, R8::D); break;
        case 0x43: Instr_BIT_B_R(0, R8::E); break;
        case 0x4b: Instr_BIT_B_R(1, R8::E); break;
        case 0x53: Instr_BIT_B_R(2, R8::E); break;
        case 0x5b: Instr_BIT_B_R(3, R8::E); break;
        case 0x63: Instr_BIT_B_R(4, R8::E); break;
        case 0x6b: Instr_BIT_B_R(5, R8::E); break;
        case 0x73: Instr_BIT_B_R(6, R8::E); break;
        case 0x7b: Instr_BIT_B_R(7, R8::E); break;
        case 0x44: Instr_BIT_B_R(0, R8::H); break;
        case 0x4c: Instr_BIT_B_R(1, R8::H); break;
        case 0x54: Instr_BIT_B_R(2, R8::H); break;
        case 0x5c: Instr_BIT_B_R(3, R8::H); break;
        case 0x64: Instr_BIT_B_R(4, R8::H); break;
        case 0x6c: Instr_BIT_B_R(5, R8::H); break;
        case 0x74: Instr_BIT_B_R(6, R8::H); break;
        case 0x7c: Instr_BIT_B_R(7, R8::H); break;
        case 0x45: Instr_BIT_B_R(0, R8::L); break;
        case 0x4d: Instr_BIT_B_R(1, R8::L); break;
        case 0x55: Instr_BIT_B_R(2, R8::L); break;
        case 0x5d: Instr_BIT_B_R(3, R8::L); break;
        case 0x65: Instr_BIT_B_R(4, R8::L); break;
        case 0x6d: Instr_BIT_B_R(5, R8::L); break;
        case 0x75: Instr_BIT_B_R(6, R8::L); break;
        case 0x7d: Instr_BIT_B_R(7, R8::L); break;
        case 0x46: Instr_BIT_B_MEM_HL(0); break;
        case 0x4e: Instr_BIT_B_MEM_HL(1); break;
        case 0x56: Instr_BIT_B_MEM_HL(2); break;
        case 0x5e: Instr_BIT_B_MEM_HL(3); break;
        case 0x66: Instr_BIT_B_MEM_HL(4); break;
        case 0x6e: Instr_BIT_B_MEM_HL(5); break;
        case 0x76: Instr_BIT_B_MEM_HL(6); break;
        case 0x7e: Instr_BIT_B_MEM_HL(7); break;

        case 0x87: Instr_RES_B_R(0, R8::A); break;
        case 0x8f: Instr_RES_B_R(1, R8::A); break;
        case 0x97: Instr_RES_B_R(2, R8::A); break;
        case 0x9f: Instr_RES_B_R(3, R8::A); break;
        case 0xa7: Instr_RES_B_R(4, R8::A); break;
        case 0xaf: Instr_RES_B_R(5, R8::A); break;
        case 0xb7: Instr_RES_B_R(6, R8::A); break;
        case 0xbf: Instr_RES_B_R(7, R8::A); break;
        case 0x80: Instr_RES_B_R(0, R8::B); break;
        case 0x88: Instr_RES_B_R(1, R8::B); break;
        case 0x90: Instr_RES_B_R(2, R8::B); break;
        case 0x98: Instr_RES_B_R(3, R8::B); break;
        case 0xa0: Instr_RES_B_R(4, R8::B); break;
        case 0xa8: Instr_RES_B_R(5, R8::B); break;
        case 0xb0: Instr_RES_B_R(6, R8::B); break;
        case 0xb8: Instr_RES_B_R(7, R8::B); break;
        case 0x81: Instr_RES_B_R(0, R8::C); break;
        case 0x89: Instr_RES_B_R(1, R8::C); break;
        case 0x91: Instr_RES_B_R(2, R8::C); break;
        case 0x99: Instr_RES_B_R(3, R8::C); break;
        case 0xa1: Instr_RES_B_R(4, R8::C); break;
        case 0xa9: Instr_RES_B_R(5, R8::C); break;
        case 0xb1: Instr_RES_B_R(6, R8::C); break;
        case 0xb9: Instr_RES_B_R(7, R8::C); break;
        case 0x82: Instr_RES_B_R(0, R8::D); break;
        case 0x8a: Instr_RES_B_R(1, R8::D); break;
        case 0x92: Instr_RES_B_R(2, R8::D); break;
        case 0x9a: Instr_RES_B_R(3, R8::D); break;
        case 0xa2: Instr_RES_B_R(4, R8::D); break;
        case 0xaa: Instr_RES_B_R(5, R8::D); break;
        case 0xb2: Instr_RES_B_R(6, R8::D); break;
        case 0xba: Instr_RES_B_R(7, R8::D); break;
        case 0x83: Instr_RES_B_R(0, R8::E); break;
        case 0x8b: Instr_RES_B_R(1, R8::E); break;
        case 0x93: Instr_RES_B_R(2, R8::E); break;
        case 0x9b: Instr_RES_B_R(3, R8::E); break;
        case 0xa3: Instr_RES_B_R(4, R8::E); break;
        case 0xab: Instr_RES_B_R(5, R8::E); break;
        case 0xb3: Instr_RES_B_R(6, R8::E); break;
        case 0xbb: Instr_RES_B_R(7, R8::E); break;
        case 0x84: Instr_RES_B_R(0, R8::H); break;
        case 0x8c: Instr_RES_B_R(1, R8::H); break;
        case 0x94: Instr_RES_B_R(2, R8::H); break;
        case 0x9c: Instr_RES_B_R(3, R8::H); break;
        case 0xa4: Instr_RES_B_R(4, R8::H); break;
        case 0xac: Instr_RES_B_R(5, R8::H); break;
        case 0xb4: Instr_RES_B_R(6, R8::H); break;
        case 0xbc: Instr_RES_B_R(7, R8::H); break;
        case 0x85: Instr_RES_B_R(0, R8::L); break;
        case 0x8d: Instr_RES_B_R(1, R8::L); break;
        case 0x95: Instr_RES_B_R(2, R8::L); break;
        case 0x9d: Instr_RES_B_R(3, R8::L); break;
        case 0xa5: Instr_RES_B_R(4, R8::L); break;
        case 0xad: Instr_RES_B_R(5, R8::L); break;
        case 0xb5: Instr_RES_B_R(6, R8::L); break;
        case 0xbd: Instr_RES_B_R(7, R8::L); break;
        case 0x86: Instr_RES_B_MEM_HL(0); break;
        case 0x8e: Instr_RES_B_MEM_HL(1); break;
        case 0x96: Instr_RES_B_MEM_HL(2); break;
        case 0x9e: Instr_RES_B_MEM_HL(3); break;
        case 0xa6: Instr_RES_B_MEM_HL(4); break;
        case 0xae: Instr_RES_B_MEM_HL(5); break;
        case 0xb6: Instr_RES_B_MEM_HL(6); break;
        case 0xbe: Instr_RES_B_MEM_HL(7); break;

        case 0xc7: Instr_SET_B_R(0, R8::A); break;
        case 0xcf: Instr_SET_B_R(1, R8::A); break;
        case 0xd7: Instr_SET_B_R(2, R8::A); break;
        case 0xdf: Instr_SET_B_R(3, R8::A); break;
        case 0xe7: Instr_SET_B_R(4, R8::A); break;
        case 0xef: Instr_SET_B_R(5, R8::A); break;
        case 0xf7: Instr_SET_B_R(6, R8::A); break;
        case 0xff: Instr_SET_B_R(7, R8::A); break;
        case 0xc0: Instr_SET_B_R(0, R8::B); break;
        case 0xc8: Instr_SET_B_R(1, R8::B); break;
        case 0xd0: Instr_SET_B_R(2, R8::B); break;
        case 0xd8: Instr_SET_B_R(3, R8::B); break;
        case 0xe0: Instr_SET_B_R(4, R8::B); break;
        case 0xe8: Instr_SET_B_R(5, R8::B); break;
        case 0xf0: Instr_SET_B_R(6, R8::B); break;
        case 0xf8: Instr_SET_B_R(7, R8::B); break;
        case 0xc1: Instr_SET_B_R(0, R8::C); break;
        case 0xc9: Instr_SET_B_R(1, R8::C); break;
        case 0xd1: Instr_SET_B_R(2, R8::C); break;
        case 0xd9: Instr_SET_B_R(3, R8::C); break;
        case 0xe1: Instr_SET_B_R(4, R8::C); break;
        case 0xe9: Instr_SET_B_R(5, R8::C); break;
        case 0xf1: Instr_SET_B_R(6, R8::C); break;
        case 0xf9: Instr_SET_B_R(7, R8::C); break;
        case 0xc2: Instr_SET_B_R(0, R8::D); break;
        case 0xca: Instr_SET_B_R(1, R8::D); break;
        case 0xd2: Instr_SET_B_R(2, R8::D); break;
        case 0xda: Instr_SET_B_R(3, R8::D); break;
        case 0xe2: Instr_SET_B_R(4, R8::D); break;
        case 0xea: Instr_SET_B_R(5, R8::D); break;
        case 0xf2: Instr_SET_B_R(6, R8::D); break;
        case 0xfa: Instr_SET_B_R(7, R8::D); break;
        case 0xc3: Instr_SET_B_R(0, R8::E); break;
        case 0xcb: Instr_SET_B_R(1, R8::E); break;
        case 0xd3: Instr_SET_B_R(2, R8::E); break;
        case 0xdb: Instr_SET_B_R(3, R8::E); break;
        case 0xe3: Instr_SET_B_R(4, R8::E); break;
        case 0xeb: Instr_SET_B_R(5, R8::E); break;
        case 0xf3: Instr_SET_B_R(6, R8::E); break;
        case 0xfb: Instr_SET_B_R(7, R8::E); break;
        case 0xc4: Instr_SET_B_R(0, R8::H); break;
        case 0xcc: Instr_SET_B_R(1, R8::H); break;
        case 0xd4: Instr_SET_B_R(2, R8::H); break;
        case 0xdc: Instr_SET_B_R(3, R8::H); break;
        case 0xe4: Instr_SET_B_R(4, R8::H); break;
        case 0xec: Instr_SET_B_R(5, R8::H); break;
        case 0xf4: Instr_SET_B_R(6, R8::H); break;
        case 0xfc: Instr_SET_B_R(7, R8::H); break;
        case 0xc5: Instr_SET_B_R(0, R8::L); break;
        case 0xcd: Instr_SET_B_R(1, R8::L); break;
        case 0xd5: Instr_SET_B_R(2, R8::L); break;
        case 0xdd: Instr_SET_B_R(3, R8::L); break;
        case 0xe5: Instr_SET_B_R(4, R8::L); break;
        case 0xed: Instr_SET_B_R(5, R8::L); break;
        case 0xf5: Instr_SET_B_R(6, R8::L); break;
        case 0xfd: Instr_SET_B_R(7, R8::L); break;
        case 0xc6: Instr_SET_B_MEM_HL(0); break;
        case 0xce: Instr_SET_B_MEM_HL(1); break;
        case 0xd6: Instr_SET_B_MEM_HL(2); break;
        case 0xde: Instr_SET_B_MEM_HL(3); break;
        case 0xe6: Instr_SET_B_MEM_HL(4); break;
        case 0xee: Instr_SET_B_MEM_HL(5); break;
        case 0xf6: Instr_SET_B_MEM_HL(6); break;
        case 0xfe: Instr_SET_B_MEM_HL(7); break;
        default: LOG_ERROR("CPU: Unknown prefixed opcode {:X}", opcode); break;
        }
        break;
    }
    default: LOG_ERROR("CPU: Unknown opcode {:X}", opcode);
    }
}

}  // namespace gb
