#pragma once

#include <cstdint>

class Latte
{
public:
    void Reset();
    void Update();

    uint32_t Read32(uint32_t addr);
    void Write32(uint32_t addr, uint32_t data);
private:
    uint32_t timer;
    uint32_t alarm;
};