#include <cpu/ARM.h>
#include <util/debug.h>
#include <bit>
#include <cassert>

uint32_t DoShift(uint32_t shtype, uint32_t shamt, uint32_t op, bool doFlags, Starbuck::PSR* cpsr, std::string& disasm)
{
    // 0 is a special shift type. Handle it here
    if (shamt == 0)
    {
        switch (shtype)
        {
        case 0: // LSL
            return op;
        case 1:
            if (doFlags)
                cpsr->c = (op >> 31) & 1;
            return 0;
        default:
            printf("TODO: shtype %d, shamt=0\n", shtype);
            exit(1);
        }
    }
    else
    {
        switch (shtype)
        {
        case 0:
        {
            uint32_t data = op << shamt;
            if (doFlags)
                cpsr->c = (op & (1 << (32-shamt)));
            disasm += ", lsl #" + std::to_string(shamt);
            return data;
        }
        case 1:
        {
            uint32_t data = op >> shamt;
            if (doFlags)
                cpsr->c = (op & (1 << shamt));
            disasm += ", lsr #" + std::to_string(shamt);
            return data;
        }
        case 2:
        {
            uint32_t data = ((int32_t)op) >> shamt;
            if (doFlags)
                cpsr->c = (op & (1 << shamt));
            disasm += ", asr #" + std::to_string(shamt);
            return data;
        }
        default:
            printf("TODO: shtype %d, shamt=%d\n", shtype, shamt);
            exit(1);
        }
    }
}

