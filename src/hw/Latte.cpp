#include <hw/Latte.h>
#include <cstdio>
#include <stdexcept>

#include <fstream>
#include <cassert>

void Latte::Reset()
{
    timer = 0;
    alarm = 0;

    gpioOwner = gpioOut = gpioEnable = 0;
    resets = 0;

    std::ifstream file("otp.bin", std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    assert(size == 0x400);
    file.seekg(0, std::ios::beg);
    file.read((char*)efuse, 0x400);
    file.close();
}

void Latte::Update()
{
    timer++;

    if (timer == alarm)
    {
        printf("TODO: TIMER ALARM");
        throw std::runtime_error("TIMER ALARM");
    }
}

uint32_t Latte::Read32(uint32_t addr)
{
    switch (addr)
    {
    case 0x0d800010:
        return timer;
    case 0x0d8000e0:
        return gpioOut & gpioOwner;
    case 0x0d8001EC:
        return eFuseAddr;
    case 0x0d8001f0:
        return eFuseData;
    case 0x0d800214:
        return 0x21;
    case 0x0d8005a0:
        return 0xCAFE0060;
    case 0x0d8005ec:
        return resets;
    default:
        printf("Read32 from unknown addr 0x%08x\n", addr);
        throw std::runtime_error("READ FROM UNKNOWN LATTE ADDRESS");
    }
}

void Latte::Write32(uint32_t addr, uint32_t data)
{
    switch (addr)
    {
    case 0x0d800010:
        timer = data;
        return;
    case 0x0d8000e0:
    {
        uint32_t mask = gpioDir & gpioEnable & gpioOwner;
        for (int i = 0; i < 32; i++)
        {
            if (mask & (1 << i))
            {
                if ((data & (1 << i)) != (gpioOut & (1 << i)))
                {
                    printf("TODO: GPIO write\n");
                    exit(1);
                }
            }
        }
        gpioOut = (gpioOut & ~gpioOwner) | (data & gpioOwner);
        break;
    }
    case 0x0d800050:
    case 0x0d800054:
    case 0x0d8004a4:
    case 0x0d8004a8:
        return;
    case 0x0d800014:
        alarm = data;
        return;
    case 0x0d8000f0:
        printf("0x%08x -> HW_GPIOIOPINTSTS\n", data);
        gpioIopIntStatus &= ~data;
        return;
    case 0x0d8000f4:
        printf("0x%08x -> HW_GPIOIOPINTEN\n", data);
        gpioIopIntEn = data;
        return;
    case 0x0d800470:
        printf("0x%08x -> LT_IOPINTSTSALL\n", data);
        iopIntStatusAll &= ~data;
        return;
    case 0x0d800474:
        printf("0x%08x -> LT_IOPINTSTSLATTE\n", data);
        iopIntStatusLatte &= ~data;
        return;
    case 0x0d800478:
        printf("0x%08x -> LT_IOPIRQINTENALL\n", data);
        iopIrqIntEnAll = data;
        return;
    case 0x0d80047C:
        printf("0x%08x -> LT_IOPIRQINTENLATTE\n", data);
        iopIrqIntEnLatte = data;
        return;
    case 0x0d800480:
        printf("0x%08x -> LT_IOPFIQINTENALL\n", data);
        iopFiqIntEnAll = data;
        return;
    case 0x0d800484:
        printf("0x%08x -> LT_IOPFIQINTENLATTE\n", data);
        iopFiqIntEnLatte = data;
        return;
    case 0x0d8001ec:
    {
        eFuseAddr = data;
        bool isRead = (data >> 31) & 1;
        uint8_t bank = (data >> 8) & 0x7;
        uint8_t addr = data & 0x1F;
        eFuseOffset = (bank*32*4)+(addr*4);
        printf("0x%08x -> HW_EFUSEADDR (0x%08x, %s)\n", data, eFuseOffset, isRead ? "read" : "write");
        if (isRead)
            eFuseData = __builtin_bswap32(*(uint32_t*)&efuse[eFuseOffset]);
        break;
    }
    default:
        printf("Write32 to unknown addr 0x%08x\n", addr);
        throw std::runtime_error("WRITE TO UNKNOWN LATTE ADDRESS");
    }
}