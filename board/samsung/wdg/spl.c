// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <spl.h>

void wdg_led_status(int);

u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_MMC_SUPPORT
	return BOOT_DEVICE_MMC1;
#endif
	return BOOT_DEVICE_NAND;
}

/* the size and addr in emmc of bl1 and bl2 */
#define MOVI_BLKSIZE            (1<<9) /* 512 bytes */
#define MOVI_BL1_SDCARD_POS		512 /* 512 bytes */
#define MOVI_BL1_SIZE           (8 * 1024) /* 8KB */
#define MOVI_BL1_BLKCNT         (MOVI_BL1_SIZE / MOVI_BLKSIZE)        /* 16 sections */
#define MOVI_BL1_ENV_BLKCNT     (CONFIG_ENV_SIZE / MOVI_BLKSIZE)   /* CONFIG_ENV_SIZE=0x4000 defined, 32 sections */

#define MOVI_BL2_SDCARD_POS		((MOVI_BL1_SDCARD_POS / MOVI_BLKSIZE) + MOVI_BL1_BLKCNT + MOVI_BL1_ENV_BLKCNT) /* place at forty-ninth section in sdcard 49*/
#define MOVI_BL2_SIZE			(2* 512 * 1024) /* uboot.bin 512 KB */
#define MOVI_BL2_BLKCNT			(MOVI_BL2_SIZE / MOVI_BLKSIZE)


/* The point of the addr function "copy_sd_mmc_to_mem" */
#define CopySDMMCtoMem 0xD0037F98
#define SDMMC_BASE    0xD0037488
#define SDMMC_CH0_BASE_ADDR    0xEB000000
#define SDMMC_CH2_BASE_ADDR    0xEB200000

/* the function of copy_sd_mmc_to_mem that have been implement by s5pv210*/
typedef u32(*copy_sd_mmc_to_mem)
		(u32 channel, u32 start_block, u16 block_size, u32 *trg, u32 init);

void copy_bl2_to_ddr(void)
{
	u32 sdmmc_base_addr;
	copy_sd_mmc_to_mem copy_bl2 = (copy_sd_mmc_to_mem)(*(u32*)CopySDMMCtoMem);

	sdmmc_base_addr = *(u32 *)SDMMC_BASE;

	if(sdmmc_base_addr == SDMMC_CH0_BASE_ADDR)
		copy_bl2(0, MOVI_BL2_SDCARD_POS, MOVI_BL2_BLKCNT, (u32 *)CONFIG_SYS_TEXT_BASE, 0);
	if(sdmmc_base_addr == SDMMC_CH2_BASE_ADDR)
		copy_bl2(2, MOVI_BL2_SDCARD_POS, MOVI_BL2_BLKCNT, (u32 *)CONFIG_SYS_TEXT_BASE, 0);
}

void board_init_f(ulong dummy)
{
	wdg_led_status(0x6);

	__attribute__((noreturn)) void (*uboot)(void);

	int val;
#define DDR_TEST_ADDR 0x30000000
#define DDR_TEST_CODE 0xaa
	writel(DDR_TEST_CODE, DDR_TEST_ADDR);
	val = readl(DDR_TEST_ADDR);
	if(val == DDR_TEST_CODE) {
		wdg_led_status(0x7);
	} else {
		wdg_led_status(0xa);
		while(1);
	}

	copy_bl2_to_ddr();

	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	(*uboot)();
}
