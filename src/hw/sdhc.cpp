#include <hw/sdhc.h>
#include <stdio.h>
#include <cstdlib>
#include <stdexcept>

#define printf(x, ...) do {0;} while (0)

SDHC::SDHC()
    : csd()
{
    Reset();

    csd.csd_structure = 1;
    csd.taac = 0xE;
    csd.nsac = 0;
	csd.tran_speed = 0x32; // 25 Mbit/s
	csd.ccc = 0x5B5;
	csd.read_bl_len = 0x9; // 512 bytes
	csd.read_bl_partial = 0;
	csd.write_blk_misalign = 0;
	csd.read_blk_misalign = 0;
	csd.dsr_imp = 0;
	csd.csize_hi = 0;
	csd.csize_lo = 0;
	csd.erase_blk_enable = 0x1;
	csd.sector_size = 0x7F; // 128 blocks
	csd.wp_grp_size = 0;
	csd.wp_grp_enable = 0;
	csd.r2w_factor = 0x2; // x4
	csd.write_bl_len = 0x9; // 512 bytes
	csd.write_grp_enable = 0;
	csd.file_format_grp = 0;
	csd.copy = 0;
	csd.perm_write_protect = 0;
	csd.twp_write_protect = 0;
	csd.file_format = 0;
	csd.crc = 0x01;

    std::ifstream file("sdcard.img", std::ios::ate | std::ios::binary);
    size_t size = file.tellg();
    file.seekg(std::ios::beg);
    fileData = new uint8_t[size];
    file.read((char*)fileData, size);
    file.close();
}

uint8_t mode_status[64] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint32_t SDHC::Read32(uint32_t addr)
{
    switch (addr)
    {
    case 0x04:
        return block_size | (block_count << 16);
    case 0x10:
        return result0;
    case 0x14:
        return result1;
    case 0x18:
        return result2;
    case 0x1C:
        return result3;
    case 0x20:
    {
        uint32_t data = *(uint32_t*)transferSrc;
        transferSrc += 4;
        return data;
    }
    case 0x24:
        return 0x30000 | (1 << 11);
    case 0x28:
        return control;
    case 0x2c:
        printf("Reading from CLOCK_CTL\n");
        return clock_control | (timeout_control << 16);
    case 0x30:
        return int_status;
    case 0x34:
    case 0x38:
        return 0;
    case 0x40:
        return capabilities >> 32;
    case 0xfc:
        return 0;
    default:
        printf("Read from unknown SDIO address 0x%08x\n", addr);
        exit(1);
    }
}

void SDHC::DoCommand(int index)
{
    switch (index)
    {
    case 0:
        printf("SDHC_GO_IDLE\n");
        state = STATE_IDLE;
        break;
    case 2:
        printf("SDHC_ALL_SEND_CID\n");
        break;
    case 3:
        printf("SDHC_SEND_RELATIVE_ADDR\n");
        result0 = 0x400;
        break;
    case 7:
        printf("SDHC_SELECT_CARD\n");
        state = STATE_STBY;
        result0 = (state << 9) | 0x100;
        break;
    case 8:
        printf("SDHC_RESULT_IF_COND 0x%03x\n", argument & 0xFFF);
        result0 = argument & 0xFFF;
        break;
    case 9:
        printf("SDHC_SEND_CID\n");
        result3 = (csd.data[0] >> 8) | (csd.data[3] << 24);
        result2 = (csd.data[1] >> 8) | (csd.data[0] << 24);
        result1 = (csd.data[2] >> 8) | (csd.data[1] << 24);
        result0 = (csd.data[3] >> 8) | (csd.data[2] << 24);
        break;
    case 13:
        printf("SDHC_SEND_STATUS\n");
        state = STATE_TRAN;
        result0 = (state << 9) | 0x100;
        break;
    case 16:
        printf("SDHC_SET_BLOCKLEN\n");
        result0 = (state << 9) | 0x100;
        break;
    case 17:
    {
        printf("SDHC_READ_BLOCK_SINGLE\n");
        printf("Transferring %d bytes from offset 0x%08x\n", block_count*block_size, argument);
        
        transferSrc = fileData+argument;
        
        break;
    }
    case 55:
        app_cmd = true;
        result0 = 0x20;
        break;
    default:
        printf("Unknown SDHC command 0x%02x\n", index);
        exit(1);
    }
}

void SDHC::DoAppCommand(int index)
{
    switch (index)
    {
    case 6:
        printf("SDHC_SET_BUS_WIDTH %d\n", argument & 3);
        bus_width = argument & 3;
        result0 = (state << 9) | 0x120;
        break;
    case 41:
        result0 = 0x300000;
		if (argument & result0)
			result0 |= 0xC0000000;
        result0 |= 0x80000000;
        printf("SDHC_SEND_OP_COND 0x%x\n", argument);
        break;
    default:
        printf("Unknown SDHC app command 0x%02x\n", index);
        exit(1);
    }
}

void SDHC::Write32(uint32_t addr, uint32_t data)
{
    switch (addr)
    {
    case 0x04:
    {
        printf("0x%08x -> BLOCK_SIZE\n", data);
        block_size = data & 0xFFF;
        block_count = data >> 16;
        break;
    }
    case 0x08:
        printf("0x%08x -> SDIO_ARG\n", data);
        argument = data;
        break;
    case 0x0c:
    {
        transfer_mode = data & 0xFFFF;
        command = data >> 16;

        result0 = result1 = result2 = result3 = 0;

        int index = (command >> 8) & 0x3F;

        if (app_cmd)
        {
            DoAppCommand(index);
            app_cmd = false;
        }
        else
        {
            DoCommand(index);
        }

        int_status |= 3;
        break;
    }
    case 0x28:
        printf("0x%08x -> SDIO_CTL\n", data);
        control = data;
        break;
    case 0x2c:
    {
        clock_control = data & 0xFFFF;
        if (clock_control & 1)
            clock_control |= 2;
        timeout_control = (data >> 16) & 0xFF;
        
        static int firstTime = 16;

        int reset = (data >> 24) & 0xFF;
        if (reset & 1)
            Reset();

        // if (firstTime)
        //     firstTime--;
        // else
        //     throw std::runtime_error("Infinite loop");
        break;
    }
    case 0x30:
        printf("0x%08x -> SDHC_INT_STATUS\n", data);
        int_status &= ~data;
        break;
    case 0x34:
        printf("0x%08x -> SDHC_INT_ENABLE\n", data);
        int_enable = data;
        break;
    case 0x38:
        printf("0x%08x -> SDHC_INT_SIGNAL\n", data);
        int_signal = data;
        break;
    default:
        printf("Write to unknown SDIO address 0x%08x\n", addr);
        exit(1);
    }
}

void SDHC::Reset()
{
    printf("Resetting SDHC\n");
    argument = 0;
	transfer_mode = 0;
	command = 0;
	result0 = 0;
	result1 = 0;
	result2 = 0;
	result3 = 0;
	control = 0;
	clock_control = 0;
	timeout_control = 0;
	int_status = 0;
	int_enable = 0;
	int_signal = 0;
	capabilities = 0;
	
	state = STATE_IDLE;
	app_cmd = false;
	
	bus_width = 0;
	cd_disable = 0;
}
