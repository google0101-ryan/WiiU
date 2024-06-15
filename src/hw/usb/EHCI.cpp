#include <hw/usb/EHCI.h>
#include <cstdio>
#include <stdexcept>

EHCIPort::EHCIPort()
{
    status = 0;
}

void EHCIPort::WriteStatus(uint32_t data)
{
    status = data;
}

uint32_t EHCIPort::ReadStatus()
{
    return status;
}

uint32_t EHCI::Read32(uint32_t addr)
{
    if (addr >= PORT_START && addr < PORT_END)
    {
        int port = (addr - PORT_START) / 4;
        return ports[port].ReadStatus();
    }

    switch (addr)
    {
    default:
        printf("Read from unknown EHCI controller address 0x%08x\n", addr);
        throw std::runtime_error("UNKNOWN EHCI ADDRESS!");
    }
}

void EHCI::Write32(uint32_t addr, uint32_t data)
{
    if (addr >= PORT_START && addr < PORT_END)
    {
        int port = (addr - PORT_START) / 4;
        return ports[port].WriteStatus(data);
    }

    switch (addr)
    {
    default:
        printf("Write to unknown EHCI controller address 0x%08x\n", addr);
        throw std::runtime_error("UNKNOWN EHCI ADDRESS!");
    }
}
