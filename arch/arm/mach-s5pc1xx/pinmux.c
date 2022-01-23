// SPDX-License-Identifier: GPL-2.0+
/*
 * Dummy functions to keep s5p_goni building (although it won't work)
 *
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <asm/arch/pinmux.h>

#include <asm/gpio.h>

static int s5pv210_mmc_config(int peripheral, int flags)
{
	int i = 0;

	/* MASSMEMORY_EN: XMSMDATA7: GPJ2[7] output high */
	/* gpio_request(S5PC110_GPIO_J27, "massmemory_en"); */
	/* gpio_direction_output(S5PC110_GPIO_J27, 1); */

	/*
	 * MMC0 GPIO
	 * GPG0[0]	SD_0_CLK
	 * GPG0[1]	SD_0_CMD
	 * GPG0[2]	SD_0_CDn	-> Not used
	 * GPG0[3:6]	SD_0_DATA[0:3]
	 */

	for (i = S5PC110_GPIO_G00; i < S5PC110_GPIO_G07; i++) {
		if (i == S5PC110_GPIO_G02)
			continue;
		/* GPG0[0:6] special function 2 */
		gpio_cfg_pin(i, 0x2);
		/* GPG0[0:6] pull disable */
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		/* GPG0[0:6] drv 4x */
		gpio_set_drv(i, S5P_GPIO_DRV_4X);
	}

	return 0;
}


int exynos_pinmux_config(int peripheral, int flags)
{
	debug("%s: peripheral=%d", __func__, peripheral);

	switch (peripheral) {
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
	case PERIPH_ID_SDMMC2:
	case PERIPH_ID_SDMMC3:
		return s5pv210_mmc_config(peripheral, flags);

	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	}

	return 0;
}

int pinmux_decode_periph_id(const void *blob, int node)
{
	int periph_id = 0;
	
	periph_id = fdtdec_get_int(blob, node, "periph_id", 0);
	debug("%s: periph_id=%d", __func__, periph_id);

	return periph_id;
}
