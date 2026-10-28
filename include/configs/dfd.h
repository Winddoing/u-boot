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
 * CPU release address.
 * Note:
 *      It is released in board_init
 */
#define CPU_RELEASE_ADDR	(DFD_SHARE_BASE + 0x0)

/*
 * CPU release flag.
 * Secondary CPU execute WFE until it is set to be 1.
 */
#define CPU_RELEASE_FLAG	(DFD_SHARE_BASE + 0x10)
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