void Starbuck::DoDataProcessing(uint32_t instr)
{
    bool i = (instr >> 25) & 1;
    uint8_t opcode = (instr >> 21) & 0xF;
    bool s = (instr >> 20) & 1;
    uint8_t rn = (instr >> 16) & 0xF;
    uint8_t rd = (instr >> 12) & 0xF;

    uint32_t op2;
    std::string op2_disasm;
    if (!i)
    {
        bool r = (instr >> 4) & 1;
        uint8_t rm = instr & 0xF;
        uint8_t shtype = (instr >> 5) & 0x3;

        op2 = *(mRegs[rm]);

        if (r)
        {
            uint8_t rs = (instr >> 8) & 0xF;
            uint8_t shamt = *(mRegs[rs]) & 0x1F;
            op2_disasm = "r" + std::to_string(rm);
            op2 = DoShift(shtype, shamt, op2, s, &cpsr, op2_disasm);
            op2_disasm = "r" + std::to_string(rm) + ", r" + std::to_string(rs);
        }
        else
        {
            uint32_t shamt = (instr >> 7) & 0x1F;
            op2_disasm = "r" + std::to_string(rm);
            op2 = DoShift(shtype, shamt, op2, s, &cpsr, op2_disasm);
        }
    }
    else
    {
        uint32_t imm = instr & 0xFF;
        uint32_t is = (instr >> 8) & 0xF;

        op2 = std::rotr<uint32_t>(imm, is*2);
        op2_disasm = "#" + std::to_string(imm);
        
            if (is != 0)
                op2_disasm += ", #" + std::to_string(is*2);
            op2_disasm += " (" + std::to_string(op2) + ")";
    }

    std::string op_disasm;

    switch (opcode)
    {
    case 0x0:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 & op2;
        *(mRegs[rd]) = result;
        op_disasm = std::string("and") + (s ? "s r" : " r") + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        
        if (s)
            SetSZFlags(result);
        break;
    }
    case 0x1:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 ^ op2;
        *(mRegs[rd]) = result;
        op_disasm = std::string("eor") + (s ? "s r" : " r") + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        
        if (s)
            SetSZFlags(result);
        break;
    }
    case 0x2:
    {
        uint32_t op1 = *(mRegs[rn]);
        *(mRegs[rd]) = op1 + ~op2 + 1;
        op_disasm = std::string("sub") + (s ? "s" : "") + " r" + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        
        if (CanDisassemble)
            printf("0x%08x\n", *(mRegs[rd]));

        if (s)
        {
            SetSZFlags(*(mRegs[rd]));
            SetCVFlags(op1, ~op2+1, *(mRegs[rd]));
        }
        break;
    }
    case 0x3:
    {
        uint32_t op1 = *(mRegs[rn]);
        *(mRegs[rd]) = op2 + ~op1 + 1;
        op_disasm = std::string("rsb") + (s ? "s" : "") + " r" + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        
        if (s)
        {
            SetSZFlags(*(mRegs[rd]));
            SetCVFlags(op2, ~op1+1, *(mRegs[rd]));
        }
        break;
    }
    case 0x4:
    {
        uint32_t op1 = *(mRegs[rn]);
        *(mRegs[rd]) = op1 + op2;
        op_disasm = std::string("add") + (s ? "s" : "") + " r" + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        if (CanDisassemble) printf("0x%08x\n", *(mRegs[rd]));
        
        if (s)
        {
            SetSZFlags(*(mRegs[rd]));
            SetCVFlags(op1, op2, *(mRegs[rd]));
        }
        break;
    }
    case 0x5:
    {
        uint32_t op1 = *(mRegs[rn]);
        *(mRegs[rd]) = op1 + op2 + cpsr.c;
        op_disasm = std::string("adc") + (s ? "s" : "") + " r" + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        if (CanDisassemble) printf("0x%08x\n", *(mRegs[rd]));
        
        if (s)
        {
            SetSZFlags(*(mRegs[rd]));
            SetCVFlags(op1, op2+cpsr.c, *(mRegs[rd]));
        }
        break;
    }
    case 0x7:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op2 + (~op1+1 + cpsr.c);
        *(mRegs[rd]) = result;
        op_disasm = "rsc r" + std::to_string(rd) + ", r" + std::to_string(rn) + op2_disasm;
        
        if (s)
        {
            SetSZFlags(result);
            assert(0);
        }
        break;
    }
    case 0x8:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 & op2;
        op_disasm = "tst r" + std::to_string(rn) + ", " + op2_disasm;
        
        assert(s);
        
        SetSZFlags(result);
        break;
    }
    case 0x9:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 ^ op2;
        op_disasm = "teq r" + std::to_string(rn) + ", " + op2_disasm;
        
        assert(s);
        
        SetSZFlags(result);
        break;
    }
    case 0xA:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 + ~op2 + 1;

        assert(s);

        op_disasm = "cmp r" + std::to_string(rn) + ", " + op2_disasm;
        if (CanDisassemble) printf("0x%08x, 0x%08x ", op1, op2);
        
        SetSZFlags(result);
        SetCVSubFlags(op1, op2, result);
        break;
    }
    case 0xB:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 + op2;

        assert(s);

        op_disasm = "cmn r" + std::to_string(rn) + ", " + op2_disasm;
        
        SetSZFlags(result);
        SetCVFlags(op1, op2, result);
        break;
    }
    case 0xC:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 | op2;
        *(mRegs[rd]) = result;

        op_disasm = std::string("orr") + (s ? "s" : "") + " r" + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        
        if (s)
        {
            SetSZFlags(result);
            SetCVSubFlags(op1, op2, result);
        }
        break;
    }
    case 0xD:
    {
        *(mRegs[rd]) = op2;
        op_disasm = std::string("mov") + (s ? "s" : "") + " r" + std::to_string(rd) + ", " + op2_disasm;

        if (s)
        {
            SetSZFlags(*(mRegs[rd]));
            if (rd == 15)
            {
                cpsr.bits = curSpsr->bits;
                cpsr.t = mPc & 1;
                mPc &= ~1;
                SwitchMode(cpsr.m);
            }
        }
        break;
    }
    case 0xE:
    {
        uint32_t op1 = *(mRegs[rn]);
        uint32_t result = op1 & ~op2;
        *(mRegs[rd]) = result;
        op_disasm = std::string("bic") + (s ? "s r" : " r") + std::to_string(rd) + ", r" + std::to_string(rn) + ", " + op2_disasm;
        
        if (s)
            SetSZFlags(result);
        break;
    }
    case 0xF:
    {
        *(mRegs[rd]) = ~op2;
        op_disasm = std::string("mvn") + (s ? "s" : "") + " r" + std::to_string(rd) + ", " + op2_disasm;

        if (s)
            SetSZFlags(*(mRegs[rd]));
        break;
    }
    default:
        printf("TODO: Unimplemented data processing sub-opcode %x\n", opcode);
        throw std::runtime_error("UNIMPLEMENTED DATA PROCESSING SUBOPCODE!\n");
    }

    if (rd == 15)
        mDidBranch = true;
    
    if (CanDisassemble) printf("%s\n", op_disasm.c_str());
}

