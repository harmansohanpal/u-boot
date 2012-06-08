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
#include <asm/arch/pcie.h>

/*
 *  Application Register Offsets
 */
#define PCISTATSET                      0x010
#define CMD_STATUS                      0x004
#define CFG_SETUP                       0x008
#define IOBASE                          0x00c
#define OB_SIZE                         0x030
#define MSI_IRQ                         0x054
#define GPR0                            0x70
#define OB_OFFSET_INDEX(n)              (0x200 + (8 * n))     /* 32 Registers */
#define OB_OFFSET_HI(n)                 (0x204 + (8 * n))     /* 32 Registers */
#define IB_BAR0                         0x300
#define IB_START0_LO                    0x304
#define IB_START0_HI                    0x308
#define IB_OFFSET0                      0x30c
#define ERR_IRQ_STATUS_RAW              0x1c0
#define ERR_IRQ_STATUS                  0x1c4
#define MSI0_IRQ_STATUS                 0x104
#define MSI0_IRQ_ENABLE_SET             0x108
#define MSI0_IRQ_ENABLE_CLR             0x10c
#define IRQ_ENABLE_SET                  0x188
#define IRQ_ENABLE_CLR                  0x18c

/*
 * PCIe Config Register Offsets (capabilities)
 */
#define LINK_CAP                        0x07c

/* BAR mem width related registers */

#define BAR_NONPREF_32BIT       0x0
#define BAR_PREF_32BIT          0x8
#define BAR_NONPREF_64BIT       0x4
#define BAR_PREF_64BIT          0xC

/*
 * PCIe Config Register Offsets (misc)
 */
#define PL_LINK_CTRL                    0x710
#define DEBUG0                          0x728
#define PL_GEN2                         0x80c

/* Application command register values */
#define DBI_CS2_EN_VAL                  BIT(5)
#define IB_XLAT_EN_VAL                  BIT(2)
#define OB_XLAT_EN_VAL                  BIT(1)
#define LTSSM_EN_VAL                    BIT(0)

/* Link training encodings as indicated in DEBUG0 register */
#define LTSSM_STATE_MASK                0x1f
#define LTSSM_STATE_L0                  0x11

#define LTSSM_PART_LCTRL_MASK           0xf0000000

/* Directed Speed Change */
#define DIR_SPD                         (1 << 17)

static unsigned int bootflag;

static unsigned int wdt_base;
static unsigned int wdt_reload;
static unsigned int trig;

static inline int app_retry_enable(void)
{
	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) | (1 << 4));
	return 0;
}

static inline int app_retry_disable(void)
{
	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) & (~(1 << 4)));
	return 0;
}

int set_basic_config(void)
{
	/* set magic no */
	write_pcie_appl_reg(GPR0, MAGIC_NO);
	/* set class code */
	write_pcie_lcfg_reg(CLSCODE_REVID, 0x04000001);
	return 0;
}

int enable_dbi_cs2(void)
{

	write_pcie_appl_reg(CMD_STATUS,
			read_pcie_appl_reg(CMD_STATUS) | (1 << 5));

	while ((read_pcie_appl_reg(CMD_STATUS) & (1 << 5)) == 0)
		;

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

void set_ti81xx_device_id(void)
{	unsigned int devid;
	devid = get_ti81xx_device_id();
	devid = ((unsigned int)(TI81XX_VENDORID  | devid << 16));
	write_pcie_lcfg_reg(VENDOR_DEVICE_ID, devid);
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

#ifdef TI81XX_NO_PIN_GPMC
void set_size_config_64(int *bar_enable_mask)
{
	write_pcie_lcfg_reg(BAR0, SIZE_4KB);
	write_pcie_lcfg_reg(BAR1, DISABLE);
	write_pcie_lcfg_reg(BAR3, DISABLE);
	write_pcie_lcfg_reg(BAR5, DISABLE);
	*bar_enable_mask |= (1 << 0);
	if (CONFIG_BAR_SIZE(CONFIG_REG2_64)) {
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_REG2_64));
		*bar_enable_mask |= (1 << 2);
	} else
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_REG2_64));

	if (CONFIG_BAR_SIZE(CONFIG_REG4_64)) {
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_REG4_64));
		*bar_enable_mask |= (1 << 4);
	} else
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_REG4_64));
}

