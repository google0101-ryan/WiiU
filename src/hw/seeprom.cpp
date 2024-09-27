#include <hw/seeprom.h>
#include <hw/Latte.h>

#include <fstream>
#include <cassert>

void SEEPROM::LoadSEEPROM(const char* fname)
{
    std::ifstream file(fname, std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    assert(size == 256*2);
    file.seekg(0, std::ios::beg);
    file.read((char*)seeprom, size);
    file.close();

    cycles = 11;
}

void SEEPROM::WriteCS(bool cs)
{
    CS = cs;
}

void SEEPROM::WriteDO(bool _do)
{
    DO = _do;
}

void SEEPROM::Clock()
{
    if (state == STATE_LISTEN)
    {
        value = (value << 1) | DO;
        if (--cycles == 0)
            DoCommand();
    }
    else if (state == STATE_READ)
    {
        mParent->SetGPIOBit(12, (value >> --cycles) & 1);
        if (cycles == 0)
        {
            cycles = 2;
            state = STATE_DELAY;
        }
    }
    else if (state == STATE_DELAY)
    {
        if (--cycles == 0)
        {
            cycles = 11;
            value = 0;
            state = STATE_LISTEN;
        }
    }
}

void SEEPROM::DoCommand()
{
    int command = value >> 8;
    int param = value & 0xFF;

    switch (command)
    {
    case 6:
        cycles = 16;
        state = STATE_READ;
        value = __builtin_bswap16(seeprom[param]);
        printf("SEEPROM: Read from 0x%02x\n", param);
        break;
    default:
        printf("Unknown SEEPROM command %d\n", value);
        exit(1);
    }
}
