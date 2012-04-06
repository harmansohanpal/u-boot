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

#ifndef _TI81XX_PCIE_H_
#define _TI81XX_PCIE_H_

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
#define CONFIG_BAR1_32	(0x800000ULL)	/* enter size in bytes */
#define CONFIG_BAR2_32  (0x800000ULL)  /* enter size in bytes */
#define CONFIG_BAR3_32  (0xfffULL)  /* enter size in bytes */
#define CONFIG_BAR4_32  (0x1001ULL)  /* enter size in bytes */
#define CONFIG_REG2_64  (0x1000000ULL)  /* enter size in bytes */
#define CONFIG_REG4_64  (0x1000000ULL)  /* enter size in bytes */
/* Replace these max and min bar sizes according to PCIE specs */
#define MAX_BAR_SIZE	(0x40000000ULL) /* maximum size will be trucated according to PCIE specs*/
#define MIN_BAR_SIZE	(0x1000ULL) /* alignment on base of min bar size  according to PCIE specs*/
/* Generalized Macro for BAR size settings */
#define CONFIG_BAR_SIZE(SIZE)	\
	((SIZE > MAX_BAR_SIZE) ? (MAX_BAR_SIZE - 1) : \
	 ((SIZE % MIN_BAR_SIZE) ? \
	  ((SIZE / MIN_BAR_SIZE + 1) * MIN_BAR_SIZE - 1) : \
	  (SIZE ? SIZE - 1 : 0)))
#endif

/*
 * OCMC location to communicate boot process
 */
#define TI814X_BOOTFLAG_ADDR				0x4031b7fc


#define TI81XX_VENDORID		((unsigned int)0x104C)
#define TI8148_DEVICEID		((unsigned int)0xB801)
#define TI8148_ID		((unsigned int)(TI81XX_VENDORID  | TI8148_DEVICEID << 16))
#define MAGIC_NO		0x10101010

#define TI81XX_PCIE_BASE		0x51000000
#define TI81XX_CONTROL_BASE		0x48140000

struct gpmc_config {
	unsigned char boot_dev_size;
        unsigned char cs0_mux_dev;
        unsigned char boot_wait_en;
        unsigned char wait_select;
};

/*
 * Control module configuraiton regsiters needed to enable/set PCIe and sample
 * configuration pins
 */

#define	PCIE_CFG	0x480
#define PCIE_PLLSTATUS  0x6EC
#define PCIE_PLLCFG0 	0x6D8
#define PCIE_PLLCFG1	0x6DC 
#define PCIE_PLLCFG2	0x6E0 
#define PCIE_PLLCFG3	0x6E4 
#define PCIE_PLLCFG4	0x6E8
#define SMA0		0x1318

/*
 * PCIe registers
 */

/*
 *  Application Register Offsets
 */
#define PCISTATSET                      0x010
#define CMD_STATUS			0x004
#define CFG_SETUP			0x008
#define IOBASE				0x00c
#define OB_SIZE				0x030
#define MSI_IRQ				0x054
#define GPR0				0x70
#define OB_OFFSET_INDEX(n)              (0x200 + (8 * n))     /* 32 Registers */
#define OB_OFFSET_HI(n)			(0x204 + (8 * n))     /* 32 Registers */
#define IB_BAR0				0x300
#define IB_START0_LO			0x304
#define IB_START0_HI			0x308
#define IB_OFFSET0			0x30c
#define ERR_IRQ_STATUS_RAW		0x1c0
#define ERR_IRQ_STATUS			0x1c4
#define MSI0_IRQ_STATUS			0x104
#define MSI0_IRQ_ENABLE_SET		0x108
#define MSI0_IRQ_ENABLE_CLR		0x10c
#define IRQ_ENABLE_SET			0x188
#define IRQ_ENABLE_CLR			0x18c


/* these register at offset 0x1000 from PCIE_BASE */
#define VENDOR_DEVICE_ID		0x0
#define STATUS_COMMAND                  0x4
#define CLSCODE_REVID			0x8
#define BAR0				0x10
#define BAR1				0x14
#define BAR2				0x18
#define BAR3				0x1c
#define BAR4				0x20
#define BAR5				0x24


/*
 * PCIe Config Register Offsets (capabilities)
 */
#define LINK_CAP			0x07c

/*
 * PCIe Config Register Offsets (misc)
 */
#define PL_LINK_CTRL			0x710
#define DEBUG0				0x728
#define PL_GEN2				0x80c

/* Various regions in PCIESS address space */
#define SPACE0_LOCAL_CFG_OFFSET		0x1000
#define SPACE0_REMOTE_CFG_OFFSET	0x2000
#define SPACE0_IO_OFFSET		0x3000

/* Application command register values */
#define DBI_CS2_EN_VAL			BIT(5)
#define IB_XLAT_EN_VAL			BIT(2)
#define OB_XLAT_EN_VAL			BIT(1)
#define LTSSM_EN_VAL			BIT(0)

/* Link training encodings as indicated in DEBUG0 register */
#define LTSSM_STATE_MASK		0x1f
#define LTSSM_STATE_L0			0x11

#define LTSSM_PART_LCTRL_MASK		0xf0000000

/* Directed Speed Change */
#define DIR_SPD				(1 << 17)


/* bar size and mask */
#define DISABLE		0x0
#define SIZE_4KB	0x00000FFF
#define SIZE_8MB	0x007FFFFF
#define SIZE_16MB	0x00FFFFFF
#define SIZE_32MB	0x01FFFFFF
#define SIZE_64MB	0x03FFFFFF
#define SIZE_128MB	0x07FFFFFF
#define SIZE_256MB	0x0FFFFFFF
#define SIZE_512MB	0x1FFFFFFF
#define SIZE_1GB	0x3FFFFFFF
#define SIZE_2GB	0x7FFFFFFF
#define SIZE_4GB	0xFFFFFFFF

/*
 * Device SYSBOOT pins values to be read from this register for setting BAR
 * size
 */
#define CONTROL_STATUS			0x40

#define TI8148_BT_DEVSIZE_MASK		(0x00010000)
#define BT_MSK_SHIFT			16

#define TI8148_CS0_DEVSIZE_MASK		(0x000c0000)
#define	CS0_MSK_SHIFT			18

#define TI8148_BTW_EN			(0x00020000)
#define BTW_EN_MSK_SHIFT		17


/* BAR mem width related registers */

#define BAR_NONPREF_32BIT	0x0
#define BAR_PREF_32BIT		0x8
#define BAR_NONPREF_64BIT	0x4
#define BAR_PREF_64BIT		0xC

#define  BAR_START_LOW   0x20000000
#define  BAR_START_LOW_0 0x40000000
#define  BAR_START_LOW_1 0x60000000
#define  BAR_START_LOW_2 0x80000000
#define  BAR_START_LOW_3 0xa0000000

#define  BAR_OFFSET_0 0x40300000  /* OCMC RAM Base */
#define  BAR_OFFSET_1 0x50000000  /* GPMC Base */
#define  BAR_OFFSET_2 0x80000000  /* DDR 0 Base */
#define  BAR_OFFSET_3 0xC0000000  /* DDR 1 Base */

#define CFG_REG_CMD_STATUS_MEM_SPACE_POS   1
#define CFG_REG_CMD_STATUS_MEM_SPACE        \
	(1 << CFG_REG_CMD_STATUS_MEM_SPACE_POS)
#define CFG_REG_CMD_STATUS_MEM_SPACE_ENB   0x1


#endif /*_TI81XX_PCIE_H_*/