void set_size_config_32(int *bar_enable_mask)
{
	write_pcie_lcfg_reg(BAR0, SIZE_4KB);
	write_pcie_lcfg_reg(BAR5, DISABLE);
	*bar_enable_mask |= (1 << 0);
	if (CONFIG_BAR_SIZE(CONFIG_BAR1_32)) {
		write_pcie_lcfg_reg(BAR1, CONFIG_BAR_SIZE(CONFIG_BAR1_32));
		*bar_enable_mask |= (1 << 1);
	} else
		write_pcie_lcfg_reg(BAR1, CONFIG_BAR_SIZE(CONFIG_BAR1_32));
	if (CONFIG_BAR_SIZE(CONFIG_BAR2_32)) {
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_BAR2_32));
		*bar_enable_mask |= (1 << 2);
	} else
		write_pcie_lcfg_reg(BAR2, CONFIG_BAR_SIZE(CONFIG_BAR2_32));
	if (CONFIG_BAR_SIZE(CONFIG_BAR3_32)) {
		write_pcie_lcfg_reg(BAR3, CONFIG_BAR_SIZE(CONFIG_BAR3_32));
		*bar_enable_mask |= (1 << 3);
	} else
		write_pcie_lcfg_reg(BAR3, CONFIG_BAR_SIZE(CONFIG_BAR3_32));
	if (CONFIG_BAR_SIZE(CONFIG_BAR4_32)) {
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_BAR4_32));
		*bar_enable_mask |= (1 << 4);
	} else
		write_pcie_lcfg_reg(BAR4, CONFIG_BAR_SIZE(CONFIG_BAR4_32));
}

#else

void set_size_pin_64(unsigned long long *size)
{	/* DISABLE stands for 0*/
	write_pcie_lcfg_reg(BAR0, size[0]);
	write_pcie_lcfg_reg(BAR1, DISABLE);
	write_pcie_lcfg_reg(BAR2, size[2]);
	write_pcie_lcfg_reg(BAR3, DISABLE);
	write_pcie_lcfg_reg(BAR4, size[4]);
	write_pcie_lcfg_reg(BAR5, DISABLE);
}

void set_size_pin_32(unsigned long long *size)
{
	write_pcie_lcfg_reg(BAR0, size[0]);
	write_pcie_lcfg_reg(BAR1, size[1]);
	write_pcie_lcfg_reg(BAR2, size[2]);
	write_pcie_lcfg_reg(BAR3, size[3]);
	write_pcie_lcfg_reg(BAR4, size[4]);
	write_pcie_lcfg_reg(BAR5, DISABLE);
}
#endif
void set_bar_config_32(int *bar_enable_mask)
{
	/* BAR0 & BAR1 are always enabled, BAR0 hardwired to appl space */
	write_pcie_lcfg_reg(BAR0, BAR_NONPREF_32BIT);
	write_pcie_lcfg_reg(BAR1, BAR_PREF_32BIT);
	if (*bar_enable_mask & (1 << 2))
		write_pcie_lcfg_reg(BAR2, BAR_PREF_32BIT);
	else
		write_pcie_lcfg_reg(BAR2, BAR_NONPREF_32BIT);

	if (*bar_enable_mask & (1 << 3))
		write_pcie_lcfg_reg(BAR3, BAR_PREF_32BIT);
	else
		write_pcie_lcfg_reg(BAR3, BAR_NONPREF_32BIT);

	if (*bar_enable_mask & (1 << 4))
		write_pcie_lcfg_reg(BAR4, BAR_PREF_32BIT);
	else
		write_pcie_lcfg_reg(BAR4, BAR_NONPREF_32BIT);

	/* BAR5 always kept disabled */
}

void set_bar_config_64(int *bar_enable_mask)
{
	write_pcie_lcfg_reg(BAR0, BAR_NONPREF_64BIT);
	if (*bar_enable_mask & (1 << 2))
		write_pcie_lcfg_reg(BAR2, BAR_PREF_64BIT);
	else
		write_pcie_lcfg_reg(BAR2, BAR_NONPREF_64BIT);
	if (*bar_enable_mask & (1 << 4))
		write_pcie_lcfg_reg(BAR4, BAR_PREF_64BIT);
	else
		write_pcie_lcfg_reg(BAR4, BAR_NONPREF_64BIT);
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

void install_exceptions(void)
{
	/* Install the exception vectors and hook them */
	asm ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));

	__raw_writel(0x4031d030, 0x40300000);
	__raw_writel(0x40300000, 0xE3A00312);
	__raw_writel(0x40300004, 0xE3800706);
	__raw_writel(0x40300008, 0xE38000A0);
	__raw_writel(0x4030000C, 0xE3A01002);
	__raw_writel(0x40300010, 0xE5801000);
}

void wdt_set_reload(unsigned int base, unsigned int reload)
{
	if (base)
		__raw_writel(base + 0x2C, reload);
}

