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
            buf[i] = mBus->SRead8((*mRegs[1]) + i);
        logFile.write(buf, strlen(buf));
        logFile.flush();
        break;
    }
    }

    mSvcRegs[1] = mPc-2;
    svcSpsr.bits = cpsr.bits;
    cpsr.m = MODE_SVC;
    SwitchMode(MODE_SVC);
    cpsr.t = false;
    cpsr.i = cpsr.f = 1;

    mPc = 0xFFFF0008;
    mDidBranch = true;
    if (CanDisassemble) printf("swi 0x%02x\n", comment);
}