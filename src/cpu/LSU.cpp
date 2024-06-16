#include <cpu/ARM.h>
#include <util/debug.h>
#include <cassert>
#include <vector>

std::string DoSingleDataTransferDisasm(bool l, bool b, bool w, bool p, uint32_t rd, std::string& part1, std::string& part2)
{
    std::string disasm = l ? "ldr" : "str";
    if (b)
        disasm += "b";
    disasm += " r" + std::to_string(rd);
    
    if (w)
        disasm += "!";
    disasm += ", [" + part1;
    if (!p)
        disasm += "], " + part2;
    else
        disasm += ", " + part2 + "]";
    
    return disasm;
}

void Starbuck::DoLdrStr(uint32_t instr)
{
    bool i = (instr >> 25) & 1;
    bool p = (instr >> 24) & 1;
    bool u = (instr >> 23) & 1;
    bool b = (instr >> 22) & 1;
    bool w = (instr >> 21) & 1;
    bool l = (instr >> 20) & 1;
    uint8_t rn = (instr >> 16) & 0xF;
    uint8_t rd = (instr >> 12) & 0xF;
    
    uint32_t addr = *(mRegs[rn]);
    std::string baseReg = "r" + std::to_string(rn);
    std::string addr2Disasm; // Second address part disasm (register or immediate)
    
    uint32_t offs = 0;
    
    if (i == 0)
    {
        offs = instr & 0xFFF;
        addr2Disasm = "#";
        if (!u) addr2Disasm += "-";
        addr2Disasm += std::to_string(offs);
    }
    else
    {
        uint8_t is = (instr >> 7) & 0x1F;
        uint8_t shtype = (instr >> 5) & 0x3;
        uint8_t rm = instr & 0xF;

        if (is == 0)
        {
            switch (shtype)
            {
            case 0:
                offs = *(mRegs[rm]);
                break;
            default:
                assert(0);
            }
        }
        else
        {
            switch (shtype)
            {
            case 0:
                offs = *(mRegs[rm]) << is;
                break;
            default:
                assert(0);
            }
        }
        addr2Disasm = "r" + std::to_string(rm);
        if (is != 0) addr2Disasm += ", #" + std::to_string(is);
    }

    std::string disasm = DoSingleDataTransferDisasm(l, b, w, p, rd, baseReg, addr2Disasm);
    if (CanDisassemble) printf("%s\n", disasm.c_str());
    
    if (p)
        addr = u ? (addr + offs) : (addr - offs);
    
    uint8_t opcode = (l << 1) | b;
    switch (opcode)
    {
    case 0b00:
    {
        // STR
        Write32(addr, *(mRegs[rd]));
        if (CanDisassemble) printf("0x%08x (0x%08x)\n", *(mRegs[rd]), addr);
        break;
    }
    case 0b10:
    {
        // LDR
        *(mRegs[rd]) = Read32(addr);
        if (CanDisassemble) printf("0x%08x (0x%08x)\n", *(mRegs[rd]), addr);
        break;
    }
    case 0b01:
    {
        // STRB
        Write8(addr, *(mRegs[rd]));
        break;
    }
    case 0b11:
    {
        // LDRB
        *(mRegs[rd]) = Read8(addr);
        break;
    }
    default:
        printf("TODO: Unknown combo l/b %d/%d\n", l, b);
        throw std::runtime_error("UNKNOWN LDR/LDRB/STR/STRB COMBO!");
    }

    if (!p)
        addr = u ? (addr + offs) : (addr - offs);

    if (!p || w)
        *(mRegs[rn]) = addr;
    
    mDidBranch = l && (rd == 15);
}

const char* ldmStmAmod[] =
{
    "da", "ia", "db", "ib"
};

std::string DisasmLdmStm(bool p, bool u, bool s, bool w, bool l, uint8_t rn, uint16_t rlist)
{
    std::string disasm = l ? "ldm" : "stm";

    std::vector<int> regs;
    for (int i = 0; i < 16; i++)
        if (rlist & (1 << i))
            regs.push_back(i);

    uint8_t amodIndex = (p << 1) | u;
    if (rn != 13 || (amodIndex != 2 && amodIndex != 1))
    {
        disasm += ldmStmAmod[amodIndex];
        disasm += " r" + std::to_string(rn);
        if (w)
            disasm += "!";
        disasm += ", ";
        if (rn < 10)
            disasm += " ";
        disasm += "{";
    }
    else
    {
        disasm = l ? "pop {" : "push {";
    }

    
    for (int i = 0; i < regs.size(); i++)
    {
        disasm += "r" + std::to_string(regs[i]);
        if (i != regs.size()-1)
            disasm += ", ";
    }
    disasm += "}";
    if (s)
        disasm += "^";

    return disasm;
}

