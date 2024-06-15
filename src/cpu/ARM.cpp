#include <cpu/ARM.h>
#include <util/debug.h>
#include <cstdio>
#include <cstdlib>

Starbuck::Starbuck(Bus* pBus)
: mBus(pBus)
{
    // PC is always 0x8 bytes ahead, due to pipelining
    // This puts us firmly in MEM1, where IOSU loader is loaded
    // We add STARBUCK_KERNEL_START_OFFS to skip over the Ancast header (0x300 bytes)
    mPc = STARBUCK_KERNEL_START_OFFS + 0x8;

    // ARM starts up in SVC mode by default
    SwitchMode(MODE_SVC);
    cpsr.m = MODE_SVC;

    mDidBranch = false;

    cp15 = new CP15();
}

void Starbuck::Tick()
{
    if (cpsr.t)
    {
        uint16_t opcode = mBus->SRead16(mPc-4);

        if (CanDisassemble) printf("0x%04x (0x%08x): ", opcode, mPc-4);

        ExecuteThumbInstruction(opcode);
        
        mPc += cpsr.t ? 2 : 4;

        // On branches, we simulate a pipeline flush by adding an extra 4 to PC
        if (mDidBranch)
        {
            mDidBranch = false;
            mPc += cpsr.t ? 2 : 4;
        }
    }
    else
    {
        uint32_t opcode = mBus->SRead32(mPc-8);

        if (opcode == 0)
            throw std::runtime_error("Probably a bug: NULL opcode!");
        
        if (mPc-8 == 0x8122500)
            CanDisassemble = true;

        if (CanDisassemble) printf("0x%08x (0x%08x): ", opcode, mPc-8);

        if (CondPassed((opcode >> 28) & 0xF))
            ExecuteInstruction(opcode);
        else
        {
            if (CanDisassemble) printf("skipped!\n");
        }
        
        mPc += cpsr.t ? 2 : 4;

        // On branches, we simulate a pipeline flush by adding an extra 4 to PC
        if (mDidBranch)
        {
            mDidBranch = false;
            mPc += cpsr.t ? 2 : 4;
        }
    }
}

void Starbuck::DumpState()
{
    for (int i = 0; i < 16; i++)
        printf("r%d  \t->\t0x%08x\n", i, *(mRegs[i]));
    printf("pc  \t->\t0x%08x\n", cpsr.t ? mPc-4 : mPc-8);
    printf("[%s%s%s%s]\n",
            cpsr.c ? "c" : ".",
            cpsr.z ? "z" : ".",
            cpsr.v ? "v" : ".",
            cpsr.n ? "n" : ".");
}

void Starbuck::SwitchMode(uint32_t mode)
{
    // Assign all of the default registers (FIQ mode overwrites some of these)
    for (int i = 0; i < 13; i++)
        mRegs[i] = &mNormalRegs[i];
    
    // R15 points to PC in all modes
    mRegs[15] = &mPc;

    switch (mode)
    {
    case MODE_SVC:
        mRegs[13] = &mSvcRegs[0]; // SP
        mRegs[14] = &mSvcRegs[1]; // LR
        curSpsr = &svcSpsr;
        break;
    case MODE_USR:
        mRegs[13] = &mUsrRegs[0]; // SP
        mRegs[14] = &mUsrRegs[1]; // LR
        break;
    case MODE_FIQ:
        for (int i = 0; i < 7; i++)
            mRegs[i+8] = &mFiqRegs[i];
        break;
    case MODE_ABT:
        mRegs[13] = &mAbtRegs[0]; // SP
        mRegs[14] = &mAbtRegs[1]; // LR
        break;
    case MODE_IRQ:
        mRegs[13] = &mIrqRegs[0]; // SP
        mRegs[14] = &mIrqRegs[1]; // LR
        break;
    case MODE_UND:
        mRegs[13] = &mUndRegs[0]; // SP
        mRegs[14] = &mUndRegs[1]; // LR
        break;
    default:
        printf("Failed to switch to unknown mode %d\n", (int)mode);
        throw std::runtime_error("SWITCH TO UNKNOWN MODE!\n");
    }
}


bool Starbuck::CondPassed(uint8_t cond)
{
    switch (cond)
    {
    case 0x0:
        return cpsr.z;
    case 0x1:
        return !cpsr.z;
    case 0x2:
        return cpsr.c;
    case 0x3:
        return !cpsr.c;
    case 0x4:
        return cpsr.n;
    case 0x5:
        return !cpsr.n;
    case 0x6:
        return cpsr.v;
    case 0x7:
        return !cpsr.v;
    case 0x8:
        return cpsr.c && !cpsr.z;
    case 0x9:
        return !cpsr.c || cpsr.z;
    case 0xA:
        return cpsr.n == cpsr.v;
    case 0xB:
        return cpsr.n != cpsr.v;
    case 0xC:
        return !cpsr.z && (cpsr.n == cpsr.v);
    case 0xD:
        return cpsr.z || (cpsr.n != cpsr.v);
    case 0xE:
        return true;
    default:
        printf("Unknown condition code 0x%01x\n", cond);
        throw std::runtime_error("UNKNOWN CONDITION CODE!\n");
    }
}