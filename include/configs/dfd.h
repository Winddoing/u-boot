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

/* SMBIOS regino is located at the end of the shared region */
#define DFD_SMBIOS_TABLE_BASE	(DFD_SHARE_BASE + DFD_SHARE_SIZE - DFD_SMBIOS_TABLE_SIZE)
#define DFD_SMBIOS_TABLE_SIZE	0x400

#define CFG_EXTRA_ENV_SETTINGS \
	"mmcboot=echo Booting from mmc ...; " \
		 "mmc read 0x3000000 0x800 0x8000;" \
		 "bootm 0x3000000;\0" \
	"dfd_boot=run mmcboot\0"

#endif//__DFD_H__
