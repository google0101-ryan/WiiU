#include <cpu/ARM.h>
#include <util/debug.h>
#include <cassert>

void Starbuck::DoBranch(uint32_t instr)
{
    bool l = (instr >> 24) & 1;
    uint32_t offs = (instr & 0xFFFFFF) << 2;
    offs = (offs & 0x2000000) ? (offs | 0xFC000000) : offs;

    if (l)
        *(mRegs[14]) = mPc-4;
    
    mPc += offs;
    mDidBranch = true;

    if (CanDisassemble) printf("b%s 0x%08x\n", l ? "l" : "", mPc);
}

void Starbuck::DoBX(uint32_t instr)
{
    uint8_t opcode = (instr >> 4) & 0xF;
    uint32_t rn = instr & 0xF;

    assert(opcode == 1 || opcode == 3);
    
    if (opcode == 3)
        *(mRegs[14]) = mPc-4;
    uint32_t target = *(mRegs[rn]);
    cpsr.t = target & 1;

    mPc = target & ~1;
    mDidBranch = true;

    if (CanDisassemble) printf("bx r%d\n", rn);
}