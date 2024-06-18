#include <cpu/ARM.h>
#include <stdexcept>

void Starbuck::DoHiRegOp(uint16_t instr)
{
    uint8_t opcode = (instr >> 8) & 0x3;
    bool rdHi = (instr >> 7) & 1;
    bool rsHi = (instr >> 6) & 1;
    uint8_t rs = (instr >> 3) & 0x7;
    uint8_t rd = instr & 0x7;

    if (rdHi)
        rd += 8;
    if (rsHi)
        rs += 8;
    
    switch (opcode)
    {
    case 2:
        *(mRegs[rd]) = *(mRegs[rs]);
        if (CanDisassemble) printf("mov r%d,r%d (0x%08x)\n", rd, rs, *(mRegs[rd]));
        break;
    case 3:
    {
        if (rdHi)
            *(mRegs[14]) = (mPc-2) | 1;
        uint32_t target = *(mRegs[rs]);
        mPc = target & ~1;
        cpsr.t = target & 1;
        mDidBranch = true;
        if (CanDisassemble) printf("b%sx r%d\n", rdHi ? "l" : "", rs);
        break;
    }
    default:
        printf("Unknown high register op 0x%02x!\n", opcode);
        throw std::runtime_error("UNKNOWN HIGH REG OP!");
    }
}

void Starbuck::DoAddSub(uint16_t instr)
{
    uint8_t opcode = (instr >> 9) & 0x3;
    uint8_t rnOrNn = (instr >> 6) & 0x7;
    uint8_t rs = (instr >> 3) & 0x7;
    uint8_t rd = instr & 0x7;

    uint32_t op1 = *(mRegs[rs]);
    uint32_t op2 = *(mRegs[rnOrNn]);
    if (opcode == 2 || opcode == 3)
        op2 = rnOrNn;

    switch (opcode)
    {
    case 2:
        *(mRegs[rd]) = op1 + op2;
        SetSZFlags(*(mRegs[rd]));
        SetCVFlags(op1, op2, *(mRegs[rd]));
        if (CanDisassemble) printf("add r%d,r%d,#%d\n", rd, rs, rnOrNn);
        break;
    default:
        printf("Unknown add/sub op 0x%02x!\n", opcode);
        throw std::runtime_error("UNKNOWN ADD/SUB OP!");
    }
}

void Starbuck::DoAddSp(uint16_t instr)
{
    int nn = (instr & 0x7F)*4;
    bool d = (instr >> 11) & 1;
    if (d)
        nn = -nn;
    
    *(mRegs[13]) += nn;

    if (CanDisassemble)
        printf("add sp, #%d\n", nn);
}

void Starbuck::DoRegOpImm(uint16_t instr)
{
    uint8_t opcode = (instr >> 11) & 0x3;
    uint8_t rs = (instr >> 8) & 0x7;
    uint8_t nn = instr & 0xF;

    switch (opcode)
    {
    case 0:
        *(mRegs[rs]) = nn;
        SetSZFlags(*(mRegs[rs]));
        if (CanDisassemble) printf("mov r%d,#%d\n", rs, nn);
        break;
    case 1:
    {
        uint32_t result = *(mRegs[rs]) - nn;
        SetSZFlags(result);
        SetCVSubFlags(*(mRegs[rs]), nn, result);
        if (CanDisassemble) printf("cmp r%d,#%d\n", rs, nn);
        break;
    }
    default:
        printf("Unknown mov/cmp/add/sub imm op 0x%02x!\n", opcode);
        throw std::runtime_error("UNKNOWN MOV/CMP/ADD/SUB OP!");
    }
}

void Starbuck::DoGetReladdr(uint16_t instr)
{
    bool isSp = (instr >> 11) & 1;
    uint8_t rd = (instr >> 8) & 0x7;
    uint32_t nn = (instr & 0xFF) << 2;

    uint32_t base;
    if (isSp)
        base = *(mRegs[13]);
    else
        base = mPc;
    
    *(mRegs[rd]) = base+nn;
}