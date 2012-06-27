#ifndef _DFU_H
#define _DFU_H

/* USB Device Firmware Update Implementation for u-boot
 * (C) 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on: USB Device Firmware Update Implementation for OpenPCD
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 * This ought to be compliant to the USB DFU Spec 1.0 as available from
 * http://www.usb.org/developers/devclass_docs/usbdfu10.pdf
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <asm/types.h>
#include <usbdescriptors.h>
#include <usb/usb_dfu_descriptors.h>
#include <config.h>
#ifdef CONFIG_CMD_NAND
#include <nand.h>
#endif

/* USB DFU functional descriptor */
#define DFU_FUNC_DESC  {						\
	.bLength		= USB_DT_DFU_SIZE,			\
	.bDescriptorType	= USB_DT_DFU,				\
	.bmAttributes		= USB_DFU_CAN_UPLOAD | USB_DFU_CAN_DOWNLOAD\
						| USB_DFU_MANIFEST_TOL, \
	.wDetachTimeOut		= 0xff00,				\
	.wTransferSize		= CONFIG_USBD_DFU_XFER_SIZE,		\
	.bcdDFUVersion		= 0x0100,				\
}

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

#define NUM_CONFIGS	1
#define NUM_STRINGS DFU_STR_COUNT
#define STR_COUNT 7
#ifndef CONFIG_USBD_PRODUCTID_DFU
#define CONFIG_USBD_PRODUCTID_DFU 0xa4a7
#endif

#ifndef DFU_NUM_ALTERNATES
#define DFU_NUM_ALTERNATES	NUMBER_OF_PARTITIONS
#endif

#define STR_LANG 0
#define DFU_STR_MANUFACTURER	0x1
#define DFU_STR_PRODUCT		0x2
#define DFU_STR_SERIAL		0x3
#define DFU_STR_CONFIG		(STR_COUNT)
#define DFU_STR_ALT(n)		(STR_COUNT+(n)+1)
#define DFU_STR_COUNT		DFU_STR_ALT(DFU_NUM_ALTERNATES)

#define CONFIG_DFU_CFG_STR	"USB Device Firmware Upgrade"
#define CONFIG_DFU_ALT0_STR	"RAM 0x82000000"

struct _dfu_desc {
	struct usb_configuration_descriptor ucfg;
	struct usb_interface_descriptor uif[DFU_NUM_ALTERNATES];
	struct usb_dfu_func_descriptor func_dfu;
};

int dfu_init_instance(void);

#define DFU_EP0_NONE		0
#define DFU_EP0_UNHANDLED	1
#define DFU_EP0_STALL		2
#define DFU_EP0_ZLP		3
#define DFU_EP0_DATA		4

/* The address for start and end of RAM being used for DFU */
#define DFU_LOAD_ADDRESS ((unsigned char *)0x82000000)
#define DFU_END_ADDRESS	((unsigned char *)0x8a000000)
extern enum dfu_state *system_dfu_state; /* for 3rd parties */

#ifdef CONFIG_CMD_NAND
extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);
#endif
int dfu_ep0_handler(struct urb *urb);

void dfu_event(struct usb_device_instance *device,
	       usb_device_event_t event, int data);

#endif /* _DFU_H */
