#include <cpu/ARM.h>

bool IsDataProcessing(uint32_t instr)
{
    return ((instr >> 26) & 0b11) == 0b00;
}

bool IsSingleDataTransfer(uint32_t instr)
{
    return ((instr >> 26) & 0b11) == 0b01;
}

bool IsBranch(uint32_t instr)
{
    return ((instr >> 25) & 0b111) == 0b101;
}

bool IsBX(uint32_t instr)
{
    return ((instr >> 8) & 0b1111'1111'1111'1111'1111) ==
        0b0001'0010'1111'1111'1111;
}

bool IsLdmStm(uint32_t instr)
{
    return ((instr >> 25) & 0b111) == 0b100;
}

bool IsLdrhStrh(uint32_t instr)
{
    return ((instr >> 25) & 0b111) == 0b000
            && ((instr >> 7) & 1) == 0b1
            && ((instr >> 4) & 1) == 0b1;
}

bool IsMcrMrc(uint32_t instr)
{
    return ((instr >> 24) & 0b1111) == 0b1110;
}

bool IsMsrMrs(uint32_t instr)
{
    return ((instr >> 26) & 0b11) == 0b00
            && ((instr >> 23) & 0b11) == 0b10
            && ((instr >> 20) & 0b1) == 0b0;
}

bool IsUmull(uint32_t instr)
{
    return ((instr >> 21) & 0x7F) == 4
            && ((instr >> 4) & 0b1111) == 0b1001;
}

bool IsSmull(uint32_t instr)
{
    return ((instr >> 21) & 0x7F) == 0b0110
            && ((instr >> 4) & 0b1111) == 0b1001;
}

bool IsMul(uint32_t instr)
{
    return ((instr >> 21) & 0x7F) == 0
            && ((instr >> 4) & 0b1111) == 0b1001;
}

bool IsMla(uint32_t instr)
{
    return ((instr >> 25) & 0x7) == 0
            && ((instr >> 21) & 0xF) == 1
            && ((instr >> 4) & 0xF) == 0b1001;
}

bool IsUdf(uint32_t instr)
{
    return (instr & 0xFFFF00FF) == 0xE7F000F0;
}

bool IsCLZ(uint32_t instr)
{
    return ((instr >> 16) & 0xFFF) == 0x16F
        && ((instr >> 4) & 0xFF) == 0xF1;
}

void Starbuck::ExecuteInstruction(uint32_t instr)
{
    if (IsUdf(instr))
        DoUndefined(instr);
    else if (IsBranch(instr))
        DoBranch(instr);
    else if (IsSingleDataTransfer(instr))
        DoLdrStr(instr);
    else if (IsBX(instr))
        DoBX(instr);
    else if (IsLdmStm(instr))
        DoLdmStm(instr);
    else if (IsUmull(instr))
        DoUmull(instr);
    else if (IsSmull(instr))
        DoSmull(instr);
    else if (IsMul(instr))
        DoMul(instr);
    else if (IsMla(instr))
        DoMla(instr);
    else if (IsLdrhStrh(instr))
        DoLdrhStrh(instr);
    else if (IsMcrMrc(instr))
        DoMCRMRC(instr);
    else if (IsCLZ(instr))
        DoCLZ(instr);
    else if (IsMsrMrs(instr))
        DoPsrTransfer(instr);
    else if (IsDataProcessing(instr))
        DoDataProcessing(instr);
    else
    {
        printf("[STARBUCK]: Failed to execute unknown instruction 0x%08x\n", instr);
        throw std::runtime_error("UIMPLEMENTED INSTRUCTION!\n");
    }
}