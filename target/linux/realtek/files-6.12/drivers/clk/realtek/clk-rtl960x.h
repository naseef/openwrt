/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Realtek RTL960X clock driver header
 * Copyright (C) 2026 Ahmed Naseef <naseefkm@gmail.com>
 *
 * Based on OEM GPL code
 */

#ifndef __CLK_RTL960X_H
#define __CLK_RTL960X_H

/*
 * RTL9607C SoC register base
 */
#define RTL960X_SOC_BASE		(0xB8000000)

/*
 * CPU PLL registers
 */
#define RTL960X_OCP_PLL_CTRL0		(0x0200)
#define RTL960X_OCP_PLL_CTRL3		(0x020C)
#define RTL960X_CMU_GCR			(0x0380)

/*
 * OCP_PLL_CTRL0 bit fields
 * Bits 21:16 = cpu_freq_sel0
 */
#define RTL960X_CPU_FREQ_SEL0_SHIFT	16
#define RTL960X_CPU_FREQ_SEL0_MASK	0x3F

/*
 * OCP_PLL_CTRL3 bit fields
 * Bit 18 = en_DIV2_cpu0
 */
#define RTL960X_EN_DIV2_CPU0_SHIFT	18
#define RTL960X_EN_DIV2_CPU0_MASK	0x1

/*
 * CMU_GCR bit fields
 * Bits 1:0 = cmu_mode
 * Bits 6:4 = freq_div
 */
#define RTL960X_CMU_MODE_SHIFT		0
#define RTL960X_CMU_MODE_MASK		0x3
#define RTL960X_FREQ_DIV_SHIFT		4
#define RTL960X_FREQ_DIV_MASK		0x7

/*
 * LX PLL register
 */
#define RTL960X_LX_PLL_BASE		(0xBB01F000)
#define RTL960X_LX_PLL_CTRL		(0x0054)

/*
 * LX_PLL_CTRL bit fields
 * Bits 3:0 = lx_freq_sel
 */
#define RTL960X_LX_FREQ_SEL_MASK	0xF

/*
 * CPU frequency formula:
 *   cpu_mhz = ((cpu_freq_sel0 + 2) * 50) / (1 << en_DIV2_cpu0)
 *   if (cmu_mode != 0) cpu_mhz /= (1 << freq_div)
 *
 * LX frequency formula:
 *   lx_mhz = 1000 / (lx_freq_sel + 5)
 */

#endif /* __CLK_RTL960X_H */
