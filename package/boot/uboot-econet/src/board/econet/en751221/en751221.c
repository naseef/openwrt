// SPDX-License-Identifier: GPL-2.0+
/*
 * EcoNet EN751221 board initialization
 */

#include <config.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Board init - called after relocation
 *
 * The factory bootloader has already initialized UART, clocks, and DRAM.
 */
int board_init(void)
{
	/* Address of boot parameters for Linux kernel */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	return 0;
}
