#pragma once

#include <cstdint>

class GX2
{
public:
    GX2();

    void Write32(uint32_t offset, uint32_t data);
    uint32_t Read32(uint32_t offset);
private:
    uint32_t d1graph_prim_surf, d1graph_secondary_surf = 0;
    uint32_t d1grph_update = 0;
    uint32_t d1ovl_update;
    uint32_t d1grph_control;
    int graphicsX, graphicsY;
    int graphicsW, graphicsH;
    int graphicsPitch;
    int graphicsXOff, graphicsYOff;
    bool d1grph_enabled = false;
    uint32_t lutAutofill;

    int GetDepthBpp();
};