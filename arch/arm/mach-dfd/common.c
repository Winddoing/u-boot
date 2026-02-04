/* Copyright (C) 2026 wqshao All rights reserved.
 *
 *  File Name    : common.c
 *  Author       : wqshao
 *  Created Time : 2026-02-03 09:51:29
 *  Description  :
 */

#include <spl.h>
#include <log.h>

void check_cpu_boot_mode(void)
{
	unsigned long el;

	asm volatile("mrs %0, CurrentEL" : "=r" (el));

	el = (el >> 2) & 3;
	printf("Boot CPU is at EL%lu\n", el);
}

