#include <hw/Latte.h>
#include <system/WiiU.h>
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

    seeprom.LoadSEEPROM("seeprom.bin");
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
    case 0x0d8000dc:
        return gpioEnable;
    case 0x0d8000e0:
        return gpioOut & gpioOwner;
    case 0x0d8000e4:
        return gpioDir;
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
    case 0x0d800624:
        if (abifSelected == 0xc0)
            return gSystem->GetGX2()->Read32(abifOffset);
        else
            return 0;
    case 0x0d800470:
        return iopIntStatusAll;
    case 0x0d800474:
        return iopIntStatusLatte;
    case 0x0d80047c:
        return iopIrqIntEnLatte;;
    case 0x0d800480:
        return iopIrqIntEnAll;
    case 0x0d8000c0:
    case 0x0d8000c4:
    case 0x0d8000d4:
    case 0x0d8000e8:
    case 0x0d8000cc:
    case 0x0d8000ec:
    case 0x0d8000fc:
    case 0x0d800520 ... 0x0d80055c:
    case 0x0d8005bc:
        return 0;
    case 0x0d806800:
        return exi0CSR;
    case 0x0d8000f4:
        return gpioIopIntEn;
    case 0x0d800478:
        return iopIrqIntEnAll;
    case 0x0d806810:
        return exi0Data;
    case 0x0d800570:
    case 0x0d800578:
    case 0x0d800580:
    case 0x0d800584:
        return i2c.Read32(addr);
    case 0x0d800628:
    case 0x0d800190:
    case 0x0d8005b0:
    case 0x0d800250 ... 0x0d80025c:
        return 0;
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
    case 0x0d8000dc:
        gpioEnable = data;
        break;
    case 0x0d8000e0:
    {
        uint32_t mask = gpioDir & gpioEnable;
        bool foundLedChange = false;
        for (int i = 0; i < 32; i++)
        {
            if (mask & (1 << i))
            {
                if ((data & (1 << i)) != (gpioOut & (1 << i)))
                {
                    if (i >= 16 && i <= 23)
                    {
                        foundLedChange = true;
                        continue; // ignore LED
                    }
                    switch (i)
                    {
                    case 2:
                        break;
                    case 10:
                        seeprom.WriteCS(data & (1 << i));
                        break;
                    case 11:
                        if ((data >> i) & 1)
                            seeprom.Clock();
                        break;
                    case 12:
                        seeprom.WriteDO((data >> i) & 1);
                        break;
                    default:
                        printf("Write to unknown GPIO bit %d\n", i);
                        exit(1);
                    }
                }
            }
        }
        gpioOut = data;
        break;
    }
    case 0x0d8000e4:
        gpioDir = data;
        break;
    case 0x0d8000c0:
    case 0x0d8000c4:
    case 0x0d8000cc:
    case 0x0d8000d4:
    case 0x0d8000e8:
    case 0x0d8000ec:
    case 0x0d800050:
    case 0x0d800054:
    case 0x0d8004a4:
    case 0x0d8004a8:
    case 0x0d800520 ... 0x0d80055c:
    case 0x0d800250 ... 0x0d80025c:
        return;
    case 0x0d8005bc:
        TriggerInterruptLatte(12);
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
    case 0x0d8000fc:
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
    case 0x0d800620:
    {
        // printf("0x%08x -> ABI_OFFSET\n", data);
        abifSelected = (data >> 24) & 0xFF;
        abifOffset = data & 0xFFFFFF;
        break;
    }
    case 0x0d800624:
        if (abifSelected == 0xc0)
            gSystem->GetGX2()->Write32(abifOffset, data);
        break;
    case 0x0d800628:
        break;
    case 0x0d806800:
        printf("0x%08x -> EXI0_CSR\n", data);
        exi0CSR = data;
        break;
    case 0x0d80680C:
    {    
        printf("0x%08x -> EXI0_CR\n", data);
        bool tstart = data & 1;
        if (!tstart)
            break;
        if (rtcRegSelected)
        {
            bool write = ((data >> 2) & 3) == 1;

            if (write)
            {
                switch (rtcAddr)
                {
                case 0x21000C00:
                    rtcConfig0 = exi0Data;
                    break;
                case 0x21000D00:
                    rtcConfig1 = exi0Data;
                    break;
                default:
                    printf("Write to unknown RTC register 0x%08x\n", rtcAddr);
                    exit(1);
                }
            }
            else
            {
                switch (rtcAddr)
                {
                case 0x21000C00:
                    exi0Data = rtcConfig0;
                    break;
                case 0x21000D00:
                    exi0Data = rtcConfig1;
                    break;
                default:
                    printf("Read from unknown RTC register 0x%08x\n", rtcAddr);
                    exit(1);
                }
            }

            rtcRegSelected = false;
        }
        else
        {
            assert(((data >> 2) & 3) == 1); // Write mode

            rtcAddr = (exi0Data & ~0x80000000);
            rtcRegSelected = true;
            break;
        }
        break;
    }
    case 0x0d806810:
        printf("0x%08x -> EXI0_DATA\n", data);
        exi0Data = data;
        break;
    case 0x0d806814:
    case 0x0d806828:
    case 0x0d800190:
    case 0x0d8005ec:
    case 0x0d8005b0:
        break;
    case 0x0d800570:
    case 0x0d800574:
    case 0x0d800578:
    case 0x0d800580:
    case 0x0d800584:
        i2c.Write(addr, data);
        break;
    default:
        printf("Write32 to unknown addr 0x%08x\n", addr);
        throw std::runtime_error("WRITE TO UNKNOWN LATTE ADDRESS");
    }
}

void Latte::SetGPIOBit(int bit, bool set)
{
    gpioOut = set ? (gpioOut | (1 << bit)) : (gpioOut & ~(1 << bit));
}

void Latte::TriggerInterruptLatte(int bit)
{
    iopIntStatusLatte |= (1 << bit);
    printf("Triggering interrupt (%x, %x)\n", iopIntStatusLatte, iopIrqIntEnLatte);
    if (iopIntStatusLatte & iopIrqIntEnLatte)
    {
        printf("Triggered latte interrupt\n");
        gSystem->GetArmCPU()->DoInterrupt();
    }
}
