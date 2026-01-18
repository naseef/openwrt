// SPDX-License-Identifier: GPL-2.0-only
/*
 * Realtek RTL960X clock driver
 * Copyright (C) 2026 Ahmed Naseef <naseefkm@gmail.com>
 *
 * Based on OEM code and clk-rtl83xx.c
 *
 * This driver provides clock support for the RTL960x SoC. It reads the PLL
 * registers to determine the actual CPU and LX bus frequencies at runtime,
 * eliminating the need for hardcoded values in the device tree.
 */

#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include <dt-bindings/clock/rtl960x-clk.h>
#include "clk-rtl960x.h"

/*
 * Register access macros using KSEG1 addresses directly
 */
#define read_soc(reg)	ioread32((void *)RTL960X_SOC_BASE + (reg))
#define read_lx(reg)	ioread32((void *)RTL960X_LX_PLL_BASE + (reg))

struct rtl960x_clk {
	struct clk_hw hw;
	unsigned int idx;
};

struct rtl960x_ccu {
	struct clk_hw_onecell_data *hw_data;
	struct rtl960x_clk clks[CLK_COUNT];
};

static struct rtl960x_ccu *rtl960x_ccu;

static const char * const rtl960x_clk_names[CLK_COUNT] = {
	"cpu_clk",
	"lxb_clk",
};

#define to_rtl960x_clk(_hw) container_of(_hw, struct rtl960x_clk, hw)

/*
 * Calculate CPU frequency from PLL registers
 * Formula: cpu_mhz = ((cpu_freq_sel0 + 2) * 50) / (1 << en_DIV2_cpu0)
 *          if (cmu_mode != 0) cpu_mhz /= (1 << freq_div)
 */
static unsigned long rtl960x_cpu_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	u32 ocp_pll_ctrl0, ocp_pll_ctrl3, cmu_gcr;
	u32 cpu_freq_sel0, en_div2_cpu0, cmu_mode, freq_div;
	unsigned long rate;

	ocp_pll_ctrl0 = read_soc(RTL960X_OCP_PLL_CTRL0);
	ocp_pll_ctrl3 = read_soc(RTL960X_OCP_PLL_CTRL3);
	cmu_gcr = read_soc(RTL960X_CMU_GCR);

	cpu_freq_sel0 = (ocp_pll_ctrl0 >> RTL960X_CPU_FREQ_SEL0_SHIFT) &
			RTL960X_CPU_FREQ_SEL0_MASK;
	en_div2_cpu0 = (ocp_pll_ctrl3 >> RTL960X_EN_DIV2_CPU0_SHIFT) &
		       RTL960X_EN_DIV2_CPU0_MASK;
	cmu_mode = (cmu_gcr >> RTL960X_CMU_MODE_SHIFT) & RTL960X_CMU_MODE_MASK;
	freq_div = (cmu_gcr >> RTL960X_FREQ_DIV_SHIFT) & RTL960X_FREQ_DIV_MASK;

	/* Calculate frequency in MHz */
	rate = ((cpu_freq_sel0 + 2) * 50) / (1 << en_div2_cpu0);
	if (cmu_mode != 0)
		rate /= (1 << freq_div);

	/* Convert MHz to Hz */
	rate *= 1000000;

	return rate;
}

/*
 * Calculate LX bus frequency from PLL register
 * Formula: lx_mhz = 1000 / (lx_freq_sel + 5)
 */
static unsigned long rtl960x_lxb_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	u32 lx_pll_ctrl, lx_freq_sel;
	unsigned long rate;

	lx_pll_ctrl = read_lx(RTL960X_LX_PLL_CTRL);
	lx_freq_sel = lx_pll_ctrl & RTL960X_LX_FREQ_SEL_MASK;

	/* Calculate frequency in MHz, then convert to Hz */
	rate = 1000 / (lx_freq_sel + 5);
	rate *= 1000000;

	return rate;
}

static unsigned long rtl960x_recalc_rate(struct clk_hw *hw,
					 unsigned long parent_rate)
{
	struct rtl960x_clk *clk = to_rtl960x_clk(hw);

	switch (clk->idx) {
	case CLK_CPU:
		return rtl960x_cpu_recalc_rate(hw, parent_rate);
	case CLK_LXB:
		return rtl960x_lxb_recalc_rate(hw, parent_rate);
	default:
		return 0;
	}
}

static const struct clk_ops rtl960x_clk_ops = {
	.recalc_rate = rtl960x_recalc_rate,
};

static void __init rtl960x_ccu_probe(struct device_node *np)
{
	struct clk_hw_onecell_data *hw_data;
	struct clk_init_data init = {};
	int i, ret;

	rtl960x_ccu = kzalloc(sizeof(*rtl960x_ccu), GFP_KERNEL);
	if (!rtl960x_ccu)
		return;

	hw_data = kzalloc(struct_size(hw_data, hws, CLK_COUNT), GFP_KERNEL);
	if (!hw_data) {
		kfree(rtl960x_ccu);
		return;
	}

	rtl960x_ccu->hw_data = hw_data;
	hw_data->num = CLK_COUNT;

	/* Register all clocks */
	for (i = 0; i < CLK_COUNT; i++) {
		init.name = rtl960x_clk_names[i];
		init.ops = &rtl960x_clk_ops;
		init.flags = 0;
		init.num_parents = 0;

		rtl960x_ccu->clks[i].hw.init = &init;
		rtl960x_ccu->clks[i].idx = i;

		ret = clk_hw_register(NULL, &rtl960x_ccu->clks[i].hw);
		if (ret) {
			pr_err("rtl960x-clk: failed to register %s\n",
			       rtl960x_clk_names[i]);
			goto err_free;
		}

		hw_data->hws[i] = &rtl960x_ccu->clks[i].hw;

		clk_hw_register_clkdev(&rtl960x_ccu->clks[i].hw,
				       rtl960x_clk_names[i], NULL);
	}

	ret = of_clk_add_hw_provider(np, of_clk_hw_onecell_get, hw_data);
	if (ret) {
		pr_err("rtl960x-clk: failed to register clock provider\n");
		goto err_free;
	}

	pr_info("rtl960x-clk: CPU: %lu MHz, LXB: %lu MHz\n",
		clk_hw_get_rate(&rtl960x_ccu->clks[CLK_CPU].hw) / 1000000,
		clk_hw_get_rate(&rtl960x_ccu->clks[CLK_LXB].hw) / 1000000);

	return;

err_free:
	kfree(hw_data);
	kfree(rtl960x_ccu);
	rtl960x_ccu = NULL;
}

CLK_OF_DECLARE_DRIVER(rtl960x_clk, "realtek,rtl960x-clock", rtl960x_ccu_probe);
