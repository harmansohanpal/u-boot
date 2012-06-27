/*
 * (C) Copyright Texas Instruments 2012
 * Harman Sohanpal <harman_sohanpal@ti.com>
 *
 * Most of this source has been derived from the Linux USB
 * project.
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <command.h>
#include <usb/musb_udc.h>

int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		return cmd_usage(cmdtp);
	/* Check if correct module number is selected */
	if ((strncmp(argv[1], "0", 1) == 0)) {
		printf("USB 0 Selected\n");
		printf("Switched to DFU mode\n");
		printf("Waiting for the images to be "
			"transferred from the "
			"Host PC\n");
		drv_usbdfu_init(0);
	} else
		if ((strncmp(argv[1], "0", 1) == 1)) {
			printf("USB 1 Selected\n");
			printf("Switched to DFU mode\n");
			printf("Waiting for the images to be "
				"transferred from the "
				"Host PC\n");
			drv_usbdfu_init(1);
		} else {
			printf("Select USB 0 or 1\n");
			return cmd_usage(cmdtp);
		}
	return 0;
}
U_BOOT_CMD(
	dfu,	5,	1,	do_dfu,
	"USB Device Firmware Upgrade",
	"[module] - enables selected usb module in Device Firmware Upgrade Mode"
);
