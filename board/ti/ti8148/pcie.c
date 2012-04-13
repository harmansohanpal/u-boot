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
#include "pcie.h"

#define __raw_readl(a)		(*(volatile unsigned int *)(a))
#define __raw_writel(a, v)	(*(volatile unsigned int *)(a) = (v))

static inline unsigned int read_reg(unsigned int a)
{
	return __raw_readl(a);
}

static inline void write_reg(unsigned int a, unsigned int v)
{
	__raw_writel(a, v);
}

static inline unsigned int read_reg_bits(unsigned int a, unsigned int mask)
{
	unsigned int val;
	val = read_reg(a);
	val &= mask;
	val >>= ffs(mask);
	return val;
}

static inline void write_reg_bits(unsigned int a, unsigned int v,
		unsigned int mask)
{
	unsigned int val;
	val = read_reg(a);
	val = (val & ~mask) | ((v << ffs(mask)) & mask);
	__raw_writel(a, val);
}

#define read_pcie_appl_reg(offset)	\
	read_reg(TI81XX_PCIE_BASE + offset)

#define write_pcie_appl_reg(offset, v)	\
	write_reg(TI81XX_PCIE_BASE + offset, v)

#define read_pcie_appl_reg_bits(offset, mask)	\
	read_reg_bits(TI81XX_PCIE_BASE + offset, mask)

#define write_pcie_appl_reg_bits(offset, v, mask)	\
	write_reg(TI81XX_PCIE_BASE + offset, v, mask)

#define read_pcie_lcfg_reg(offset)	\
	read_reg(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET + offset)

#define write_pcie_lcfg_reg(offset, v)	\
	write_reg(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET + offset, v)

#define read_pcie_lcfg_reg_bits(offset, mask)	\
	read_reg_bits(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET + offset, mask)

#define write_pcie_lcfg_reg_bits(offset, v, mask)			\
	write_reg_bits(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET	\
			+ offset, v, mask)

#define DEBUG_LTSSM

static unsigned int trig = 0;
static inline void delay_loop(volatile int count);
struct gpmc_config g_pin_conf;
static int bar_enable_mask = 0;

int app_retry_enable(void)
{
	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) | (1 << 4));
	return 0;
}

int app_retry_disable(void)
{
	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) & (~(1 << 4)));
	return 0;
}

int set_basic_config(void)
{
	/* set vendor device id */
	write_pcie_lcfg_reg(VENDOR_DEVICE_ID, TI8148_ID);
	/* set magic no */
	write_pcie_appl_reg(GPR0, MAGIC_NO);
	/* set class code */
	write_pcie_lcfg_reg(CLSCODE_REVID, 0x04000001);
	return 0;
}

int set_bar_config_32(void)
{
	/* BAR0 & BAR1 are always enabled, BAR0 hardwired to appl space */
	write_pcie_lcfg_reg(BAR0, BAR_NONPREF_32BIT);
	write_pcie_lcfg_reg(BAR1, BAR_START_LOW_0 | BAR_PREF_32BIT);
	if (bar_enable_mask & (1 << 2))
		write_pcie_lcfg_reg(BAR2, BAR_START_LOW_1 | BAR_PREF_32BIT);
	else
		write_pcie_lcfg_reg(BAR2, BAR_NONPREF_32BIT);

	if (bar_enable_mask & (1 << 3))
		write_pcie_lcfg_reg(BAR3, BAR_START_LOW_2 | BAR_PREF_32BIT);
	else
		write_pcie_lcfg_reg(BAR3, BAR_NONPREF_32BIT);

	if (bar_enable_mask & (1 << 4))
		write_pcie_lcfg_reg(BAR4, BAR_START_LOW_3 | BAR_PREF_32BIT);
	else
		write_pcie_lcfg_reg(BAR4, BAR_NONPREF_32BIT);

	/* BAR5 always kept disabled */

	return 0;
}

int set_bar_config_64(void)
{
	write_pcie_lcfg_reg(BAR0, BAR_NONPREF_64BIT);
	if (bar_enable_mask & (1 << 2))
		write_pcie_lcfg_reg(BAR2, BAR_PREF_64BIT);
	else
		write_pcie_lcfg_reg(BAR2, BAR_NONPREF_64BIT);
	if (bar_enable_mask & (1 << 4))
		write_pcie_lcfg_reg(BAR4, BAR_PREF_64BIT);
	else
		write_pcie_lcfg_reg(BAR4, BAR_NONPREF_64BIT);
	return 0;
}

