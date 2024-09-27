#include <hw/i2c.h>

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cassert>

std::vector<uint8_t> dataBuf;

void ProcessSMC()
{
    bool read = dataBuf[0] & 1;
    int slave = dataBuf[0] >> 1;

    assert(slave == 0x50);

    if (read)
    {
        assert(0);
    }
    else
    {
        if (dataBuf.size() == 2)
        {
            uint8_t cmd = dataBuf[1];
            switch (cmd)
            {
            case 0:
                printf("SMC: ODD power on\n");
                break;
            default:
                printf("Unknown SMC cmd %d\n", cmd);
                exit(1);
            }
        }
        else if (dataBuf.size() == 3)
        {
            uint8_t cmd = dataBuf[1];
            uint8_t value = dataBuf[2];
            switch (cmd)
            {
            case 0x44:
                printf("NOTIFICATION_LED = 0x%x\n", value);
                break;
            default:
                printf("Unknown SMC cmd %x (with data %x)\n", cmd, value);
                exit(1);
            }
        }
        else
        {
            printf("Invalid databuf size %d\n", dataBuf.size());
            exit(1);
        }
    }
}

void I2C::Write(uint32_t addr, uint32_t data)
{
    switch (addr)
    {
    case 0x0d800570:
        printf("0x%08x -> LT_I2CMCTRL\n", data);
        break;
    case 0x0d800574:
        printf("0x%08x -> LT_I2CMDATA\n", data);
        smcData = data;
        break;
    case 0x0d800578:
        printf("0x%08x -> LT_I2CMWREN\n", data);
        if (data & 1)
        {
            dataBuf.push_back(smcData);
            if (smcData & 0x100)
            {
                ProcessSMC();
                dataBuf.clear();
                smcIntStatus |= (1 << 1);
            }
        }
        break;
    case 0x0d800580:
        printf("0x%08x -> LT_I2CIOPINTEN\n", data);
        smcIntEn = data;
        break;
    case 0x0d800584:
        printf("0x%08x -> LT_I2CIOPINTSTS\n", data);
        smcIntStatus &= ~data;
        break;
    default:
        printf("Write to unknown I2C register 0x%08x\n", addr);
        exit(1);
    }
}

uint32_t I2C::Read32(uint32_t addr)
{
    switch (addr)
    {
    case 0x0d800580:
        return smcIntEn;
    case 0x0d800584:
        return smcIntStatus;
    default:
        printf("Read from unknown I2C register 0x%08x\n", addr);
        exit(1);
    }
}
