#!/bin/bash
##########################################################
# File Name		: m.sh
# Author		: winddoing
# Created Time	: 2021年10月01日 星期五 08时43分09秒
# Description	:
##########################################################

set -x

export ARCH=arm
export CROSS_COMPILE=arm-none-eabi-

make mrproper
make distclean
make s5p_wdg_defconfig
make -j`nproc`

