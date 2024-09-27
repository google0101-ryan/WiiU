#pragma once

#include <cstdint>

class I2C
{
public:
    void Write(uint32_t addr, uint32_t data);
    uint32_t Read32(uint32_t addr);
private:
    uint32_t smcIntEn, smcIntStatus;
    uint32_t smcData;

    bool smcDeviceSelected;
};