int set_bar_sizes_64(void)
{
	unsigned int reg2_pins, reg4_pins;
	reg2_pins = (g_pin_conf.boot_dev_size) |
			((g_pin_conf.boot_wait_en) << 1);
	reg4_pins = g_pin_conf.cs0_mux_dev;
	write_pcie_lcfg_reg(BAR0, SIZE_4KB);
	write_pcie_lcfg_reg(BAR1, DISABLE);
	write_pcie_lcfg_reg(BAR5, DISABLE);
	write_pcie_lcfg_reg(BAR3, DISABLE);
	bar_enable_mask = (1 << 0); /* for region 0*/

#ifndef TI81XX_NO_PIN_GPMC
	switch (reg2_pins) {
	case 0:
		write_pcie_lcfg_reg(BAR2, DISABLE);
		break;
	case 1:
		write_pcie_lcfg_reg(BAR2, SIZE_256MB);
		bar_enable_mask |= (1 << 2); /* for region 2*/
		break;
	case 2:
		write_pcie_lcfg_reg(BAR2, SIZE_512MB);
		bar_enable_mask |= (1 << 2);
		break;
	case 3:
		write_pcie_lcfg_reg(BAR2, SIZE_1GB);
		bar_enable_mask |= (1 << 2);
		break;
	default:
		write_pcie_lcfg_reg(BAR2, DISABLE);
	}

	switch (reg4_pins) {
	case 0:
		write_pcie_lcfg_reg(BAR4, DISABLE);
		break;
	case 1:
		write_pcie_lcfg_reg(BAR4, SIZE_1GB);
		bar_enable_mask |= (1 << 4); /* for region 4*/
		break;
	case 2:
		write_pcie_lcfg_reg(BAR4, SIZE_2GB);
		bar_enable_mask |= (1 << 4);
		break;
	case 3:
		write_pcie_lcfg_reg(BAR4, SIZE_4GB);
		bar_enable_mask |= (1 << 4);
		break;
	default:
		write_pcie_lcfg_reg(BAR4, DISABLE);
	}
#else
	DEBUGF("\nconfiguration for 64 bit present,"
			"avoiding GPMC pin config\n");
	if (CONFIG_BAR_SIZE(CONFIG_REG2_64)) {
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_REG2_64));
		bar_enable_mask |= (1 << 2);
	} else
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_REG2_64));

	if (CONFIG_BAR_SIZE(CONFIG_REG4_64)) {
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_REG4_64));
		bar_enable_mask |= (1 << 4);
	} else
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_REG4_64));
#endif
	return 0;
}




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

int set_bar_size_32(void)
{
	unsigned int bar2_pins, bar3_pins, bar4_pins;

	write_pcie_lcfg_reg(BAR0, SIZE_4KB);
	write_pcie_lcfg_reg(BAR5, DISABLE);
	bar_enable_mask = (1 << 0);
	bar2_pins = g_pin_conf.boot_dev_size;

	bar3_pins = g_pin_conf.cs0_mux_dev;
	bar4_pins = g_pin_conf.boot_wait_en;
#ifndef TI81XX_NO_PIN_GPMC
	write_pcie_lcfg_reg(BAR1, SIZE_8MB);
	bar_enable_mask |= (1 << 1);
	switch (bar2_pins) {
	case 0:
		write_pcie_lcfg_reg(BAR2, DISABLE);
		break;
	case 1:
		write_pcie_lcfg_reg(BAR2, SIZE_16MB);
		bar_enable_mask |= (1 << 2);
		break;
	default:
		write_pcie_lcfg_reg(BAR2, DISABLE);
	}

	switch (bar3_pins) {
	case 0:
		write_pcie_lcfg_reg(BAR3, DISABLE);
		break;
	case 1:
		write_pcie_lcfg_reg(BAR3, SIZE_32MB);
		bar_enable_mask |= (1 << 3);
		break;
	case 2:
		write_pcie_lcfg_reg(BAR3, SIZE_64MB);
		bar_enable_mask |= (1 << 3);
		break;
	case 3:
		write_pcie_lcfg_reg(BAR3, SIZE_128MB);
		bar_enable_mask |= (1 << 3);
		break;
	default:
			write_pcie_lcfg_reg(BAR3, DISABLE);
	}

	switch (bar4_pins) {
	case 0:
		write_pcie_lcfg_reg(BAR4, DISABLE);
		break;
	case 1:
		write_pcie_lcfg_reg(BAR4, SIZE_256MB);
		bar_enable_mask |= (1 << 4);
		break;
	default:
		write_pcie_lcfg_reg(BAR4, DISABLE);
	}
#else
	DEBUGF("\nconfiguration for 32 bit present,"
			"avoiding GPMC pin config \n");
	if (CONFIG_BAR_SIZE(CONFIG_BAR1_32)) {
		write_pcie_lcfg_reg(BAR1, CONFIG_BAR_SIZE(CONFIG_BAR1_32));
		bar_enable_mask |= (1 << 1);
	} else
		write_pcie_lcfg_reg(BAR1, CONFIG_BAR_SIZE(CONFIG_BAR1_32));
	if (CONFIG_BAR_SIZE(CONFIG_BAR2_32)) {
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_BAR2_32));
		bar_enable_mask |= (1 << 2);
	} else
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_BAR2_32));
	if (CONFIG_BAR_SIZE(CONFIG_BAR3_32)) {
		write_pcie_lcfg_reg(BAR3, CONFIG_BAR_SIZE(CONFIG_BAR3_32));
		bar_enable_mask |= (1 << 3);
	} else
		write_pcie_lcfg_reg(BAR3, CONFIG_BAR_SIZE(CONFIG_BAR3_32));
	if (CONFIG_BAR_SIZE(CONFIG_BAR4_32)) {
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_BAR4_32));
		bar_enable_mask |= (1 << 4);
	} else
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_BAR4_32));
#endif

	return 0;
}


