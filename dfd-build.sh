#!/bin/bash
##########################################################
# Copyright (C) 2026 wqshao All rights reserved.
#  File Name    : dfd-m.sh
#  Author       : wqshao
#  Created Time : 2026-01-23 11:45:13
#  Description  :
##########################################################

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

top=$(pwd)

if [ $# -eq 0 ]; then
        args_list="-db"
else
        args_list=$@
fi

echo -n "Args list: $args_list"

info() { echo -e "\n\e[34m$@\e[0m"; }
warn() { echo -e "\n\e[33m$@\e[0m"; }

set -- $(getopt -q dmbch "$args_list")
while [ -n "$1" ]
do
        case "$1" in
                -d) info "Defconfig"
			set -x
			make dfd_defconfig
			set +x
                        shift ;;
                -m) info "Menuconfig"
			set -x
			make menuconfig
			make savedefconfig
			cp -v defconfig configs/dfd_defconfig
			set +x
                        shift ;;
                -b) info "Build Project"
			set -x
                        bear -- make -j`nproc`
			set +x
			ls -lsh dfd-u-boot-spl.bin
			ls -lsh u-boot.itb
                        shift ;;
                -c) info "Clean project"
			set -x
                        make distclean
			set +x
                        shift ;;
                -h) info "Help"
			echo "-d defconfig"
			echo "-m menuconfig"
			echo "-b build make"
			echo "-c distclean"
                        shift ;;
                --) shift
                        break ;;
                -*) warn "Nothing to do";;
        esac
done

