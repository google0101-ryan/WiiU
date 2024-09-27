#pragma once

#include <cstdint>

class AES
{
public:
    void Write32(uint32_t addr, uint32_t data);
    uint32_t Read32(uint32_t addr);
private:
    uint32_t aes_ctrl;
};