int enable_dbi_cs2(void)
{

	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) | (1 << 5));

	while ((read_pcie_appl_reg(CMD_STATUS) & (1 << 5)) == 0);

	return 0;
}

int disable_dbi_cs2(void)
{
	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) & (~(1 << 5)));

	return 0;
}

int config_appl_regs(void)
{
	/*
	 * Enable inbound translation --
	 *  by default BAR 0 is for appl registers , rest of inbound config can
	 *  be set by drivers
	 */
	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) | (1 << 2));

	return 0;
}

/**
 * Should be called only for devices which have x1 link or for debug purpose.
 */
void config_force_x1(void)
{
	u32 val;

	val = read_pcie_lcfg_reg(LINK_CAP);
	val = (val & ~(0x3f << 4)) | (1 << 4);
	write_pcie_lcfg_reg(LINK_CAP, val);

	val = read_pcie_lcfg_reg(PL_GEN2);
	val = (val & ~(0xff << 8)) | (1 << 8);
	write_pcie_lcfg_reg(PL_GEN2, val);

	val = read_pcie_lcfg_reg(PL_LINK_CTRL);
	val = (val & ~(0x3F << 16)) | (1 << 16);
	write_pcie_lcfg_reg(PL_LINK_CTRL, val);
}

int pcie_init(void)
{
	unsigned int debug0;

#define WDT_LDR		0x481C702C
#define WDT_SPR		0x481C7048
#define WDT_TGR		0x481C7030

	/*
	 * Watchdog reload - tune this value so that it is as small to reset
	 * quickly when in trouble but at the same time sufficient enough to
	 * allow execution of each iteration of the loop below.
	 */
	__raw_writel(0x481C702C, 0xfffff001);

	__raw_writel(TI814X_BOOTFLAG_ADDR,0x0);

	write_reg(TI81XX_CONTROL_BASE + PCIE_CFG, 0x0);

	__raw_writel(0x48140E24,0x00000002); //PowerDown
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406D8,0x00000000); //cfgpll0//SERDES CFG0
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406DC,0x00640000); //cfgpll1//SERDES CFG1
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406E0,0x00000000); //cfgpll2//SERDES CFG2
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406E4,0x004008E0); //cfgpll3//SERDES CFG3
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406E8,0x0000609C); //cfgpll4//SERDES CFG4
	delay_loop(50); // Wait 50 us
	__raw_writel(0x48141318,0x00000E7B); //pcie_serdes_cfg_misc

	//delay_loop(1); // Wait 50 us
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406D8,0x00000004); //Config PLL CFG0 bit [2] - ENBGSC_REF
	//delay_loop(3); // Wait 50 us
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406D8,0x00000014); //Config PLL CFG0 bit [4] - DIGLDO
	//delay_loop(2); // Wait 50 us
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406D8,0x00000016); //Config PLL CFG0 bit [1] - ENPLLLDO
	//delay_loop(2); // Wait 50 us
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406D8,0x30000016); // Configure proxy TXLDO and RXLDO enables (Centaurus ECO 3/30/10)
	//delay_loop(2); // Wait 50 us
	delay_loop(50); // Wait 50 us
	__raw_writel(0x481406D8,0x70007016); // Configure multiplier
	//delay_loop(2); // Wait 200 us
	delay_loop(200); // Wait 50 us
	__raw_writel(0x481406D8,0x70007017);  // Enable PLL
	while ((__raw_readl(0x481406EC) & 0x1) == 0);
	DEBUGF("\nPCIe serdes and PLL setup done Successfully.....  \n");

	DEBUGF("\nClear PCIe EP setup.....  \n");


	__raw_writel(0x48180B10, 0x000000FF);
	__raw_writel(0x48180578, 0);
	__raw_writel(0x48180510, 0);
	delay_loop(50); // Wait 50 us
	__raw_writel(0x48180510, 2);
	__raw_writel(0x48180578, 2);
	delay_loop(50); // Wait 50 us
	__raw_writel(0x48180B10, 0x0000007F);
	delay_loop(50); // Wait 50 us

	DEBUGF("\n PCIe is out reset (PRCM)\n");

	/* Set PCIe registers
	 *	1) set vendor id etc
	 *	2) Set basic config parameters
	 *	3) Set BARs, also set inbound defaults, eg., BAR1 maps to OCMC,
	 *	BAR2 and onwards (as applicable) map to DDR locations from
	 *	0x80000000.
	 */

	app_retry_enable();
	DEBUGF("app_retry_enable() done\n");
	read_gpmc_pin_config();
	DEBUGF("read_gpmc_pin_config() done\n");
	set_basic_config();
	DEBUGF("set basic config() done\n");

	write_pcie_lcfg_reg_bits(STATUS_COMMAND,
			CFG_REG_CMD_STATUS_MEM_SPACE_ENB,
			CFG_REG_CMD_STATUS_MEM_SPACE);
	enable_dbi_cs2();
	DEBUGF("enable_dbi_cs2() done\n");
