/* Copyright (C) 2026 wqshao All rights reserved.
 *
 *  File Name    : dfd.h
 *  Author       : wqshao
 *  Created Time : 2026-01-23 14:59:29
 *  Description  :
 */

#ifndef __DFD_H__
#define __DFD_H__

/* SRAM region */
#define DFD_SRAM_BASE	0xFFF00000
#define DFD_SRAM_SIZE	0x4000

/* share region */
#define DFD_SHARE_BASE	(DFD_SRAM_BASE)
#define DFD_SHARE_SIZE	0x1000

#ifdef CONFIG_ARMV8_SPIN_TABLE
/****************************** SPIN TABLE Start ******************************/

/*
 * DFD SoC multi-core boot mechanism (defined by miniboot):
 *
 * - CPU_ENTRYPOINT: 0xFFF00000, holds jump address for secondary CPUs
 * - CPU_HOLD_BASE:  0xFFF00008, per-CPU state registers
 *   State address = CPU_HOLD_BASE + cpu_id * 8
 *
 * Secondary CPU startup (in miniboot plat_secondary_cold_boot_setup):
 *   1. Read MPIDR_EL1 & 0xFF to get own CPU ID
 *   2. Set state at CPU_HOLD_BASE + cpu_id * 8 to WAIT (0)
 *   3. WFE loop until state becomes GO (1)
 *   4. Load jump address from CPU_ENTRYPOINT and branch
 */
#define DFD_CPU_ENTRYPOINT	0xFFF00000
#define DFD_CPU_HOLD_BASE	0xFFF00008
#define DFD_CPU_HOLD_STATE_GO	1

/*
 * Only for normal compilation to pass, the boot method of the slave core in U-boot,
 * which uses the single address, is not employed.
 * */
#define CPU_RELEASE_ADDR       (DFD_SHARE_BASE + 0x0)
/******************************* SPIN TABLE End *******************************/
#endif

/*
 * SMBIOS table location.
 *
 * Must be in Normal memory (DDR), not Device memory (SRAM), to avoid
 * unaligned access faults on packed structures containing u16 fields.
 * DDR range: 0x0 ~ 0x80000000 (2GB)
 */
#define DFD_SMBIOS_TABLE_BASE	0x7FFFF000
#define DFD_SMBIOS_TABLE_SIZE	0x1000

#define CFG_EXTRA_ENV_SETTINGS \
	"mmcboot=echo Booting from mmc ...; " \
		 "mmc read 0x3000000 0x800 0x8000;" \
		 "bootm 0x3000000;\0" \
	"dfd_boot=run mmcboot\0"

#endif//__DFD_H__
