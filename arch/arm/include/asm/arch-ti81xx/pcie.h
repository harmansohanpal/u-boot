/*
 * Copyright (C) 2012, Texas Instruments, Incorporated
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _TI81XX_PCIE_COMMON_
#define _TI81XX_PCIE_COMMON_

#ifdef TI81XX_PCIE_DBG
#define DEBUGF(fmt, ...) \
	printf(fmt, ##__VA_ARGS__)
#else
#define DEBUGF(fmt, ...) \
	({ if (0) printf(fmt, ##__VA_ARGS__); })
#endif

/*
 * Bar sizes can be decided on two basis:
 * 1 - CONFIG_MACROS defined below
 * 2 - GPMC pin settings
 * if config macros are present then they will be prefered over pin
 * settings and pin setting in this  case will  have no effect over
 * bar sizes. if config macro are not present then pin setting will
 * be considered for bar sizes.
 */
/* CONFIG MACROS for bar size */
/* In case of 32 bit PCIE settings
 * BAR(n)-- Region(n) referring same thing
 * BAR1 -- Region 1
 * BAR2 -- Region 2
 * BAR3 -- region 3
 * BAR4 -- region 4
 * BAR5 -- Region 5
 * In case of 64 bit PCIE settings
 * BAR(n+1):BAR(n) : Region(n) n-- 0,2,4
 * [BAR1 : BAR0] : Region 0
 * [BAR3 : BAR2] : Region 2
 * [BAR5 : BAR4] : Region 4
 * Region0 and BAR0 will always be 4KB
 */

#define TI81XX_NO_PIN_GPMC /* commnet this Macro to use GPMC pin setting */
#ifdef TI81XX_NO_PIN_GPMC

#ifndef CONFIG_BAR1_32
#define CONFIG_BAR1_32  (0x800000ULL)   /* enter size in bytes */
#endif

#ifndef CONFIG_BAR2_32
#define CONFIG_BAR2_32  (0x800000ULL)  /* enter size in bytes */
#endif

#ifndef CONFIG_BAR3_32
#define CONFIG_BAR3_32  (0xfffULL)  /* enter size in bytes */
#endif

#ifndef CONFIG_BAR4_32
#define CONFIG_BAR4_32  (0x1001ULL)  /* enter size in bytes */
#endif

#ifndef CONFIG_REG2_64
#define CONFIG_REG2_64  (0x1000000ULL)  /* enter size in bytes */
#endif

#ifndef CONFIG_REG4_64
#define CONFIG_REG4_64  (0x1000000ULL)  /* enter size in bytes */
#endif

/*
 * TODO: Replace these max and min bar sizes according to PCIE spec
 */

/* Maximum size will be trucated according to PCIE specs*/
#define MAX_BAR_SIZE    (0x40000000ULL)
/* Alignment on base of min bar size  according to PCIE specs*/
#define MIN_BAR_SIZE    (0x1000ULL)

/* Generalized Macro for BAR size settings */
#define CONFIG_BAR_SIZE(SIZE)   \
	((SIZE > MAX_BAR_SIZE) ? (MAX_BAR_SIZE - 1) : \
	 ((SIZE % MIN_BAR_SIZE) ? \
	  ((SIZE / MIN_BAR_SIZE + 1) * MIN_BAR_SIZE - 1) : \
	  (SIZE ? SIZE - 1 : 0)))
#endif

#define TI81XX_VENDORID         ((unsigned int)0x104C)
#define TI8148_DEVICEID         ((unsigned int)0xB801)
#define TI8168_DEVICEID         ((unsigned int)0xB800)
#define MAGIC_NO                0x10101010

#define TI81XX_PCIE_BASE                0x51000000
#define TI81XX_CONTROL_BASE             0x48140000


#define __raw_readl(a)          (*(volatile unsigned int *)(a))
#define __raw_writel(a, v)      (*(volatile unsigned int *)(a) = (v))

#define read_pcie_appl_reg(offset)      \
	read_reg(TI81XX_PCIE_BASE + offset)

#define write_pcie_appl_reg(offset, v)  \
	write_reg(TI81XX_PCIE_BASE + offset, v)
#define read_pcie_appl_reg_bits(offset, mask)   \
	read_reg_bits(TI81XX_PCIE_BASE + offset, mask)

#define write_pcie_appl_reg_bits(offset, v, mask)       \
	write_reg(TI81XX_PCIE_BASE + offset, v, mask)