#ifdef CONFIG_TI81XX_PCIE_32
	set_bar_size_32();
#endif

#ifdef CONFIG_TI81XX_PCIE_64
	set_bar_sizes_64();
#endif

	DEBUGF("set_bar_size() done\n");
	disable_dbi_cs2();
	DEBUGF("disable_dbi_cs2() done\n");
#ifdef CONFIG_TI81XX_PCIE_32
	set_bar_config_32();
#endif

#ifdef CONFIG_TI81XX_PCIE_64
	set_bar_config_64();
#endif

	DEBUGF("set_bar_config32() done\n");
	config_appl_regs();
	DEBUGF("config_appl_regs() done\n");
	config_force_x1();	/* Not for TI816X */
	DEBUGF("config_force_x1() done\n");

	/* Stop, load and start WDT */
	__raw_writel(0x481C7030, trig++);
	__raw_writel(0x481C7048, 0xbbbb);
	while (__raw_readl(0x481C7034) != 0x0);
	__raw_writel(0x481C7048, 0x4444);
	while (__raw_readl(0x481C7034) != 0x0);

	/* Enable LTSSM */
	__raw_writel(0x51000004, __raw_readl(0x51000004) | 0xb01);
	DEBUGF("LTTSM enabled\n");

	while (1) {
		debug0 = __raw_readl(0x51001728);

		if ((debug0 & LTSSM_STATE_MASK) == LTSSM_STATE_L0)
			break;

		DEBUGF("\nDebug0 = %#x, Debug1 = #%x", debug0, __raw_readl(0x5100172C));

	}

	/* Stop watchdog */
	__raw_writel(0x481C7048, 0xaaaa);
	while (__raw_readl(0x481C7034) != 0x0);
	__raw_writel(0x481C7048, 0x5555);
	while (__raw_readl(0x481C7034) != 0x0);

	/* Disable application retry now that we have link up */
	app_retry_disable();
	DEBUGF("\napp_retry_disable() done\n");

	printf("PCIe link UP, waiting for boot from HOST...\n");

	/* Install the exception vectors and hook them */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (0));

	__raw_writel(0x4031d030,0x40300000);
	__raw_writel(0x40300000,0xE3A00312);
	__raw_writel(0x40300004,0xE3800706);
	__raw_writel(0x40300008,0xE38000A0);
	__raw_writel(0x4030000C,0xE3A01002);
	__raw_writel(0x40300010,0xE5801000);


	while (1) {
		if (__raw_readl(TI814X_BOOTFLAG_ADDR) != 0 )
		{
			DEBUGF("data tranfer happened\n");
			break;
		}
		DEBUGF("waiting for data transfer\t");

		debug0 = __raw_readl(0x51001728);
#ifdef DEBUG_LTSSM
		DEBUGF("\tD0: %#x\tD1 %#x\n", debug0, __raw_readl(0x5100172C));
#endif


		/*	if (((debug0 & 0xF0000000) != 0)) 
			{
			printf( "\nhot reset or disbale link appeared while reading - D0=%#x", debug0);
			reset_cpu(0);
			}
			else if (((debug1 & 0xF0000000) == 0x2))
			{
			printf( "\nsystem appears to be in training while reading - D1 = %#x",debug1);
			reset_cpu(0);

			}
			*/		__asm__("dmb");
	}

	printf("\t---> boot command received, proceed to auto boot...\n");

	return 0;
}

void delay_loop(volatile int count)
{
	while (--count);
}
