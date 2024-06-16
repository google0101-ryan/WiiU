#include <hw/Latte.h>
#include <cstdio>
#include <stdexcept>

void Latte::Reset()
{
    timer = 0;
    alarm = 0;
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
    default:
        printf("Read32 from unknown addr 0x%08x\n", addr);
        throw std::runtime_error("READ FROM UNKNOWN LATTE ADDRESS");
    }
}

void Latte::Write32(uint32_t addr, uint32_t data)
{
    switch (addr)
    {
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
    default:
        printf("Write32 to unknown addr 0x%08x\n", addr);
        throw std::runtime_error("WRITE TO UNKNOWN LATTE ADDRESS");
    }
}