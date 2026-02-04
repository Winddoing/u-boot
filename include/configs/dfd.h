/* Copyright (C) 2026 wqshao All rights reserved.
 *
 *  File Name    : dfd.h
 *  Author       : wqshao
 *  Created Time : 2026-01-23 14:59:29
 *  Description  :
 */

#ifndef __DFD_H__
#define __DFD_H__

#define CFG_EXTRA_ENV_SETTINGS \
	"mmcboot=echo Booting from mmc ...; " \
		 "mmc read 0x3000000 0x800 0x8000;" \
		 "bootm 0x3000000;\0" \
	"dfd_boot=run mmcboot\0"

#endif//__DFD_H__
