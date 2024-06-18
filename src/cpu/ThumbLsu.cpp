#include <cpu/ARM.h>
#include <stdexcept>
#include <stdio.h>
#include <bit>

void Starbuck::DoPushPop(uint16_t instr)
{
    uint8_t rlist = (uint8_t)instr;
    bool pcLr = (instr >> 8) & 1;
    bool l = (instr >> 11) & 1;

    uint32_t sp = *(mRegs[13]);

    if (l)
    {
        int regs = 0;
        for (int i = 0; i < 8; i++)
        {
            if (rlist & (1 << i))
            {
                regs++;
                *(mRegs[i]) = Read32(sp & ~3);
                sp += 4;
            }
        }

        if (pcLr)
        {
            mPc = Read32(sp & ~3);
            mDidBranch = true;
            regs++;
            sp += 4;

            cpsr.t = mPc & 1;
            mPc &= ~1;
        }

        *(mRegs[13]) = sp;
    }
    else
    {
        int regs = 0;

        if (pcLr)
        {
            sp -= 4;
            Write32(sp & ~3, *(mRegs[14]));
            regs++;
        }
        
        for (int i = 7; i >= 0; i--)
        {
            if (rlist & (1 << i))
            {
                regs++;
                sp -= 4;
                Write32(sp & ~3, *(mRegs[i]));
            }
        }

        *(mRegs[13]) = sp;
    }
}

void Starbuck::DoLoadStoreSpRel(uint16_t instr)
{
    uint32_t base = *(mRegs[13]);
    bool l = (instr >> 11) & 1;
    uint8_t rd = (instr >> 8) & 7;
    uint32_t nn = (instr & 0xFF) << 2;

    uint32_t addr = base+nn;

    if (l)
        *(mRegs[rd]) = std::rotr<uint32_t>(Read32(addr & ~3), (addr&3) * 8);
    else
        Write32(addr & ~3, *(mRegs[rd]));
    
    if (CanDisassemble)
        printf("%s r%d, [sp+#%d]\n", l ? "ldr" : "str", rd, nn);
}

void Starbuck::DoLoadPcRel(uint16_t instr)
{
    uint32_t base = mPc;
    uint8_t rd = (instr >> 8) & 7;
    uint32_t nn = (instr & 0xFF) << 2;

    uint32_t addr = base+nn;

    *(mRegs[rd]) = Read32(addr);
    
    if (CanDisassemble)
        printf("ldr r%d, [pc+#%d]\n", rd, nn);
}

void Starbuck::DoLoadStoreImm(uint16_t instr)
{
    uint8_t op = (instr >> 11) & 3;
    uint32_t nn = (instr >> 6) & 0x1F;
    uint8_t rb = (instr >> 3) & 7;
    uint8_t rd = instr & 7;

    uint32_t base = *(mRegs[rb]);

    switch (op)
    {
    case 0:
    {
        nn <<= 2;
        base += nn;

        Write32(base & ~3, *(mRegs[rd]));

        if (CanDisassemble) printf("str r%d, [r%d, #%d]\n", rd, rb, nn);

        break;
    }
    case 1:
    {
        nn <<= 2;
        base += nn;

        *(mRegs[rd]) = Read32(base & ~3);
        *(mRegs[rd]) = std::rotr<uint32_t>(*(mRegs[rd]), (base & 3) * 8);

        if (CanDisassemble) printf("ldr r%d, [r%d, #%d]\n", rd, rb, nn);

        break;
    }
    default:
        printf("Unknown thumb ldr/str op 0x%02x\n", op);
        throw std::runtime_error("UNKNOWN THUMB LDR/STR");
    }
}