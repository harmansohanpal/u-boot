/*
 * am335x.c - TI's AM335x platform specific usb wrapper functions.
 *
 * Author: Gene Zarkhin <gene_zarkhin@bose.com>
 * Modified by Harman Sohanpal <harman_sohanpal@ti.com>
*
 * Based on drivers/usb/musb/da8xx.c
 *
 * Copyright (c) 2010 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <common.h>
#include "am335x.h"

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	.timeout	= AM335X_USB_OTG_TIMEOUT,
	.musb_speed	= 0,
};

/*
 * Enable the USB phy
 */
#ifdef CONFIG_USBD_DFU
static u8 phy_on(int i)
#else
static u8 phy_on(void)
#endif
{
	u32 timeout;
	u32 regAddr = CM_REGISTERS+USB_CTRL0_REG_OFFSET;
	u32 usb_ctrl_reg;

#ifdef CONFIG_USBD_DFU
	if (i == 1)
		regAddr += 8;
#endif

	usb_ctrl_reg = readl(regAddr);
	usb_ctrl_reg &= ~(CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	usb_ctrl_reg |= (OTGVDET_EN | OTGSESSENDEN);
	writel(usb_ctrl_reg, regAddr);

	timeout = musb_cfg.timeout;
	writel(0x1, &am335x_usb_regs->ctrl);
	udelay(6000);
	while (timeout--) {
		if ((readl(&am335x_usb_regs->ctrl) & SOFT_RESET_BIT) == 0)
			return 1;
	}
	/* USB phy was not turned on */
	return 0;
}

/*
 * Disable the USB phy
 */
#ifdef CONFIG_USBD_DFU
static void phy_off(int i)
#else
static void phy_off(void)
#endif
{
	u32 regAddr = CM_REGISTERS + USB_CTRL0_REG_OFFSET;
	u32 usb_ctrl_reg;
#ifdef CONFIG_USBD_DFU
	if (i == 1)
		regAddr += 8;
#endif
	usb_ctrl_reg = readl(regAddr);
	usb_ctrl_reg |= (CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	writel(usb_ctrl_reg, regAddr);
}

/*
 * This function performs platform specific initialization for usb0.
 */
#ifdef CONFIG_USBD_DFU
int musb_platform_init(int i)
#else
int musb_platform_init(void)
#endif
{
	u32 revision;
#ifdef CONFIG_USBD_DFU
	if (i == 1)
		musb_cfg.regs = (struct musb_regs *)
			(AM335X_USB_OTG_CORE_BASE + 0x800);
	else
		 musb_cfg.regs = (struct musb_regs *)
			 (AM335X_USB_OTG_CORE_BASE);

		/* start the on-chip usb phy and its pll */
	if (phy_on(i) == 0)
		return -1;
#else
	musb_cfg.regs = (struct musb_regs *)(AM335X_USB_OTG_CORE_BASE);
	/* start the on-chip usb phy and its pll */
	if (phy_on() == 0)
		return -1;
#endif
	/* Returns zero if e.g. not clocked */
	revision = readl(&am335x_usb_regs->revision);
	if (revision == 0)
		return -1;

	return 0;
}

/*
 * This function performs platform specific deinitialization for usb0.
 */
#ifdef CONFIG_USBD_DFU
void musb_platform_deinit(int i)
{
	/* Turn off the phy */
	phy_off(i);
}
#else
void musb_platform_deinit(void)
{
	/* Turn off ther PHY */
	phy_off();
}
#endif
