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
#include <asm/arch-ti81xx/cpu.h>
#include <asm/arch-ti81xx/sys_proto.h>
#include <asm/arch/pcie.h>

/*
 * OCMC location to communicate boot process
 */
#define TI814X_BOOTFLAG_ADDR                            0x4031b7fc

/*
 * Control module configuraiton regsiters needed to enable/set PCIe and sample
 * configuration pins
 */

#define PCIE_CFG        0x480
#define PCIE_PLLSTATUS  0x6EC
#define PCIE_PLLCFG0    0x6D8
#define PCIE_PLLCFG1    0x6DC
#define PCIE_PLLCFG2    0x6E0
#define PCIE_PLLCFG3    0x6E4
#define PCIE_PLLCFG4    0x6E8
#define SMA0            0x1318

/*
 * Device SYSBOOT pins values to be read from this register for setting BAR
 * size
 */
#define CONTROL_STATUS                  0x40
#define TI8148_BT_DEVSIZE_MASK          (0x00010000)
#define BT_MSK_SHIFT                    16

#define TI8148_CS0_DEVSIZE_MASK         (0x000c0000)
#define CS0_MSK_SHIFT                   18

#define TI8148_BTW_EN                   (0x00020000)
#define BTW_EN_MSK_SHIFT                17

#define  BAR_START_LOW   0x20000000
#define  BAR_START_LOW_0 0x40000000
#define  BAR_START_LOW_1 0x60000000
#define  BAR_START_LOW_2 0x80000000
#define  BAR_START_LOW_3 0xa0000000

#define  BAR_OFFSET_0 0x40300000  /* OCMC RAM Base */
#define  BAR_OFFSET_1 0x50000000  /* GPMC Base */
#define  BAR_OFFSET_2 0x80000000  /* DDR 0 Base */
#define  BAR_OFFSET_3 0xC0000000  /* DDR 1 Base */


struct boot_cfg g_pin_conf; /* to fetch gpmc pin config */

int read_gpmc_pin_config(void)
{
	g_pin_conf.boot_dev_size = (unsigned char)
		((read_reg(TI81XX_CONTROL_BASE +
			   CONTROL_STATUS) &
		  (TI8148_BT_DEVSIZE_MASK)) >>
		 BT_MSK_SHIFT);

	g_pin_conf.cs0_mux_dev = (unsigned char) ((read_reg(TI81XX_CONTROL_BASE
					+ CONTROL_STATUS) &
				(TI8148_CS0_DEVSIZE_MASK)) >>
			CS0_MSK_SHIFT);

	g_pin_conf.boot_wait_en = (unsigned char) ((read_reg(TI81XX_CONTROL_BASE
					+ CONTROL_STATUS) & (TI8148_BTW_EN)) >>
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
		bar_size[2] = SIZE_16MB;
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
		bar_size[3] = SIZE_32MB;
		*bar_enable_mask |= (1 << 3);
	break;
	case 2:
		bar_size[3] = SIZE_64MB;
		*bar_enable_mask |= (1 << 3);
	break;
	case 3:
		bar_size[3] = SIZE_128MB;
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
		bar_size[2] = SIZE_256MB;
		*bar_enable_mask |= (1 << 2); /* for region 2*/
	break;
	case 2:
		bar_size[2] = SIZE_512MB;
		*bar_enable_mask |= (1 << 2);
	break;
	case 3:
		bar_size[2] = SIZE_1GB;
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
		bar_size[4] = SIZE_1GB;
		*bar_enable_mask |= (1 << 4); /* for region 4*/
	break;
	case 2:
		bar_size[4] = SIZE_2GB;
		*bar_enable_mask |= (1 << 4);
	break;
	case 3:
		bar_size[4] = SIZE_4GB;
		*bar_enable_mask |= (1 << 4);
	break;
	default:
		bar_size[4] = DISABLE;
	}
}

