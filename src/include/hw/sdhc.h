#pragma once

#include <cstdint>
#include <fstream>

class SDHC
{
public:
    SDHC();

    uint32_t Read32(uint32_t addr);
    void Write32(uint32_t addr, uint32_t data);

    void Reset();
private:
    uint32_t dma_addr = 0;
    uint16_t block_size = 0;
    uint16_t block_count = 0;
    uint32_t argument = 0;
    uint16_t transfer_mode = 0;
    uint16_t command = 0;
    uint32_t result0 = 0;
    uint32_t result1 = 0;
    uint32_t result2 = 0;
    uint32_t result3 = 0;
    uint32_t control = 0;
    uint16_t clock_control = 0;
    uint8_t timeout_control = 0;
    uint32_t int_status;
    uint32_t int_enable;
    uint32_t int_signal;
    uint64_t capabilities;

    int bus_width;
    int cd_disable;

    enum State
    {
		STATE_IDLE = 0,
		STATE_READY = 1,
		STATE_IDENT = 2,
		STATE_STBY = 3,
		STATE_TRAN = 4,
		STATE_DATA = 5,
		STATE_RCV = 6,
		STATE_PRG = 7,
		STATE_DIS = 8
	} state = STATE_IDLE;

    bool app_cmd = false;

    void DoCommand(int index);
    void DoAppCommand(int index);

    union CardSpecificData {
		struct {
			uint32_t tran_speed : 8;
			uint32_t nsac : 8;
			uint32_t taac : 8;
			uint32_t reserved0 : 6;
			uint32_t csd_structure : 2;

			uint32_t csize_hi : 6;
			uint32_t reserved1 : 6;
			uint32_t dsr_imp : 1;
			uint32_t read_blk_misalign : 1;
			uint32_t write_blk_misalign : 1;
			uint32_t read_bl_partial : 1;
			uint32_t read_bl_len : 4;
			uint32_t ccc : 12;

			uint32_t wp_grp_size : 7;
			uint32_t sector_size : 7;
			uint32_t erase_blk_enable : 1;
			uint32_t reserved2 : 1;
			uint32_t csize_lo : 16;

			uint32_t crc : 8;
			uint32_t reserved5 : 2;
			uint32_t file_format : 2;
			uint32_t twp_write_protect : 1;
			uint32_t perm_write_protect : 1;
			uint32_t copy : 1;
			uint32_t file_format_grp : 1;
			uint32_t reserved4 : 5;
			uint32_t write_grp_enable : 1;
			uint32_t write_bl_len : 4;
			uint32_t r2w_factor : 3;
			uint32_t reserved3 : 2;
			uint32_t wp_grp_enable : 1;
		} __attribute__((packed));
		uint32_t data[4];
	};
	CardSpecificData csd;

    uint8_t* transferSrc;

    uint8_t* fileData;
};