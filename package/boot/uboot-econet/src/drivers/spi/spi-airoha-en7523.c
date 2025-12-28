// SPDX-License-Identifier: GPL-2.0+
/*
 * Airoha EN7523 SPI Controller driver for U-Boot
 *
 * Used for EcoNet EN751221 and Airoha EN7523 SoCs.
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/bitops.h>

/* Register offsets from SPI base */
#define ENSPI_READ_IDLE_EN		0x0004
#define ENSPI_MTX_MODE_TOG		0x0014
#define ENSPI_RDCTL_FSM			0x0018
#define ENSPI_MANUAL_EN			0x0020
#define ENSPI_OPFIFO_EMPTY		0x0024
#define ENSPI_OPFIFO_WDATA		0x0028
#define ENSPI_OPFIFO_FULL		0x002c
#define ENSPI_OPFIFO_WR			0x0030
#define ENSPI_DFIFO_FULL		0x0034
#define ENSPI_DFIFO_WDATA		0x0038
#define ENSPI_DFIFO_EMPTY		0x003c
#define ENSPI_DFIFO_RD			0x0040
#define ENSPI_DFIFO_RDATA		0x0044

/* Operation codes for OPFIFO */
#define OP_CSH				0x00	/* Chip select high */
#define OP_CSL				0x01	/* Chip select low */
#define OP_CK				0x02	/* Clock cycles */
#define OP_OUTS				0x08	/* Output single (MOSI) */
#define OP_INS				0x0c	/* Input single (MISO) */

#define OP_CMD_SHIFT			9
#define OP_LEN_MASK			0x1ff
#define OP_LEN_MAX			511

/* Mode values */
#define MTX_MODE_MANUAL			0x09

#define DFIFO_MASK			0xff

struct en7523_spi_priv {
	void __iomem *base;
	bool manual_mode;
};

static void opfifo_write(struct en7523_spi_priv *priv, u32 cmd, u32 len)
{
	u32 val = ((cmd & 0x1f) << OP_CMD_SHIFT) | (len & OP_LEN_MASK);

	writel(val, priv->base + ENSPI_OPFIFO_WDATA);

	/* Wait for room in OPFIFO */
	while (readl(priv->base + ENSPI_OPFIFO_FULL))
		;

	/* Trigger write */
	writel(1, priv->base + ENSPI_OPFIFO_WR);

	/* Wait for command to finish */
	while (!readl(priv->base + ENSPI_OPFIFO_EMPTY))
		;
}

/*
 * Set chip select state
 *
 * EN751221 drops writes if we don't send chip select twice.
 */
static void set_cs(struct en7523_spi_priv *priv, int state)
{
	u32 cmd = state ? OP_CSH : OP_CSL;

	opfifo_write(priv, cmd, 1);
	opfifo_write(priv, cmd, 1);
}

static void manual_begin_cmd(struct en7523_spi_priv *priv)
{
	if (priv->manual_mode)
		return;

	/* Disable read idle state */
	writel(0, priv->base + ENSPI_READ_IDLE_EN);

	/* Wait for FSM to idle */
	while (readl(priv->base + ENSPI_RDCTL_FSM))
		;

	/* Switch to manual mode */
	writel(MTX_MODE_MANUAL, priv->base + ENSPI_MTX_MODE_TOG);

	/* Enable manual mode */
	writel(1, priv->base + ENSPI_MANUAL_EN);

	priv->manual_mode = true;
}

static void dfifo_write(struct en7523_spi_priv *priv, const u8 *buf, u32 len)
{
	u32 i;

	for (i = 0; i < len; i++) {
		while (readl(priv->base + ENSPI_DFIFO_FULL))
			;
		writel(buf[i] & DFIFO_MASK, priv->base + ENSPI_DFIFO_WDATA);
		while (readl(priv->base + ENSPI_DFIFO_FULL))
			;
	}
}

static void dfifo_read(struct en7523_spi_priv *priv, u8 *buf, u32 len)
{
	u32 i;

	for (i = 0; i < len; i++) {
		while (readl(priv->base + ENSPI_DFIFO_EMPTY))
			;
		buf[i] = readl(priv->base + ENSPI_DFIFO_RDATA) & DFIFO_MASK;
		writel(1, priv->base + ENSPI_DFIFO_RD);
	}
}

static int xfer_write(struct en7523_spi_priv *priv, const u8 *buf, u32 len)
{
	u32 chunk_len;
	u32 offset = 0;

	while (len > 0) {
		chunk_len = min_t(u32, len, OP_LEN_MAX);
		opfifo_write(priv, OP_OUTS, chunk_len);
		dfifo_write(priv, buf + offset, chunk_len);
		offset += chunk_len;
		len -= chunk_len;
	}

	return 0;
}

static int xfer_read(struct en7523_spi_priv *priv, u8 *buf, u32 len)
{
	u32 chunk_len;
	u32 offset = 0;

	while (len > 0) {
		chunk_len = min_t(u32, len, OP_LEN_MAX);
		opfifo_write(priv, OP_INS, chunk_len);
		dfifo_read(priv, buf + offset, chunk_len);
		offset += chunk_len;
		len -= chunk_len;
	}

	return 0;
}

static int en7523_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct en7523_spi_priv *priv = dev_get_priv(bus);
	int total_size = bitlen >> 3;
	int ret = 0;

	debug("%s: dout=%p, din=%p, len=%x, flags=%lx\n", __func__, dout, din,
	      total_size, flags);

	if (dout && din) {
		printf("en7523_spi: only half-duplex SPI supported\n");
		return -EINVAL;
	}

	manual_begin_cmd(priv);

	if (flags & SPI_XFER_BEGIN)
		set_cs(priv, 0);  /* CS low = active */

	if (din) {
		ret = xfer_read(priv, din, total_size);
	} else if (dout) {
		ret = xfer_write(priv, dout, total_size);
	}

	if (flags & SPI_XFER_END)
		set_cs(priv, 1);  /* CS high = inactive */

	return ret;
}

static int en7523_spi_set_speed(struct udevice *bus, uint speed)
{
	/* Speed is fixed by hardware */
	debug("%s: speed=%d (ignored)\n", __func__, speed);
	return 0;
}

static int en7523_spi_set_mode(struct udevice *bus, uint mode)
{
	/* Only mode 0 supported */
	debug("%s: mode=0x%x\n", __func__, mode);
	if (mode & (SPI_CPOL | SPI_CPHA)) {
		printf("Only SPI mode 0 supported\n");
		return -EINVAL;
	}
	return 0;
}

static int en7523_spi_probe(struct udevice *dev)
{
	struct en7523_spi_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	priv->manual_mode = false;

	debug("%s: base=%p\n", __func__, priv->base);
	return 0;
}

static const struct dm_spi_ops en7523_spi_ops = {
	.xfer = en7523_spi_xfer,
	.set_speed = en7523_spi_set_speed,
	.set_mode = en7523_spi_set_mode,
};

static const struct udevice_id en7523_spi_ids[] = {
	{ .compatible = "airoha,en7523-spi" },
	{ }
};

U_BOOT_DRIVER(en7523_spi) = {
	.name = "en7523_spi",
	.id = UCLASS_SPI,
	.of_match = en7523_spi_ids,
	.ops = &en7523_spi_ops,
	.priv_auto = sizeof(struct en7523_spi_priv),
	.probe = en7523_spi_probe,
};
