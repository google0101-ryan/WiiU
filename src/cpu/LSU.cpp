#include <cpu/ARM.h>
#include <util/debug.h>
#include <cassert>
#include <vector>
#include <bit>

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
                printf("Unknown shift type %d, shamt=0\n", shtype);
                throw std::runtime_error("Unknown shift type");
            }
        }
        else
        {
            switch (shtype)
            {
            case 0:
                offs = *(mRegs[rm]) << is;
                break;
            case 1:
                offs = *(mRegs[rm]) >> is;
                break;
            default:
                printf("Unknown shift type %d, shamt=%d\n", shtype, is);
                throw std::runtime_error("Unknown shift type");
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
        Write32(addr & ~3, *(mRegs[rd]));
        if (CanDisassemble) printf("0x%08x (0x%08x)\n", *(mRegs[rd]), addr);
        break;
    }
    case 0b10:
    {
        // LDR
        *(mRegs[rd]) = std::rotr<uint32_t>(Read32(addr & ~3), (addr & 3) * 8);
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
    
    if (l && rd == 15)
    {
        mDidBranch = true;
        cpsr.t = mPc & 1;
        mPc &= ~1;
    }
}
const char *ldmStmAmod[] =
    {
        "da", "ia", "db", "ib"};

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

    if (CanDisassemble)
        printf("%s\n", DisasmLdmStm(p, u, s, w, l, rn, rlist).c_str());
    
    if (l)
        DoBlockLoad(instr);
    else
        DoBlockStore(instr);
}

void Starbuck::DoBlockLoad(uint32_t instr)
{
    bool p = (instr >> 24) & 1;
    bool u = (instr >> 23) & 1;
    bool s = (instr >> 22) & 1;
    bool w = (instr >> 21) & 1;
    uint8_t rn = (instr >> 16) & 0xF;
    uint16_t rlist = instr & 0xFFFF;
    bool change_cpsr = s && (rlist & (1 << 15));
    bool user_bank_transfer = s && !(rlist & (1 << 15));
    
    uint32_t address = *(mRegs[rn]);
    int offset;
    if (u)
        offset = 4;
    else
        offset = -4;
    
    uint32_t old_mode = cpsr.m;
    if (user_bank_transfer)
    {
        SwitchMode(MODE_USR);
        cpsr.m = MODE_USR;
    }

    int regs = 0;
    if (u)
    {
        for (int i = 0; i < 15; i++)
        {
            uint32_t bit = 1 << i;
            if (rlist & bit)
            {
                regs++;
                if (p)
                {
                    address += offset;
                    *(mRegs[i]) = Read32(address);
                }
                else
                {
                    *(mRegs[i]) = Read32(address);
                    address += offset;
                }
            }
        }
        if (rlist & (1 << 15))
        {
            if (p)
            {
                address += offset;
                mPc = Read32(address);
                cpsr.t = mPc & 1;
                mPc &= ~1;
                mDidBranch = true;
            }
            else
            {
                mPc = Read32(address);
                cpsr.t = mPc & 1;
                mPc &= ~1;
                address += offset;
                mDidBranch = true;
            }
        }
    }
    else
    {
        if (rlist & (1 << 15))
        {
            if (p)
            {
                address += offset;
                mPc = Read32(address);
                cpsr.t = mPc & 1;
                mPc & ~1;
            }
            else
            {
                mPc = Read32(address);
                cpsr.t = mPc & 1;
                mPc & ~1;
                address += offset;
            }
            regs++;
        }
        for (int i = 14; i >= 0; i--)
        {
            int bit = 1 << i;
            if (rlist & bit)
            {
                regs++;
                if (p)
                {
                    address += offset;
                    *(mRegs[i]) = Read32(address);
                }
                else
                {
                    *(mRegs[i]) = Read32(address);
                    address += offset;
                }
            }
        }
    }

    if (user_bank_transfer)
    {
        SwitchMode(old_mode);
        cpsr.m = old_mode;
    }

    if (w && !(rlist & (1 << rn)))
        *(mRegs[rn]) = address;
    
    if (change_cpsr)
    {
        cpsr = *curSpsr;
        SwitchMode(cpsr.m);
    }
}

void Starbuck::DoBlockStore(uint32_t instr)
{
    bool p = (instr >> 24) & 1;
    bool u = (instr >> 23) & 1;
    bool s = (instr >> 22) & 1;
    bool w = (instr >> 21) & 1;
    uint8_t rn = (instr >> 16) & 0xF;
    uint16_t rlist = instr & 0xFFFF;
    bool change_cpsr = s && (rlist & (1 << 15));
    bool user_bank_transfer = s && !(rlist & (1 << 15));
    
    uint32_t address = *(mRegs[rn]);
    int offset;
    if (u)
        offset = 4;
    else
        offset = -4;
    
    uint32_t old_mode = cpsr.m;
    if (user_bank_transfer)
    {
        SwitchMode(MODE_USR);
        cpsr.m = MODE_USR;
    }

    int regs = 0;
    if (u)
    {
        for (int i = 0; i < 16; i++)
        {
            int bit = 1 << i;
            if (rlist & bit)
            {
                regs++;
                if (p)
                {
                    address += offset;
                    Write32(address, *(mRegs[i]));
                }
                else
                {
                    Write32(address, *(mRegs[i]));
                    address += offset;
                }
            }
        }
    }
    else
    {
        for (int i = 15; i >= 0; i--)
        {
            int bit = 1 << i;
            if (rlist & bit)
            {
                regs++;
                if (p)
                {
                    address += offset;
                    Write32(address, *(mRegs[i]));
                }
                else
                {
                    Write32(address, *(mRegs[i]));
                    address += offset;
                }
            }
        }
    }

    if (user_bank_transfer)
    {
        SwitchMode(old_mode);
        cpsr.m = old_mode;
    }

    if (w)
        *(mRegs[rn]) = address;
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
            *(mRegs[rd]) = Read16(addr & ~1);
        else
            Write16(addr & ~1, *(mRegs[rd]));
        break;
    }
    case 2:
    {
        assert(l);
        if (l)
            *(mRegs[rd]) = (int32_t)(int8_t)Read8(addr);
        break;
    }
    case 3:
    {
        assert(l);
        if (l)
            *(mRegs[rd]) = (int32_t)(int16_t)Read16(addr & ~1);
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