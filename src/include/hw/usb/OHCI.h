#pragma once

#include <cstdint>

class OHCIPort
{
public:
    uint32_t status;
};

#define OHCI_PORT_START 0x54
#define OHCI_PORT_END 0x64

// USB OHCI controller
class OHCI
{
public:
    OHCI();

    void Write32(uint32_t addr, uint32_t data);
private:
    uint32_t control;
    int id;
    static int curId;

    OHCIPort ports[4];
};