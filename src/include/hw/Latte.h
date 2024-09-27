#pragma once

#include <cstdint>
#include <hw/seeprom.h>
#include <hw/i2c.h>

class Latte
{
public:
    Latte() : seeprom(this) {}

    void Reset();
    void Update();

    uint32_t Read32(uint32_t addr);
    void Write32(uint32_t addr, uint32_t data);

    void SetGPIOBit(int bit, bool set);

    void TriggerInterruptLatte(int bit);
private:
    uint32_t timer;
    uint32_t alarm;
    uint32_t resets;
    uint32_t gpioIopIntStatus, gpioIopIntEn;
    uint32_t iopIntStatusAll, iopIntStatusLatte;
    uint32_t iopIrqIntEnAll, iopFiqIntEnAll, iopIrqIntEnLatte, iopFiqIntEnLatte;

    uint32_t abifOffset;
    uint32_t abifSelected;

    uint32_t eFuseOffset;
    uint32_t eFuseData;
    uint32_t eFuseAddr; // Raw value for readback

    uint8_t efuse[0x400];

    uint32_t gpioOwner;
    uint32_t gpioOut;
    uint32_t gpioEnable;
    uint32_t gpioDir;

    uint32_t exi0CSR;
    uint32_t exi0Data;

    bool rtcRegSelected = false;
    uint32_t rtcAddr;
    uint32_t rtcConfig0, rtcConfig1;

    SEEPROM seeprom;
    I2C i2c;
};