#!/bin/bash
##########################################################
# File Name		: m.sh
# Author		: winddoing
# Created Time	: 2021年10月01日 星期五 08时43分09秒
# Description	:
##########################################################

# SD card partition diagram
#            0     1            17                49
#            +-----+------------+-----------------+---------------------+
#            | MBR |    SPL     |      ENV        |       uboot         |
#            | 1blk|   16blk    |     32blk       |       2048blk       |
#            +-----+------------+-----------------+---------------------+
# size KB --> 512B      8KB            16KB                512KB

export ARCH=arm
export CROSS_COMPILE=arm-none-eabi-

make_uboot() {
	echo "Build u-boot"
	set -x

	#make mrproper
	#make distclean
	make clean
	make s5p_wdg_defconfig
	make -j`nproc`
	ls -lsh spl/wdg-spl.bin
	ls -lsh u-boot.bin
	ctags -R ./*
	cp -arpv u-boot.bin ~/tftprootfs/

	set +x
}

save_defconfig() {
	echo "Save defconfig"
	set -x

	make savedefconfig
	cp -v defconfig configs/s5p_wdg_defconfig

	set +x
}

burn_uboot() {
	echo "Burn u-boot"
	local spl_bin="spl/wdg-spl.bin"
	local uboot_bin="u-boot.bin"
	local sd_dev="/dev/sda"

	if [ ! -f $spl_bin ]; then
		echo "spl bin($spl_bin) does not exist"
		exit 255
	fi

	if [ ! -b $sd_dev ]; then
		echo "SD dev($sd_dev) does not exist"
		exit 255
	fi

	set -x
	sudo dd if=$spl_bin of=$sd_dev bs=512 seek=1
	sudo dd if=$uboot_bin of=$sd_dev bs=512 seek=49

	sync
	set +x
}

make_config()
{
	echo "Make menuconfig"

	make s5p_wdg_defconfig
	make menuconfig
}

usage() {
cat << EOF
	*** No parameters for compiling uboot ***

	Usage:
	  $0 <option>

	option:
		h|help      Help
		s|save      Save current config
		c|config    make menuconfig
		m|make      Build uboot
		b|burn      Burn spl & uboot to SD
EOF
}

case $1 in
	h|help)
		usage
		;;
	c|config)
		make_config
		save_defconfig
		;;
	s|save)
		save_defconfig
		;;
	m|make)
		make_uboot
		;;
	b|burn)
		burn_uboot
		;;
	*)
		make_uboot
		;;
esac
