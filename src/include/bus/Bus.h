#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>
#include <bus/MemoryMap.h>
#include <hw/usb/OHCI.h>
#include <hw/usb/EHCI.h>
#include <hw/Latte.h>
#include <hw/sdhc.h>

// Prefixes:
// E = Espresso (PPC core)
// S = Starbuck (ARM core)
class Bus
{
public:
    Bus(std::string starbuckKernel);
    void Update();

    void LoadIOSU();

    void Dump();

    uint32_t SRead32(uint32_t addr);
    uint16_t SRead16(uint32_t addr);
    uint8_t SRead8(uint32_t addr);

    void SWrite32(uint32_t addr, uint32_t data);
    void SWrite16(uint32_t addr, uint16_t data);
    void SWrite8(uint32_t addr, uint8_t data);
private:
    uint8_t* mem0;
    uint8_t* mem1;
    uint8_t* mem2;
    uint8_t* sram0;

    // These are, in order, 0:0, 0:1, 1:0, 2:0
    OHCI* ochi[4];
    EHCI* ehci[3];
    Latte* latte;
    SDHC* sdio0;
};