void wdt_load_start(unsigned int base, unsigned int load)
{
	if (base) {
		__raw_writel(base + 0x30, load);
		__raw_writel(base + 0x48, 0xbbbb);

		while (__raw_readl(base + 0x34) != 0x0)
			;

		__raw_writel(base + 0x48, 0x4444);

		while (__raw_readl(base + 0x34) != 0x0)
			;
	}
}

void wdt_stop(unsigned int base)
{
	if (base) {
		__raw_writel(base + 0x48, 0xaaaa);

		while (__raw_readl(base + 0x34) != 0x0)
			;

		__raw_writel(base + 0x48, 0x5555);

		while (__raw_readl(base + 0x34) != 0x0)
			;
	}
}

void pcie_enable_link(void)
{
	unsigned int debug0;
	wdt_load_start(wdt_base, trig++);

	/* Enable LTSSM */
	__raw_writel(0x51000004, __raw_readl(0x51000004) | 0xb01);
	DEBUGF("LTTSM enabled\n");

	while (1) {
		debug0 = __raw_readl(0x51001728);

		if ((debug0 & LTSSM_STATE_MASK) == LTSSM_STATE_L0)
			break;
		DEBUGF("\nDebug0 = %#x, Debug1 = #%x",
				debug0, __raw_readl(0x5100172C));
	}
	wdt_stop(wdt_base);

	/* Disable application retry now that we have link up */
	app_retry_disable();
	DEBUGF("\napp_retry_disable() done\n");

	printf("PCIe link UP, waiting for boot from HOST...\n");
}

void wait_for_host(void)
{
	unsigned int debug0;
	while (1) {
		if (__raw_readl(bootflag) != 0) {
			DEBUGF("data tranfer happened\n");
			break;
		}
		DEBUGF("waiting for data transfer\t");

		debug0 = __raw_readl(0x51001728);
#ifdef DEBUG_LTSSM
		DEBUGF("\tD0: %#x\tD1 %#x\n", debug0, __raw_readl(0x5100172C));
#endif

		__asm__("dmb");
	}

	__raw_writel(bootflag, 0x0);
	printf("\t---> boot command received, proceed to auto boot...\n");
}

int pcie_init(void)
{
	int bar_enable_mask = 0;
#ifndef TI81XX_NO_PIN_GPMC
	unsigned long long bar_size[6] = {0};
#endif

	trig = 1;
	get_pcie_wdt_base_reload(&wdt_base, &wdt_reload);
	wdt_set_reload(wdt_base, wdt_reload);

	bootflag = (unsigned int) get_bootflag_addr();
	__raw_writel(bootflag, 0x0);

	pcie_hw_setup();
	pcie_pll_setup();
	pcie_enable_module();
	app_retry_enable();
	DEBUGF("app_retry_enable() done\n");
	set_basic_config();
	DEBUGF("set basic config() done\n");

	write_pcie_lcfg_reg_bits(STATUS_COMMAND,
			CFG_REG_CMD_STATUS_MEM_SPACE_ENB,
			CFG_REG_CMD_STATUS_MEM_SPACE);
	enable_dbi_cs2();
	set_ti81xx_device_id();
#ifdef CONFIG_TI81XX_PCIE_32
#ifdef TI81XX_NO_PIN_GPMC
	set_size_config_32(&bar_enable_mask);
#else
	get_size_boot_mode_pin_32(&bar_enable_mask, bar_size);
	set_size_pin_32(bar_size);
#endif
#endif

#ifdef CONFIG_TI81XX_PCIE_64
#ifdef TI81XX_NO_PIN_GPMC
	set_size_config_64(&bar_enable_mask);
#else
	get_size_boot_mode_pin_64(&bar_enable_mask, bar_size);
	set_size_pin_64(bar_size);
#endif
#endif

	DEBUGF("set_bar_size() done\n");
	disable_dbi_cs2();
	DEBUGF("disable_dbi_cs2() done\n");
#ifdef CONFIG_TI81XX_PCIE_32
	set_bar_config_32(&bar_enable_mask);
#endif

#ifdef CONFIG_TI81XX_PCIE_64
	set_bar_config_64(&bar_enable_mask);
#endif

	DEBUGF("set_bar_config() done\n");
	config_appl_regs();
	DEBUGF("config_appl_regs() done\n");
	if (get_ti81xx_device_id() == TI8148_DEVICEID) {
		config_force_x1();      /* Not for TI816X */
		DEBUGF("config_force_x1() done\n");
	}
	pcie_enable_link();
	install_exceptions();
	wait_for_host();
	return 0;
}
