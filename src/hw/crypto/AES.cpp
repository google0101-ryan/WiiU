#include <hw/crypto/AES.h>

#include <stdio.h>
#include <stdexcept>
#include <cassert>

void AES::Write32(uint32_t addr, uint32_t data)
{
    switch (addr)
    {
    case 0x0d020000:
        aes_ctrl = data;
        assert(!(aes_ctrl & 1));
        break;
    default:
        printf("Write to unknown AES address 0x%08x\n", addr);
        throw std::runtime_error("WRITE TO UNKNOWN AES ADDRESS");
    }
}

uint32_t AES::Read32(uint32_t addr)
{
    switch (addr)
    {
    case 0x0d020000:
        return aes_ctrl;
    default:
        printf("Read from unknown AES address 0x%08x\n", addr);
        throw std::runtime_error("READ FROM UNKNOWN AES ADDRESS");
    }
}