#define read_pcie_lcfg_reg(offset)      \
	read_reg(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET + offset)

#define write_pcie_lcfg_reg(offset, v)  \
	write_reg(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET + offset, v)

#define read_pcie_lcfg_reg_bits(offset, mask)   \
	read_reg_bits(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET + offset, mask)

#define write_pcie_lcfg_reg_bits(offset, v, mask)                       \
	write_reg_bits(TI81XX_PCIE_BASE + SPACE0_LOCAL_CFG_OFFSET       \
			+ offset, v, mask)
#define DEBUG_LTSSM


/* these register at offset 0x1000 from PCIE_BASE */
#define VENDOR_DEVICE_ID                0x0
#define STATUS_COMMAND                  0x4
#define CLSCODE_REVID                   0x8
#define BAR0                            0x10
#define BAR1                            0x14
#define BAR2                            0x18
#define BAR3                            0x1c
#define BAR4                            0x20
#define BAR5                            0x24

/* Various regions in PCIESS address space */
#define SPACE0_LOCAL_CFG_OFFSET         0x1000
#define SPACE0_REMOTE_CFG_OFFSET        0x2000
#define SPACE0_IO_OFFSET                0x3000

/* bar size and mask */
#define DISABLE         0x0
#define SIZE_4KB        0x00000FFF
#define SIZE_8MB        0x007FFFFF
#define SIZE_16MB       0x00FFFFFF
#define SIZE_32MB       0x01FFFFFF
#define SIZE_64MB       0x03FFFFFF
#define SIZE_128MB      0x07FFFFFF
#define SIZE_256MB      0x0FFFFFFF
#define SIZE_512MB      0x1FFFFFFF
#define SIZE_1GB        0x3FFFFFFF
#define SIZE_2GB        0x7FFFFFFF
#define SIZE_4GB        0xFFFFFFFF

#define CFG_REG_CMD_STATUS_MEM_SPACE_POS   1
#define CFG_REG_CMD_STATUS_MEM_SPACE        \
	(1 << CFG_REG_CMD_STATUS_MEM_SPACE_POS)
#define CFG_REG_CMD_STATUS_MEM_SPACE_ENB   0x1


struct boot_cfg {
	unsigned char boot_dev_size;
	unsigned char cs0_mux_dev;
	unsigned char boot_wait_en;
	unsigned char wait_select;
};

static inline unsigned int read_reg(unsigned int a)
{
	return __raw_readl(a);
}

static inline void write_reg(unsigned int a, unsigned int v)
{
	__raw_writel(a, v);
}

static inline void delay_loop(volatile int count)
{
	while (--count)
		;
}

/* ----------------------------------------------------------------------------
 * Functions that need to be implemented by platform specific code.
 *	- These will be called at various times by PCIe core code.
 * ----------------------------------------------------------------------------
 */

/**
 * This function should be implemented to return the base address of watchdog
 * timer to be used as PCIe watchdog as well as provide reload value for wdt
 * expiry.
 *
 * A value NULL will mean that the PCIe code will not use watchdog and thus
 * recovery from PCIe link and other error timeouts is not possible.
 *
 * The reload value should be tuned for particular platform so that it is as
 * small to reseta quickly when in trouble but at the same time sufficient
 * enough to allow execution of pcie setup and polling.
 */
void get_pcie_wdt_base_reload(unsigned int *base, unsigned int *reload);
/**
 * This function should be implemented to return address of bootflag
 * u-boot polls on bootflag location to wiat for data transfer.
 */
void *get_bootflag_addr(void);
/**
 * This function should be implemented to initialize general details of PCIESS h/w.
 * Example : mode of PCIESS.
 */
void pcie_hw_setup(void);
/**
 * This function should be implemented to initialize PCIE-PLL.
 */
void pcie_pll_setup(void);
/**
 * This function should be implemented to enable PCIESS and get it out of reset.
 */
void pcie_enable_module(void);
/**
 * This function should be implemented to get device id.
 */
unsigned int get_ti81xx_device_id(void);
/**
 * This function should be implemented to process GPMC pin config and get size
 * to be set for BARS while PCIESS operating in 32 bit mode.
 */
void get_size_boot_mode_pin_32(int *, unsigned long long *);
/**
 * This function should be implemented toprocess GPMC pin config and get size to
 * be set for regions while PCIESS operatiin in 64 bit mode.
 */
void get_size_boot_mode_pin_64(int *, unsigned long long *);

#endif /* common  */
