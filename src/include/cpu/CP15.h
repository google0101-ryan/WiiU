#pragma once

#include <cstdint>

class CP15
{
public:
    void Write(uint32_t cpopc, uint32_t cn, uint32_t cm, uint32_t cp, uint32_t data);
    uint32_t Read(uint32_t cpopc, uint32_t cn, uint32_t cm, uint32_t cp);
private:
    uint32_t control;
    uint32_t ttb0;
};