#include <cpu/ARM.h>
#include <util/debug.h>

void Starbuck::DoUmull(uint32_t instr)
{
    bool s = (instr >> 20) & 1;
    uint8_t rdHi = (instr >> 16) & 0xF;
    uint8_t rdLo = (instr >> 12) & 0xF;
    uint8_t rs = (instr >> 8) & 0xF;
    uint8_t rm = instr & 0xF;

    uint64_t res = (uint64_t)*(mRegs[rm]) * (uint64_t)*(mRegs[rs]);
    *(mRegs[rdHi]) = res >> 32;
    *(mRegs[rdLo]) = (uint32_t)res;

    if (s)
        SetSZFlags(res);

    if (CanDisassemble) printf("umull r%d,r%d,r%d,r%d\n", rdLo, rdHi, rm, rs);
}

void Starbuck::DoSmull(uint32_t instr)
{
    bool s = (instr >> 20) & 1;
    uint8_t rdHi = (instr >> 16) & 0xF;
    uint8_t rdLo = (instr >> 12) & 0xF;
    uint8_t rs = (instr >> 8) & 0xF;
    uint8_t rm = instr & 0xF;

    uint64_t res = (int64_t)(int32_t)*(mRegs[rm]) * (int64_t)(int32_t)*(mRegs[rs]);
    *(mRegs[rdHi]) = res >> 32;
    *(mRegs[rdLo]) = (uint32_t)res;

    if (s)
        SetSZFlags(res);

    if (CanDisassemble) printf("smull r%d,r%d,r%d,r%d\n", rdLo, rdHi, rm, rs);
}

void Starbuck::DoMul(uint32_t instr)
{
    bool s = (instr >> 20) & 1;
    uint8_t rd = (instr >> 16) & 0xF;
    uint8_t rs = (instr >> 8) & 0xF;
    uint8_t rm = instr & 0xF;

    uint32_t res = *(mRegs[rm]) * *(mRegs[rs]);
    *(mRegs[rd]) = res;

    if (s)
        SetSZFlags(res);

    if (CanDisassemble) printf("mul r%d,r%d,r%d\n", rd, rm, rs);
}