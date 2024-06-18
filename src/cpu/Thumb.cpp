#include <cpu/ARM.h>

bool IsHighRegOp(uint16_t instr)
{
    return ((instr >> 10) & 0x3F) == 0b10001;
}

bool IsAddSub(uint16_t instr)
{
    return ((instr >> 11) & 0x1F) == 0b11;
}

bool IsRegOpImm(uint16_t instr)
{
    return ((instr >> 13) & 0x7) == 0b001;
}

bool IsSWI(uint16_t instr)
{
    return ((instr >> 8) & 0xFF) == 0b11011111;
}

bool IsAddSp(uint16_t instr)
{
    return ((instr >> 8) & 0xFF) == 0b10110000;
}

bool IsPushPop(uint16_t instr)
{
    return ((instr >> 12) & 0xF) == 0b1011
            && ((instr >> 9) & 0x3) == 0b10;
}

bool IsGetRelAddr(uint16_t instr)
{
    return ((instr >> 12) & 0xF) == 0b1010;
}

bool IsSpRelLoadStore(uint16_t instr)
{
    return ((instr >> 12) & 0xF) == 0b1001;
}

bool IsPcRelLoad(uint16_t instr)
{
    return ((instr >> 11) & 0x1F) == 0b01001;
}

bool IsLongBranch1(uint16_t instr)
{
    return ((instr >> 11) & 0x1F) == 0b11110;
}

bool IsLongBranch2(uint16_t instr)
{
    return ((instr >> 11) & 0x1F) == 0b11111;
}

bool IsLdrStrImm(uint16_t instr)
{
    return ((instr >> 13) & 0x7) == 0b011;
}

bool IsBranchCond(uint16_t instr)
{
    return ((instr >> 12) & 0xF) == 0b1101;
}

void Starbuck::ExecuteThumbInstruction(uint16_t instr)
{
    if (IsHighRegOp(instr))
        DoHiRegOp(instr);
    else if (IsAddSub(instr))
        DoAddSub(instr);
    else if (IsRegOpImm(instr))
        DoRegOpImm(instr);
    else if (IsSWI(instr))
        DoSvc(instr);
    else if (IsAddSp(instr))
        DoAddSp(instr);
    else if (IsPushPop(instr))
        DoPushPop(instr);
    else if (IsGetRelAddr(instr))
        DoGetReladdr(instr);
    else if (IsSpRelLoadStore(instr))
        DoLoadStoreSpRel(instr);
    else if (IsPcRelLoad(instr))
        DoLoadPcRel(instr);
    else if (IsLongBranch1(instr))
        DoLongBL1(instr);
    else if (IsLongBranch2(instr))
        DoLongBL2(instr);
    else if (IsLdrStrImm(instr))
        DoLoadStoreImm(instr);
    else if (IsBranchCond(instr))
        DoCondBranch(instr);
    else
    {
        printf("[STARBUCK]: Failed to execute unknown thumb instruction 0x%04x\n", instr);
        throw std::runtime_error("UIMPLEMENTED THUMB INSTRUCTION!\n");
    }
}