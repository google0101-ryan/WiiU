#include <hw/usb/OHCI.h>
#include <cstdio>
#include <stdexcept>

const char *OHCIStateNames[] = {
	"USBReset", "USBResume", "USBOperational", "USBSuspend"
};

int OHCI::curId = 0;

OHCI::OHCI()
{
    id = curId++;
}

void OHCI::Write32(uint32_t addr, uint32_t data)
{
    if (addr >= OHCI_PORT_START && addr < OHCI_PORT_END)
    {
        int port = (addr - OHCI_PORT_START) / 4;
        ports[port].status = data;
        return;
    }

    switch (addr)
    {
    case 4:
        control = data;
        printf("OHCI controller %d state set to %s\n", id, OHCIStateNames[(control >> 6) & 3]);
        break;
    default:
        printf("Write to unknown OHCI controller address 0x%08x\n", addr);
        throw std::runtime_error("UNKNOWN OHCI ADDRESS!");
    }
}