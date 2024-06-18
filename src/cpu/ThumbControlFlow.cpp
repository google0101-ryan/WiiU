#include <cpu/ARM.h>
#include <fstream>
#include <string.h>

std::ofstream logFile;

void Starbuck::DoSvc(uint16_t instr)
{
    uint8_t comment = instr & 0xFF;

    switch ((*mRegs[0]))
    {
    case 0x4:
    {
        if (!logFile.is_open())
            logFile.open("log.txt");
        char buf[1024];
        for (int i = 0; i < 1024; i++)
        {
            buf[i] = Read8((*mRegs[1]) + i);
            if (buf[i] == 0)
                break;
        }
        logFile.write(buf, strlen(buf));
        logFile.flush();
        break;
    }
    }

    mSvcRegs[1] = (mPc-2) | 1;
    svcSpsr.bits = cpsr.bits;
    cpsr.m = MODE_SVC;
    SwitchMode(MODE_SVC);
    cpsr.t = false;
    cpsr.i = cpsr.f = 1;

    mPc = 0xFFFF0008;
    mDidBranch = true;
    if (CanDisassemble) printf("swi 0x%02x\n", comment);
}

void Starbuck::DoCondBranch(uint16_t instr)
{
    uint8_t cond = (instr >> 8) & 0xF;

    if (!CondPassed(cond))
    {
        if (CanDisassemble) printf("Passed\n");
        return;
    }
    
    int16_t offset = static_cast<int32_t>(instr << 24) >> 23;
    mPc += offset;
    mDidBranch = true;

    if (CanDisassemble) printf("b 0x%08x\n", mPc);
}

void Starbuck::DoLongBL1(uint16_t instr)
{
    int32_t offset = ((instr & 0x7FF) << 21) >> 9;
    offset += mPc;

    *(mRegs[14]) = offset;

    if (CanDisassemble) printf("bl prep\n");
}

void Starbuck::DoLongBL2(uint16_t instr)
{
    uint32_t addr = *(mRegs[14]);
    addr += (instr & 0x7FF) << 1;

    uint32_t newLr = mPc-2;
    newLr |= 1;

    *(mRegs[14]) = newLr;

    mPc = addr;
    mDidBranch =  true;

    if (CanDisassemble)
        printf("bl 0x%08x\n", addr);
}