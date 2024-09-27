#include <gpu/GX2.h>

#include <cstdio>
#include <stdexcept>

GX2::GX2()
{
    d1graph_prim_surf = 0;
}

const char* GetFormatName(int format, int depthBpp)
{
    if (depthBpp == 0)
    {
        return "indexed";
    }
    else if (depthBpp == 1)
    {
        switch (format)
        {
        case 0:
            return "ARGB1555";
        case 1:
            return "RGB565";
        case 2:
            return "ARGB4444";
        case 3:
            return "AlphaIndex88";
        case 4:
            return "Monochrome16";
        case 5:
            return "BGRA5551";
        }
    }
    else if (depthBpp == 2)
    {
        switch (format)
        {
        case 0:
            return "ARGB8888";
        case 1:
            return "ARGB2101010";
        case 2:
            return "32bpp digital";
        case 3:
            return "8-bit ARGB2101010";
        case 4:
            return "BGRA1010102";
        case 5:
            return "8-bit BGRA1010102";
        case 6:
            return "RGB 111110";
        case 7:
            return "BGR 101111";
        }
    }
    else
    {
        switch (format)
        {
        case 0:
            return "ARGB161616";
        }
    }

    return "<INVALID>";
}

void GX2::Write32(uint32_t offset, uint32_t data)
{
    switch (offset)
    {
        // PLL bullshit
    case 0x898:
    case 0x58E8:
    case 0x5900:
    case 0x5930:
    case 0x5938:
    case 0x60E0:
    case 0x6114:
    case 0x611C:
    case 0x6140:
    case 0x6528:
    case 0x6580 ... 0x658c:
    case 0xF4A8:
    case 0xF4B0:
    case 0x6300 ... 0x63fc:
    case 0x438 ... 0x580:
        return;
    case 0x6100:
        printf("%d -> D1GRPH_ENABLE\n", data);
        d1grph_enabled = data & 1;
        break;
    case 0x6104:
    {
        d1grph_control = data;
        printf("D1GRPH_CONTROL updated:\n");
        printf("\t%d bpp\n", GetDepthBpp());
        printf("\tFormat: %s\n", GetFormatName((data >> 8) & 0x7, data & 3));
        break;
    }
    case 0x610C:
        printf("0x%08x -> D1GRPH_SWAP_CONTROL\n", data);
        break;
    case 0x6110:
        printf("0x%08x -> D1GRPH_SURF_ADDR\n", data);
        d1graph_prim_surf = (data & ~0xFF);
        break;
    case 0x6118:
        printf("0x%08x -> D1GRPH_SECONDARY_SURF_ADDR\n", data);
        d1graph_secondary_surf = data & ~0xFF;
        break;
    case 0x6120:
        printf("%d -> D1GRAPH_PITCH\n", data);
        graphicsPitch = data;
        break;
    case 0x6124:
        printf("%d -> D1GRAPH_SURFACE_OFFSET_X\n", data);
        graphicsXOff = data;
        break;
    case 0x6128:
        printf("%d -> D1GRAPH_SURFACE_OFFSET_Y\n", data);
        graphicsYOff = data;
        break;
    case 0x612C:
        printf("0x%08x -> D1GRAPH_X_START\n", data);
        graphicsX = data;
        break;
    case 0x6130:
        printf("0x%08x -> D1GRAPH_Y_START\n", data);
        graphicsY = data;
        break;
    case 0x6134:
        printf("%d -> D1GRAPH_X_END\n", data);
        graphicsW = data;
        break;
    case 0x6138:
        printf("%d -> D1GRAPH_Y_END\n", data);
        graphicsH = data;
        break;
    case 0x6144:
        d1grph_update = data;
        return;
    case 0x6180:
        printf("%d -> D1OVL_ENABLE\n", data);
        break;
    case 0x61AC:
        d1ovl_update = data;
        return;
    case 0x64A0:
        if (data & 1)
            lutAutofill = 2;
        break;
    default:
        printf("GX2: Write32 to unknown addr 0x%08x (0x%08x)\n", offset, data);
        throw std::runtime_error("WRITE TO UNKNOWN GX2 ADDRESS");
    }
}

uint32_t GX2::Read32(uint32_t offset)
{
    switch (offset)
    {
    case 0x898:
    case 0x58E8:
    case 0x5900:
    case 0x5930:
    case 0x5938:
    case 0x6528:
    case 0xF4A8:
    case 0xF4B0:
    case 0x60E0:
    case 0x6114:
    case 0x611C:
    case 0x6140:
    case 0x6300 ... 0x63fc:
    case 0x6580 ... 0x658c:
    case 0x438 ... 0x580:
        return 0;
    case 0x6100:
        return d1grph_enabled;
    case 0x6104:
        return d1grph_control;
    case 0x610C:
        return 0;
    case 0x6110:
        return d1graph_prim_surf;
    case 0x6118:
        return d1graph_secondary_surf;
    case 0x6120:
        return graphicsPitch;
    case 0x6124:
        return graphicsXOff;
    case 0x6128:
        return graphicsYOff;
    case 0x612C:
        return graphicsX;
    case 0x6130:
        return graphicsY;
    case 0x6134:
        return graphicsW;
    case 0x6138:
        return graphicsH;
    case 0x6144:
        return d1grph_update;
    case 0x6180:
        return 0;
    case 0x61AC:
        return d1ovl_update;
    case 0x64A0:
        return lutAutofill;
    default:
        printf("GX2: Read from unknown addr 0x%08x\n", offset);
        throw std::runtime_error("READ FROM UNKNOWN GX2 ADDRESS");
    }
}

int GX2::GetDepthBpp()
{
    switch (d1grph_control & 3)
    {
    case 0:
        return 8;
    case 1:
        return 16;
    case 2:
        return 32;
    case 3:
        return 64;
    }
}
