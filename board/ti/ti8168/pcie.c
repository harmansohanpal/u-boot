/*
 * copyright (c) 2012, texas instruments, incorporated
 *
 * see file credits for list of people who contributed to this
 * project.
 *
 * this program is free software; you can redistribute it and/or
 * modify it under the terms of the gnu general public license as
 * published by the free software foundation version 2.
 *
 * this program is distributed "as is" without any warranty of any
 * kind, whether express or implied; without even the implied warranty
 * of merchantability or fitness for a particular purpose.  see the
 * gnu general public license for more details.
 */

#include <common.h>
#include <exports.h>
#include <asm/arch/pcie.h>

/*
 * OCMC location to communicate boot process
 */
#define TI816X_BOOTFLAG_ADDR                            0x4043FFFC
#define PCIE_CFG					0x640

#define CONTROL_STATUS                  0x40
#define TI8168_BT_DEVSIZE_MASK          (0x00010000)
#define BT_MSK_SHIFT                    16

#define TI8168_CS0_DEVSIZE_MASK         (0x000c0000)
#define CS0_MSK_SHIFT                   18

#define TI8168_BTW_EN                   (0x00020000)
#define BTW_EN_MSK_SHIFT                17




struct boot_cfg g_pin_conf; /* to fetch gpmc pin config */

int read_gpmc_pin_config(void)
{
	g_pin_conf.boot_dev_size = (unsigned char)
		((read_reg(TI81XX_CONTROL_BASE +
			   CONTROL_STATUS) &
		  (TI8168_BT_DEVSIZE_MASK)) >>
		 BT_MSK_SHIFT);

	g_pin_conf.cs0_mux_dev = (unsigned char) ((read_reg(TI81XX_CONTROL_BASE
					+ CONTROL_STATUS) &
				(TI8168_CS0_DEVSIZE_MASK)) >>
			CS0_MSK_SHIFT);

	g_pin_conf.boot_wait_en = (unsigned char) ((read_reg(TI81XX_CONTROL_BASE
					+ CONTROL_STATUS) & (TI8168_BTW_EN)) >>
			BTW_EN_MSK_SHIFT);
	return 0;
}


void get_size_boot_mode_pin_32(int *bar_enable_mask,
		unsigned long long *bar_size)
{
	unsigned int bar2_pins, bar3_pins, bar4_pins;

	/* Gather boot pin configurations */
	read_gpmc_pin_config();

	bar2_pins = g_pin_conf.boot_dev_size;
	bar3_pins = g_pin_conf.cs0_mux_dev;
	bar4_pins = g_pin_conf.boot_wait_en;
	bar_size[0] = SIZE_4KB;
	bar_size[1] = SIZE_8MB;
	bar_size[5] = DISABLE;
	*bar_enable_mask |= (1 << 0);
	*bar_enable_mask |= (1 << 1);

	switch (bar2_pins) {
	case 0:
		bar_size[2] = DISABLE;
	break;
	case 1:
		bar_size[2] = SIZE_8MB;
		*bar_enable_mask |= (1 << 2);
	break;
	default:
		bar_size[2] = DISABLE;
	}

	switch (bar3_pins) {
	case 0:
		bar_size[3] = DISABLE;
	break;
	case 1:
		bar_size[3] = SIZE_64MB;
		*bar_enable_mask |= (1 << 3);
	break;
	case 2:
		bar_size[3] = SIZE_128MB;
		*bar_enable_mask |= (1 << 3);
	break;
	case 3:
		bar_size[3] = SIZE_256MB;
		*bar_enable_mask |= (1 << 3);
	break;
	default:
		bar_size[3] = DISABLE;
	}

	switch (bar4_pins) {
	case 0:
		bar_size[4] = DISABLE;
	break;
	case 1:
		bar_size[4] = SIZE_256MB;
		*bar_enable_mask |= (1 << 4);
	break;
	default:
		bar_size[4] = DISABLE;
	}
}


void get_size_boot_mode_pin_64(int *bar_enable_mask,
		unsigned long long *bar_size)
{
	unsigned int reg2_pins, reg4_pins;

	/* Gather boot pin configurations */
	read_gpmc_pin_config();

	reg2_pins = (g_pin_conf.boot_dev_size) |
		((g_pin_conf.boot_wait_en) << 1);
	reg4_pins = g_pin_conf.cs0_mux_dev;
	bar_size[0] = SIZE_4KB;
	*bar_enable_mask = (1 << 0); /* for region 0*/

	switch (reg2_pins) {
	case 0:
		bar_size[2] = DISABLE;
	break;
	case 1:
		bar_size[2] = SIZE_8MB;
		*bar_enable_mask |= (1 << 2); /* for region 2*/
	break;
	case 2:
		bar_size[2] = SIZE_64MB;
		*bar_enable_mask |= (1 << 2);
	break;
	case 3:
		bar_size[2] = SIZE_128MB;
		*bar_enable_mask |= (1 << 2);
	break;
	default:
		bar_size[2] = DISABLE;
	}

	switch (reg4_pins) {
	case 0:
		bar_size[4] = DISABLE;
	break;
	case 1:
		bar_size[4] = SIZE_64MB;
		*bar_enable_mask |= (1 << 4); /* for region 4*/
	break;
	case 2:
		bar_size[4] = SIZE_128MB;
		*bar_enable_mask |= (1 << 4);
	break;
	case 3:
		bar_size[4] = SIZE_256MB;
		*bar_enable_mask |= (1 << 4);
	break;
	default:
		bar_size[4] = DISABLE;
	}
}

inline unsigned int get_ti81xx_device_id(void)
{
	return TI8168_DEVICEID;
}

void pcie_pll_setup()
{
	int lock;

	/* PLL setup */
	write_reg(0x48140640, 0x01C90000);

	do {
		lock = read_reg(0x48140640) & (1 << 8);
	} while (!lock);

	delay_loop(100);
	DEBUGF("***PLL locked\n");
}

void pcie_enable_module()
{
	DEBUGF("\nPRCM: begin PCIe setup\n");

	/* Clockdomain enable */
	write_reg(0x48180510, 2);
	/* Clock enable */
	write_reg(0x48180578, 2);
	/* De-assert LRST */
	write_reg(0x48180B10, read_reg(0x48180B10) & ~(1 << 7));

	while ((read_reg(0x48180578) & (0x3 << 16)) != 0)
		;

	DEBUGF("\nPRCM: PCIe is out reset\n");
}

void *get_bootflag_addr()
{
	return (void *)TI816X_BOOTFLAG_ADDR;
}

void pcie_hw_setup(void)
{
	/* Set up Endpoint mode */
	write_reg(TI81XX_CONTROL_BASE + PCIE_CFG, 0x0);

}

void get_pcie_wdt_base_reload(unsigned int *base, unsigned int *reload)
{
	*base = 0x480C2000;
	*reload = 0Xffff8000;
	/* time duration -- 1sec with 32 kHZ clock */
}
