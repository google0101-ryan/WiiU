#pragma once

#include <cstdint>

class EHCIPort
{
public:
    EHCIPort();

    void WriteStatus(uint32_t data);
    uint32_t ReadStatus();
private:
    uint32_t status;
};

#define PORT_START 0x54
#define PORT_END 0x6C

class EHCI
{
public:
    uint32_t Read32(uint32_t addr);
    void Write32(uint32_t addr, uint32_t data);
private:
    EHCIPort ports[6];
};