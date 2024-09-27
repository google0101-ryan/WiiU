#include <cpu/ARM.h>
#include <util/debug.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>

Starbuck* gDebugStarbuck;

Starbuck::Starbuck(Bus* pBus)
: mBus(pBus)
{
    // PC is always 0x8 bytes ahead, due to pipelining
    // This puts us firmly in MEM1, where IOSU loader is loaded
    // We add STARBUCK_KERNEL_START_OFFS to skip over the Ancast header (0x300 bytes)
    mPc = 0xD400218;

    // ARM starts up in SVC mode by default
    SwitchMode(MODE_SVC);
    cpsr.m = MODE_SVC;

    mDidBranch = false;

    cp15 = new CP15();

    gDebugStarbuck = this;
}

void Starbuck::DoInterrupt()
{
    if (cpsr.i == 0)
    {
        mIrqRegs[1] = (cpsr.t ? mPc-2 : mPc-4);
        irqSpsr.bits = cpsr.bits;
        cpsr.t = 0;
        cpsr.m = MODE_IRQ;
        SwitchMode(MODE_IRQ);
        cpsr.i = 1;
        mPc = 0xFFFF0018 + 8;
        printf("STARBUCK: Entered interrupt\n");
        waitForInterrupt = false;
        interruptPending = false;
    }
    else
        interruptPending = true;
}

void Starbuck::Tick()
{
    // if ((mPc - 8) == 0xd414b98)
    //     CanDisassemble = true;

    if (interruptPending)
        DoInterrupt();
    else if (waitForInterrupt)
        return;

    if (cpsr.t)
    {
        uint16_t opcode = Read16(mPc-4);

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
        uint32_t opcode = Read32(mPc-8);

        if (opcode == 0)
            throw std::runtime_error("Probably a bug: NULL opcode!");

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
    printf("[%s%s%s%s%d]\n",
            cpsr.c ? "c" : ".",
            cpsr.z ? "z" : ".",
            cpsr.v ? "v" : ".",
            cpsr.n ? "n" : ".",
            cpsr.m);
}

void Starbuck::SwitchMode(uint32_t mode)
{
    // Assign all of the default registers (FIQ mode overwrites some of these)
    for (int i = 0; i < 13; i++)
        mRegs[i] = &mNormalRegs[i];
    
    // R15 points to PC in all modes
    mRegs[15] = &mPc;

    if (CanDisassemble) printf("Switching to mode 0x%02x\n", mode);

    switch (mode)
    {
    case MODE_SVC:
        mRegs[13] = &mSvcRegs[0]; // SP
        mRegs[14] = &mSvcRegs[1]; // LR
        curSpsr = &svcSpsr;
        break;
    case MODE_SYS:
    case MODE_USR:
        mRegs[13] = &mUsrRegs[0]; // SP
        mRegs[14] = &mUsrRegs[1]; // LR
        curSpsr = NULL;
        break;
    case MODE_FIQ:
        for (int i = 0; i < 7; i++)
            mRegs[i+8] = &mFiqRegs[i];
        curSpsr = &fiqSpsr;
        break;
    case MODE_ABT:
        mRegs[13] = &mAbtRegs[0]; // SP
        mRegs[14] = &mAbtRegs[1]; // LR
        curSpsr = &abtSpsr;
        break;
    case MODE_IRQ:
        mRegs[13] = &mIrqRegs[0]; // SP
        mRegs[14] = &mIrqRegs[1]; // LR
        curSpsr = &irqSpsr;
        break;
    case MODE_UND:
        mRegs[13] = &mUndRegs[0]; // SP
        mRegs[14] = &mUndRegs[1]; // LR
        curSpsr = &undSpsr;
        break;
    default:
        printf("Failed to switch to unknown mode %d\n", (int)mode);
        throw std::runtime_error("SWITCH TO UNKNOWN MODE!\n");
    }
}

bool CheckPermissions(bool isWrite, bool svc, int ap)
{
    if (ap == 0) return false;
    else
    {
        if (svc) return true;

        if (ap == 1) return false;
        if (ap == 2) return !isWrite;
        return true;
    }
}

uint32_t Starbuck::TranslateAddr(uint32_t addr, bool isWrite)
{
    if (!cp15->IsMMUEnabled())
        return addr;
    
    uint32_t firstLevelOffs = (addr >> 20) * 4;
    uint32_t firstLevelAddr = cp15->GetPTBase() + firstLevelOffs;
    uint32_t firstLevelDesc = mBus->SRead32(firstLevelAddr);

    int firstLevelType = firstLevelDesc & 3;
    if (firstLevelType == 0)
    {
        printf("TODO: Signal page fault for address 0x%08x (0x%08x, 0x%08x)!\n", addr, firstLevelDesc, *(mRegs[14]));
        exit(1);
    }
    else if (firstLevelType == 1)
    {
        uint32_t secondLevelBase = firstLevelDesc & ~0x3FF;
		uint32_t secondLevelOffs = ((addr >> 12) & 0xFF) * 4;
		uint32_t secondLevelAddr = secondLevelBase + secondLevelOffs;
		uint32_t secondLevelDesc = mBus->SRead32(secondLevelAddr);

        int secondLevelType = secondLevelDesc & 3;
        assert(secondLevelType == 2);
		if (secondLevelType == 0) 
        {
            printf("TODO: Signal page fault for second level! (0x%08x, 0x%08x)\n", secondLevelDesc, addr);
            throw std::runtime_error("2ND LEVEL PAGE FAULT!\n");
		}
        uint32_t pageBase = secondLevelDesc & ~0xFFF;
        addr = pageBase + (addr & 0xFFF);
    }
    else if (firstLevelType == 2)
    {
        uint32_t sectionBase = firstLevelDesc & ~0xFFFFF;
        addr = sectionBase + (addr & 0xFFFFF);
    }
    return addr;
}

uint32_t Starbuck::Read32(uint32_t addr)
{
    if (addr == 0xe6000e18)
        CanDisassemble = true;
    addr = TranslateAddr(addr, false);

    return mBus->SRead32(addr);
}

uint16_t Starbuck::Read16(uint32_t addr)
{
    addr = TranslateAddr(addr, false);

    return mBus->SRead16(addr);
}

uint8_t Starbuck::Read8(uint32_t addr)
{
    addr = TranslateAddr(addr, false);

    return mBus->SRead8(addr);
}

void Starbuck::Write32(uint32_t addr, uint32_t data)
{
    addr = TranslateAddr(addr, true);

    mBus->SWrite32(addr, data);
}

void Starbuck::Write16(uint32_t addr, uint16_t data)
{
    addr = TranslateAddr(addr, true);

    mBus->SWrite16(addr, data);
}

void Starbuck::Write8(uint32_t addr, uint8_t data)
{
    addr = TranslateAddr(addr, true);

    mBus->SWrite8(addr, data);
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