inline unsigned int get_ti81xx_device_id(void)
{
	if (get_cpu_type() == TI813X)
		return TI813X_DEVICEID;
	else
		return TI8148_DEVICEID;
}

void pcie_pll_setup(void)
{
	__raw_writel(0x48140E24, 0x00000002); /*PowerDown*/
	delay_loop(50); /* Wait 50 us*/
	__raw_writel(0x481406D8, 0x00000000); /*cfgpll0//SERDES CFG0*/
	delay_loop(50); /* Wait 50 us*/
	__raw_writel(0x481406DC, 0x00640000); /*cfgpll1//SERDES CFG1*/
	delay_loop(50); /*Wait 50 us*/
	__raw_writel(0x481406E0, 0x00000000); /*cfgpll2//SERDES CFG2*/
	delay_loop(50); /*Wait 50 us*/
	__raw_writel(0x481406E4, 0x004008E0); /*cfgpll3//SERDES CFG3*/
	delay_loop(50); /*Wait 50 us*/
	__raw_writel(0x481406E8, 0x0000609C); /*cfgpll4//SERDES CFG4*/
	delay_loop(50); /* Wait 50 us*/

	if ((get_cpu_type == TI8148) && (get_cpu_rev() < PG2_0))
		__raw_writel(0x48141318, 0x00000E7B); /*pcie_serdes_cfg_misc*/

	/*delay_loop(1); // Wait 50 us*/
	delay_loop(50); /*Wait 50 us*/
	 /* Config PLL CFG0 bit [2] - ENBGSC_REF*/
	__raw_writel(0x481406D8, 0x00000004);
	/*delay_loop(3); // Wait 50 us*/
	delay_loop(50); /*Wait 50 us*/
	/* Config PLL CFG0 bit [4] - DIGLDO*/
	__raw_writel(0x481406D8, 0x00000014);
	/*delay_loop(2); // Wait 50 us*/
	delay_loop(50); /*Wait 50 us*/
	/* Config PLL CFG0 bit [1] - ENPLLLDO*/
	__raw_writel(0x481406D8, 0x00000016);
	/*delay_loop(2); // Wait 50 us*/
	delay_loop(50); /*Wait 50 us*/
	/* Configure proxy TXLDO and RXLDO enables */
	__raw_writel(0x481406D8, 0x30000016);
	/*delay_loop(2); // Wait 50 us*/
	delay_loop(50); /*Wait 50 us*/
	__raw_writel(0x481406D8, 0x70007016); /*Configure multiplier*/
	/*delay_loop(2); // Wait 200 us*/
	delay_loop(200); /* Wait 50 us*/
	__raw_writel(0x481406D8, 0x70007017);  /*Enable PLL*/
	while ((__raw_readl(0x481406EC) & 0x1) == 0)
		;

	DEBUGF("\nPCIe serdes and PLL setup done Successfully.....\n");
}

void pcie_enable_module()
{
	DEBUGF("\nClear PCIe EP setup.....\n");

	__raw_writel(0x48180B10, 0x000000FF);
	__raw_writel(0x48180578, 0);
	__raw_writel(0x48180510, 0);
	delay_loop(50); /*Wait 50 us*/
	__raw_writel(0x48180510, 2);
	__raw_writel(0x48180578, 2);
	delay_loop(50); /*Wait 50 us*/
	__raw_writel(0x48180B10, 0x0000007F);
	delay_loop(50); /*Wait 50 us*/

	DEBUGF("\n PCIe is out reset (PRCM)\n");
}

void *get_bootflag_addr()
{
	return (void *)TI814X_BOOTFLAG_ADDR;
}

void pcie_hw_setup(void)
{
	/* Set up Endpoint mode */
	write_reg(TI81XX_CONTROL_BASE + PCIE_CFG, 0x0);

}

void get_pcie_wdt_base_reload(unsigned int *base, unsigned int *reload)
{
	*base = 0x481C7000;
	*reload = 0Xffff8000;
	/*timeout duration - 1sec with 32KHz clock*/
}
