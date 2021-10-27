# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>

obj-y	:= wdg.o onenand.o
obj-y	+= lowlevel_init.o led.o mem_setup.o

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
endif