void Starbuck::DoMCRMRC(uint32_t instr)
{
    bool isMrc = (instr >> 20) & 1;
    uint8_t cpopc = (instr >> 21) & 0x7;
    uint8_t cn = (instr >> 16) & 0xF;
    uint8_t rd = (instr >> 12) & 0xF;
    uint8_t pn = (instr >> 8) & 0xF;
    uint8_t cp = (instr >> 5) & 0x7;
    uint8_t cm = instr & 0xF;

    assert(pn == 15);

    if (isMrc)
    {
        *(mRegs[rd]) = cp15->Read(cpopc, cn, cm, cp);
        if (CanDisassemble) printf("mrc cp%d,%d,r%d,c%d,c%d,%d\n", pn, cpopc, rd, cn, cm, cp);
    }
    else
    {
        cp15->Write(cpopc, cn, cm, cp, *(mRegs[rd]));
        if (CanDisassemble) printf("mcr cp%d,%d,r%d,c%d,c%d,%d\n", pn, cpopc, rd, cn, cm, cp);
    }
}

void Starbuck::DoPsrTransfer(uint32_t instr)
{
    bool i = (instr >> 25) & 1;
    bool psr = (instr >> 22) & 1;
    bool isMsr = (instr >> 21) & 1;

    if (isMsr)
    {
        bool f = (instr >> 19) & 1;
        bool s = (instr >> 18) & 1;
        bool x = (instr >> 17) & 1;
        bool c = (instr >> 16) & 1;

        PSR* target;
        if (psr)
            target = curSpsr;
        else
            target = &cpsr;

        uint32_t source = target->bits;

        uint32_t op2;
        std::string op2Disasm;
        if (i)
        {
            uint32_t imm = instr & 0xFF;
            uint32_t shamt = (instr >> 8) & 0xF;
            op2 = std::rotr<uint32_t>(imm, shamt);
            op2Disasm = "#" + std::to_string(imm);
            if (shamt != 0) op2Disasm += ", lsl #" + std::to_string(shamt);
        }
        else
        {
            uint8_t rm = instr & 0xF;
            op2 = *(mRegs[rm]);
            op2Disasm = "r" + std::to_string(rm);
        }

        if (CanDisassemble) printf("msr %spsr_%s%s, %s (%d, %d)\n", psr ? "s" : "c", f ? "f" : "", c ? "c" : "", op2Disasm.c_str(), cpsr.m, op2 & 0x1F);

        uint32_t mask = 0;
        if (f)
            mask |= 0xFF000000;
        
        if (c)
            mask |= 0xFF;
        
        if (cpsr.m == MODE_USR)
            mask &= 0xFFFFFF00;
        
        if (!psr)
            mask &= 0xFFFFFFDF;
        
        source &= ~mask;
        source |= (op2 & mask);

        if (!psr)
        {
            uint32_t newMode = (op2 & 0x1F);
            SwitchMode(newMode);
        }
        target->bits = source;
    }
    else
    {
        PSR* target;
        if (psr)
            target = curSpsr;
        else
            target = &cpsr;

        uint8_t rd = (instr >> 12) & 0xF;
        *(mRegs[rd]) = target->bits;

        if (CanDisassemble) printf("mrs %spsr, r%d\n", psr ? "s" : "c", rd);
    }
}