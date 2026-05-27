/* Copyright (C) 2026 wqshao All rights reserved.
 *
 *  File Name    : spl.c
 *  Author       : wqshao
 *  Created Time : 2026-01-23 15:02:09
 *  Description  :
 */
#define DEBUG
#include <asm/armv8/mmu.h>
#include <spl.h>
#include <log.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <debug_uart.h>
#ifdef CONFIG_ARMV8_SPIN_TABLE
#include <linux/delay.h>
#endif

#include "common.h"

#if CONFIG_IS_ENABLED(GENERATE_SMBIOS_TABLE)
#include <smbios.h>
#include <asm/global_data.h>
#include <linux/sizes.h>

static int dfd_install_smbios_table(void)
{
	ulong addr = DFD_SMBIOS_TABLE_BASE;

	if (!write_smbios_table(addr)) {
		log_err("Failed to write SMBIOS table\n");
		return -EINVAL;
	}

	log_debug("SMBIOS tables written to 0x%lx\n", addr);
	gd->arch.smbios_start = addr;

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, dfd_install_smbios_table);
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return 0;
}
#endif

#ifdef CONFIG_BOARD_INIT
int board_init(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	check_cpu_boot_mode();
}
#endif

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}
#endif /* CONFIG_SYSRESET */

int dram_init(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
        int ret;

        ret = fdtdec_setup_memory_banksize();
        if (ret)
                return ret;

        ret = fdtdec_setup_mem_size_base();

        return ret;
}

int board_late_init(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	check_cpu_boot_mode();
	//TODO:
        return 0;
}

int board_fdt_blob_setup(void **fdtp)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return 0;
}

void set_dfu_alt_info(char *interface, char *devstr)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
}

static struct mm_region dfd_mem_map[] = {
        {
                /* DDR address space */
                .virt = 0x0UL,
                .phys = 0x0UL,
                .size = 0x0000f0000000UL, /* 3.75GB */
                .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                         PTE_BLOCK_INNER_SHARE
        }, {
                /* IO address space */
                .virt = 0x0000f0000000UL,
                .phys = 0x0000f0000000UL,
                .size = 0x000010000000UL,
                .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
                         PTE_BLOCK_NON_SHARE |
                         PTE_BLOCK_PXN | PTE_BLOCK_UXN
        }, {
                /* List terminator */
                0,
        }
};
struct mm_region *mem_map = dfd_mem_map;

void board_cleanup_before_linux(void)
{
	debug("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	check_cpu_boot_mode();
}
