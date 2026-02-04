/* Copyright (C) 2026 wqshao All rights reserved.
 *
 *  File Name    : spl.c
 *  Author       : wqshao
 *  Created Time : 2026-01-24 15:13:47
 *  Description  :
 */
#define DEBUG
#include <spl.h>
#include <hang.h>
#include <log.h>

#include "common.h"

#if 0
void board_boot_order(u32 *spl_boot_list)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);

	spl_boot_list[0] = BOOT_DEVICE_MMC1;
	spl_boot_list[1] = BOOT_DEVICE_NOR;
	spl_boot_list[2] = BOOT_DEVICE_DFU;
}
#endif

u32 spl_boot_device(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);

	return BOOT_DEVICE_MMC1;
}

void board_init_f(ulong dummy)
{
	printf("uart early init in spl.\n");

	check_cpu_boot_mode();

	if (CONFIG_IS_ENABLED(OF_CONTROL)) {
		int ret;

		ret = spl_early_init();
		if (ret) {
			log_err("spl_early_init() failed: %d\n", ret);
			hang();
		}
	}

	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	preloader_console_init();

	/* TODO intialize clock tree */
	/* TODO intialize dram */
	printf("spl:init dram\n");
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void spl_board_init(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

void spl_board_prepare_for_boot(void)
{
	/* Before jumping to U-boot */
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	check_cpu_boot_mode();
}

