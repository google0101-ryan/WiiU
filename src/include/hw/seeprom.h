#pragma once

#include <cstdint>

class Latte;

class SEEPROM
{
public:
    SEEPROM(Latte* pParent) : mParent(pParent) {}
    void LoadSEEPROM(const char* fname);

    void WriteCS(bool cs);
    void WriteDO(bool _do);

    void Clock();
private:
    uint16_t seeprom[256];

    void DoCommand();

    bool CS;
    bool DO;
    bool DI;

    int cycles = 11;
    uint32_t value = 0;

    enum
    {
        STATE_LISTEN,
        STATE_READ,
        STATE_DELAY,
    } state = STATE_LISTEN;

    Latte* mParent;
};