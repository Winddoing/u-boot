// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2008-2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/arch/mmc.h>
#include <dm.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <usb/dwc2_udc.h>
#include <asm/arch/cpu.h>
#include <power/max8998_pmic.h>
#include <samsung/misc.h>
#include <usb.h>
#include <usb_mass_storage.h>
#include <asm/mach-types.h>

#include "qt210_val.h"

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
#define SMSC9220_Tacs	(0x0)	// 0clk		address set-up
#define SMSC9220_Tcos	(0x4)	// 4clk		chip selection set-up
#define SMSC9220_Tacc	(0xe)	// 14clk	access cycle
#define SMSC9220_Tcoh	(0x1)	// 1clk		chip selection hold
#define SMSC9220_Tah	(0x4)	// 4clk		address holding time
#define SMSC9220_Tacp	(0x6)	// 6clk		page mode access cycle
#define SMSC9220_PMC	(0x0)	// normal(1data)page mode configuration

#define SROM_DATA16_WIDTH(x)	(1<<((x*4)+0))
#define SROM_ADDR_MODE_16BIT(x)	(1<<((x*4)+1))
#define SROM_WAIT_ENABLE(x)	(1<<((x*4)+2))
#define SROM_BYTE_ENABLE(x)	(1<<((x*4)+3))

/*
 * Miscellaneous platform dependent initialisations
 */
static void smsc9220_pre_init(int bank_num)
{
	unsigned int tmp;
//	unsigned char smc_bank_num=1;

	/* gpio configuration */
	tmp = MP01CON_REG;
	tmp &= ~(0xf << bank_num*4);
	tmp |= (0x2 << bank_num*4);
	MP01CON_REG = tmp;

	tmp = SROM_BW_REG;
	tmp &= ~(0xF<<(bank_num * 4));
	tmp |= SROM_DATA16_WIDTH(bank_num);
	tmp |= SROM_ADDR_MODE_16BIT(bank_num);
	SROM_BW_REG = tmp;

	if(bank_num == 0)
		SROM_BC0_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 1)
		SROM_BC1_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 2)
		SROM_BC2_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 3)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 4)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 5)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
}


u32 get_board_rev(void)
{
	return 0;
}

int board_init(void)
{
	smsc9220_pre_init(5);

	/* Set Initial global variables */
	gd->bd->bi_arch_number = 0xffffffff;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
void i2c_init_board(void)
{
	gpio_request(S5PC110_GPIO_J43, "i2c_clk");
	gpio_request(S5PC110_GPIO_J40, "i2c_data");
	gpio_direction_output(S5PC110_GPIO_J43, 1);
	gpio_direction_output(S5PC110_GPIO_J40, 1);
}
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE;//PHYS_SDRAM_3_SIZE;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	//gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	//gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tWdg\n");
	return 0;
}
#endif

#ifdef CONFIG_USB_GADGET
static int s5pc1xx_phy_control(int on)
{
	struct udevice *dev;
	static int status;
	int reg, ret;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret)
		return ret;

	if (on && !status) {
		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
		reg |= MAX8998_LDO3;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
		reg |= MAX8998_LDO8;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}
		status = 1;
	} else if (!on && status) {
		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
		reg &= ~MAX8998_LDO3;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
		reg &= ~MAX8998_LDO8;
		ret = pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -EINVAL;
		}
		status = 0;
	}
	udelay(10000);
	return 0;
}

struct dwc2_plat_otg_data s5pc110_otg_data = {
	.phy_control = s5pc1xx_phy_control,
	.regs_phy = S5PC110_PHY_BASE,
	.regs_otg = S5PC110_OTG_BASE,
	.usb_phy_ctrl = S5PC110_USB_PHY_CONTROL,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return dwc2_udc_probe(&s5pc110_otg_data);
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_info();
#endif
	return 0;
}
#endif

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
