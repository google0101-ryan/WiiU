#include <cpu/ARM.h>
#include <util/debug.h>
#include <cassert>

void Starbuck::DoBranch(uint32_t instr)
{
    bool l = (instr >> 24) & 1;
    uint32_t offs = (instr & 0xFFFFFF) << 2;
    offs = (offs & 0x2000000) ? (offs | 0xFC000000) : offs;

    static bool firstTime = true;
    if (firstTime && (mPc + offs) == 0x0d410994)
    {
        firstTime = false;
        printf("Loading fw.img\n");
        mBus->LoadIOSU();

        uint32_t contextAddr = *(mRegs[0]);
        mBus->SWrite32(contextAddr+0x74, 0x01000000 + mBus->SRead32(contextAddr+0x60));

        *(mRegs[0]) = 0;

        return;
    }

    if (l)
        *(mRegs[14]) = mPc-4;
    
    mPc += offs;
    mPc &= ~1;
    mDidBranch = true;

    if (CanDisassemble) printf("b%s 0x%08x\n", l ? "l" : "", mPc);
}

void Starbuck::DoUndefined(uint32_t instr)
{
    uint32_t syscallNum = (instr >> 8) & 0xFF;

    printf("0x%08x: (udf): ", mPc);

    switch (syscallNum)
    {
    case 0x00:
        printf("IOS_CreateThread(0x%08x, 0x%08x, 0x%08x, 0x%08x, %d, 0x%08x)\n", *(mRegs[0]), *(mRegs[1]), *(mRegs[2]), *(mRegs[3]), *(mRegs[4]), *(mRegs[5]));
        break;
    case 0x05:
        printf("IOS_GetPid()\n");
        break;
    case 0x06:
        printf("IOS_GetProcessName(%d, 0x%08x)\n", *(mRegs[0]), *(mRegs[1]));
        break;
    case 0x07:
        printf("IOS_StartThread(%d)\n", *(mRegs[0]));
        break;
    case 0x0b:
        printf("IOS_SetThreadPriority(%d, %d)\n", *(mRegs[0]), *(mRegs[1]));
        break;
    case 0x0c:
        printf("IOS_CreateMessageQueue(0x%08x, %d)\n", *(mRegs[0]), *(mRegs[1]));
        break;
    case 0x11:
        printf("IOS_HandleEvent(%d, %d, 0x%08x)\n", *(mRegs[0]), *(mRegs[1]), *(mRegs[2]));
        break;
    case 0x13:
        printf("IOS_CreateTimer(%d, %d, %d, 0x%08x)\n", *(mRegs[0]), *(mRegs[1]), *(mRegs[2]), *(mRegs[3]));
        break;
    case 0x15:
        printf("IOS_StopTimer(%d)\n", *(mRegs[0]));
        break;
    case 0x25:
        printf("IOS_CreateCrossProcessHeap(%d)\n", *(mRegs[0]));
        break;
    case 0x2c:
    {
        uint32_t addr = *(mRegs[0]);
        char buf[1024];
        for (int i = 0; i < 1024; i++)
        {
            buf[i] = Read8(addr+i);
            if (buf[i] == 0)
                break;
        }
        printf("IOS_RegisterResourceManager(\"%s\", %d)\n", buf, *(mRegs[1]));
        break;
    }
    case 0x2d:
    {
        uint32_t addr = *(mRegs[0]);
        char buf[1024];
        for (int i = 0; i < 1024; i++)
        {
            buf[i] = Read8(addr+i);
            if (buf[i] == 0)
                break;
        }
        printf("IO_DeviceAssociate(0x%08x: \"%s\", %d)\n", *(mRegs[0]), buf, *(mRegs[1]));
        break;
    }
    case 0x57:
        printf("IOS_PushIob(0x%08x, %hi)\n", *(mRegs[0]), *(mRegs[1]));
        break;
    case 0x58:
        printf("IOS_PullIob(0x%08x, %hi)\n", *(mRegs[0]), *(mRegs[1]));
        break;
    case 0x59:
        printf("IOS_VerifyIob(0x%08x)\n", *(mRegs[0]));
        break;
    default:
        printf("Unknown syscall 0x%02x\n", syscallNum);
        throw std::runtime_error("UNKNOWN SYSCALL");
    }

    mUndRegs[1] = mPc-4;
    undSpsr.bits = cpsr.bits;
    cpsr.t = 0;
    cpsr.m = MODE_UND;
    SwitchMode(MODE_UND);
    cpsr.i = 1;

    mPc = 0xFFFF0004;

    mDidBranch = true;
    if (CanDisassemble) printf("udf #%02x\n", syscallNum);
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