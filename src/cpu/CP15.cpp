#include <cpu/CP15.h>
#include <cpu/ARM.h>

#include <cstdio>
#include <stdexcept>

void CP15::Write(uint32_t cpopc, uint32_t cn, uint32_t cm, uint32_t cp, uint32_t data)
{
    uint32_t addr = (cpopc << 11) | (cn << 7) | (cm << 3) | cp;

    switch (addr)
    {
    case 0x80:
    {
        control = data;
        return;
    }
    case 0x180: // Domain access permission
    case 0x280: // Instruction fault status
    case 0x281: // Data fault status
    case 0x300: // Fault address
    case 0x3A8: // Data cache invalidation
    case 0x3B0: // Instruction cache invalidation
    case 0x3D1: // Data cache invalidation by MVA
    case 0x3D3:
    case 0x3D4: // Drain write buffer
    case 0x438: // Invalidate TLB
        return;
    case 0x100:
        printf("0x%08x -> Translation Table Base 0\n", data);
        ttb0 = data;
        return;
    default:
        printf("Write to unknown CP15 register %d,c%d,c%d,%d (0x%08x)\n", cpopc, cn, cm, cp, addr);
        throw std::runtime_error("UNKNOWN CP15 REGISTER!");
    }
}

uint32_t CP15::Read(Starbuck* core, uint32_t cpopc, uint32_t cn, uint32_t cm, uint32_t cp)
{
    uint32_t addr = (cpopc << 11) | (cn << 7) | (cm << 3) | cp;
    switch (addr)
    {
    case 0x80:
        return control;
    case 0x3D3:
        return core->cpsr.z;
    default:
        printf("Read from unknown CP15 register %d,c%d,c%d,%d (0x%08x)\n", cpopc, cn, cm, cp, addr);
        throw std::runtime_error("UNKNOWN CP15 REGISTER!");
    }
}

bool CP15::IsMMUEnabled()
{
    return control & 1;
}