void Starbuck::DoLdmStm(uint32_t instr)
{
    bool p = (instr >> 24) & 1;
    bool u = (instr >> 23) & 1;
    bool s = (instr >> 22) & 1;
    bool w = (instr >> 21) & 1;
    bool l = (instr >> 20) & 1;
    uint8_t rn = (instr >> 16) & 0xF;
    uint16_t rlist = instr & 0xFFFF;

    assert(rlist);

    uint32_t addr = *(mRegs[rn]);

    bool rnInReglist = false, rnLast;
    int regCount = 0;
    bool pcWasInList = (rlist & (1 << 15)) != 0;

    uint32_t oldMode = cpsr.m;

    if (s && (!l || !pcWasInList))
    {
        SwitchMode(MODE_USR);
    }

    auto doLdmStm = [&](int i) {
        if (rlist & (1 << i))
        {
            regCount++;
            rnLast = false;
            if (i == rn)
            {
                rnInReglist = true;
                rnLast = true;
            }
            
            if (p)
                addr = u ? (addr + 4) : (addr - 4);
            
            if (l)
                *(mRegs[i]) = Read32(addr);
            else
                Write32(addr, *(mRegs[i]));
            
            if (!p)
                addr = u ? (addr + 4) : (addr - 4);
        }
    };
    
    if (u)
    {
        for (int i = 0; i < 16; i++)
            doLdmStm(i);
    }
    else
    {
        for (int i = 15; i >= 0; i--)
            doLdmStm(i);
    }
    
    if (s)
    {
        if (l && pcWasInList)
        {
            cpsr.bits = curSpsr->bits;
            SwitchMode(cpsr.m);
        }
        else
            SwitchMode(oldMode);
    }

    if (w)
    {
        if (rnInReglist)
        {
            if (l)
            {
                if (!rnLast || regCount == 1)
                    *(mRegs[rn]) = addr;
            }
        }
        else
        {
            *(mRegs[rn]) = addr;
        }
    }
    
    if (CanDisassemble) printf("%s\n", DisasmLdmStm(p, u, s, w, l, rn, rlist).c_str());

    if (pcWasInList)
        mDidBranch = true;
}

std::string DisasmLdrhStrh(bool p, bool u, bool i, bool w, bool l, uint8_t rn, uint8_t rd, uint32_t imm, uint8_t op, uint32_t rmOrImm)
{
    std::string disasm;
    switch (op)
    {
    case 1:
        disasm = l ? "ldrh" : "strh";
        break;
    case 2:
        disasm = l ? "ldrsb" : "ldrd";
        break;
    case 3:
        disasm = l ? "ldrsh" : "strd";
        break;
    }

    disasm += " r" + std::to_string(rd);
    if (w)
        disasm += "!";
    
    disasm += ", [r" + std::to_string(rn);
    if (!p)
        disasm += "], ";
    else
        disasm += ", ";
    if (i)
    {
        uint32_t offs = (imm << 4) | rmOrImm;
        disasm += "#" + std::to_string(offs);
    }
    else
    {
        disasm += "r" + std::to_string(rmOrImm);
    }

    if (p)
        disasm += "]";
    
    return disasm;
}

void Starbuck::DoLdrhStrh(uint32_t instr)
{
    bool p = (instr >> 24) & 1;
    bool u = (instr >> 23) & 1;
    bool i = (instr >> 22) & 1;
    bool w = (instr >> 21) & 1;
    bool l = (instr >> 20) & 1;
    uint8_t rn = (instr >> 16) & 0xF;
    uint8_t rd = (instr >> 12) & 0xF;
    uint8_t upperImm = (instr >> 8) & 0xF;
    uint8_t op = (instr >> 5) & 0x3;

    uint8_t rmOrImm = instr & 0xF;

    uint32_t addr = *(mRegs[rn]);
    uint32_t offs;

    if (i)
    {
        offs = (upperImm << 4) | rmOrImm;
    }
    else
    {
        offs = *(mRegs[rmOrImm]);
    }

    if (p)
        addr = u ? (addr + offs) : (addr - offs);

    if (CanDisassemble) printf("%s\n", DisasmLdrhStrh(p, u, i, w, l, rn, rd, upperImm, op, rmOrImm).c_str());
    
    switch (op)
    {
    case 1:
    {
        if (l)
            *(mRegs[rd]) = Read16(addr);
        else
            Write16(addr, *(mRegs[rd]));
        break;
    }
    case 3:
    {
        assert(l);
        if (l)
            *(mRegs[rd]) = (int32_t)(int16_t)Read16(addr);
        break;
    }
    default:
        printf("Unknown ldrh/strh/ldrd/strd subopcode 0x%02x\n", op);
        throw std::runtime_error("UNKNOWN LDRH/STRH/LDRD/STRD SUBOPCODE!");
    }

    if (!p)
        addr = u ? (addr + offs) : (addr - offs);
    
    if (w || !p)
        *(mRegs[rn]) = addr;
    
    if (l && rd == 15)
        mDidBranch = true;
}