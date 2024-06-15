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
    else
    {
        printf("[STARBUCK]: Failed to execute unknown thumb instruction 0x%04x\n", instr);
        throw std::runtime_error("UIMPLEMENTED THUMB INSTRUCTION!\n");
    }
}