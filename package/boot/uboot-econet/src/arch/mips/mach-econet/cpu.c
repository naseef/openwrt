// SPDX-License-Identifier: GPL-2.0+
/*
 * EcoNet EN751221 SoC CPU initialization
 */

#include <init.h>
#include <stdio.h>
#include <asm/addrspace.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/io.h>


#define EN751221_SYSCTL_BASE		0x1fb00000

/*
 * Reset Control Register
 */
#define EN751221_SYSCTL_RSTCR		0x40
#define RSTCR_RESET			BIT(31)

DECLARE_GLOBAL_DATA_PTR;


unsigned long notrace get_tbclk(void)
{
	return 450000000;  /* 450 MHz (CP0 Count = CPU / 2) */
}

/*
 * Print SoC information
 */
int print_cpuinfo(void)
{
	printf("SoC:   EcoNet EN751221 (MIPS 34Kc)\n");
	return 0;
}

/*
 * Machine reset
 */
void _machine_restart(void)
{
	void __iomem *rstcr = (void __iomem *)CKSEG1ADDR(EN751221_SYSCTL_BASE + EN751221_SYSCTL_RSTCR);

	/* Trigger system reset */
	writel(RSTCR_RESET, rstcr);

	/* Wait for reset */
	while (1)
		;
}

/*
 * Lowlevel init - called very early
 *
 * Note: The bootloader has already initialized DRAM and basic
 * peripherals. We just need to set up U-Boot specific things.
 */
void lowlevel_init(void)
{
	/* Nothing to do - bootloader already initialized hardware */
}
