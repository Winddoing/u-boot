/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * led debug
 */

#include <linux/linkage.h>

ENTRY(wdg_led_status)
	ldr r4, =0xE0200C00     /* GPH0CON寄存器 */
	ldr r5, =0x00001111
	str r5, [r4]            /* 设置GPH0_1~GPH0_3为输出模式 */
	ldr r4, =0xE0200C04     /* GPH0DAT寄存器 */

	//反转bit序,方便led指示灯读数
	//num_rev = ((num & 0x8) >> 3) | ((num & 0x4) >> 1) | ((num & 0x2) << 1) | ((num & 0x1) << 3);
	mov r3, r0
	asr r3, r3, #3
	and r2, r3, #1
	mov r3, r0
	asr r3, r3, #1
	and r3, r3, #2
	orr r2, r2, r3
	mov r3, r0
	lsl r3, r3, #1
	and r3, r3, #4
	orr r2, r2, r3
	mov r3, r0
	lsl r3, r3, #3
	and r3, r3, #8
	orr r3, r2, r3

	str r3, [r4]            /* 设置相应的GPH0_1~GPH0_3为高电平 */

	mov	pc, lr
ENDPROC(wdg_led_status)
