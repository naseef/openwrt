// SPDX-License-Identifier: GPL-2.0+
/*
 * EcoNet EN751221 DRAM detection
 */

#include <init.h>
#include <asm/addrspace.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * REG_SAVE_INFO register contains DRAM size in MB (bits 11:0)
 * Set by factory bootloader during initialization.
 */
#define EN751221_REG_SAVE_INFO		0x1fb00284
#define EN751221_DRAM_SIZE_MASK		0xfff

/*
 * Get RAM size from factory bootloader's saved info
 *
 * The factory bootloader stores the DRAM size in MB in REG_SAVE_INFO.
 * If that fails, fall back to memory probing.
 */
int dram_init(void)
{
	void __iomem *reg = (void __iomem *)CKSEG1ADDR(EN751221_REG_SAVE_INFO);
	u32 val;
	unsigned long size_mb;

	val = readl(reg);
	size_mb = val & EN751221_DRAM_SIZE_MASK;

	/* Sanity check - valid sizes are 8, 16, 32, 64, 128, 256, 512 MB */
	if (size_mb >= 8 && size_mb <= 512) {
		gd->ram_size = size_mb * SZ_1M;
	} else {
		/*
		 * Fallback: probe memory
		 * get_ram_size() writes patterns to detect actual size
		 */
		gd->ram_size = get_ram_size((void *)KSEG1, SZ_256M);
	}

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}


phys_size_t get_effective_memsize(void)
{
	return gd->ram_size;
}
