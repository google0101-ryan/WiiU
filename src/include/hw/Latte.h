#pragma once

#include <cstdint>

class Latte
{
public:
    void Reset();
    void Update();

    uint32_t Read32(uint32_t addr);
    void Write32(uint32_t addr, uint32_t data);
private:
    uint32_t timer;
    uint32_t alarm;
    uint32_t resets;
    uint32_t gpioIopIntStatus, gpioIopIntEn;
    uint32_t iopIntStatusAll, iopIntStatusLatte;
    bool iopIrqIntEnAll, iopFiqIntEnAll, iopIrqIntEnLatte, iopFiqIntEnLatte;

    uint32_t eFuseOffset;
    uint32_t eFuseData;
    uint32_t eFuseAddr; // Raw value for readback

    uint8_t efuse[0x400];

    uint32_t gpioOwner;
    uint32_t gpioOut;
    uint32_t gpioEnable;
    uint32_t gpioDir;
};