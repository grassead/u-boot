// SPDX-License-Identifier: GPL-2.0
/*
 * Generic DWC3 for i.MX8MQ
 *
 * Based on dwc3-generic.c.
 */
#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dwc3-uboot.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <malloc.h>
#include <usb.h>
#include "core.h"
#include "gadget.h"
#include <reset.h>
#include <clk.h>
#include <usb/xhci.h>

struct dwc3_imx8mq_data {
	struct clk_bulk		clks;
};

struct dwc3_imx8mq_priv {
	void *base;
	struct dwc3 dwc3;
};

struct dwc3_imx8mq_plat {
	fdt_addr_t base;
	u32 maximum_speed;
	enum usb_dr_mode dr_mode;
};

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct dwc3_imx8mq_priv *priv = dev_get_priv(dev);
	struct dwc3 *dwc3 = &priv->dwc3;

	dwc3_gadget_uboot_handle_interrupt(dwc3);
	return 0;
}

static int dwc3_imx8mq_of_to_plat(struct udevice *dev)
{
	struct dwc3_imx8mq_plat *plat = dev_get_plat(dev);
	ofnode node = dev_ofnode(dev->parent);

	plat->base = dev_read_addr(dev->parent);

	plat->maximum_speed = usb_get_maximum_speed(node);
	if (plat->maximum_speed == USB_SPEED_UNKNOWN) {
		pr_info("No USB maximum speed specified. Using super speed\n");
		plat->maximum_speed = USB_SPEED_SUPER;
	}

	plat->dr_mode = usb_get_dr_mode(node);
	if (plat->dr_mode == USB_DR_MODE_UNKNOWN) {
		pr_err("Invalid usb mode setup\n");
		return -ENODEV;
	}

	return 0;
}

static int dwc3_imx8mq_peripheral_probe(struct udevice *dev)
{
	int rc;
	struct dwc3_imx8mq_priv *priv = dev_get_priv(dev);
	struct dwc3_imx8mq_plat *plat = dev_get_plat(dev);
	struct dwc3 *dwc3 = &priv->dwc3;

	dwc3->dev = dev;
	dwc3->maximum_speed = plat->maximum_speed;
	dwc3->dr_mode = plat->dr_mode;

#if CONFIG_IS_ENABLED(OF_CONTROL)
	dwc3_of_parse(dwc3);
#endif

	priv->base = map_physmem(plat->base, DWC3_OTG_REGS_END, MAP_NOCACHE);
	dwc3->regs = priv->base + DWC3_GLOBALS_REGS_START;

	rc =  dwc3_init(dwc3);
	if (rc) {
		unmap_physmem(priv->base, MAP_NOCACHE);
		return rc;
	}

	return 0;
}

static int dwc3_imx8mq_peripheral_remove(struct udevice *dev)
{
	printf("%s\n", __func__);
	return 0;
}

U_BOOT_DRIVER(dwc3_imx8mq_peripheral) = {
	.name	= "dwc3-imx8mq-peripheral",
	.id	= UCLASS_USB_GADGET_GENERIC,
	.of_to_plat = dwc3_imx8mq_of_to_plat,
	.probe = dwc3_imx8mq_peripheral_probe,
	.remove = dwc3_imx8mq_peripheral_remove,
	.priv_auto	= sizeof(struct dwc3_imx8mq_priv),
	.plat_auto	= sizeof(struct dwc3_imx8mq_plat),
};

static int dwc3_imx8mq_bind(struct udevice *dev)
{
	ofnode node;
	int ret = 0;
	enum usb_dr_mode dr_mode;
	const char* driver_name;
	struct udevice *reg_dev;

	node = dev_ofnode(dev);
	dr_mode = usb_get_dr_mode(node);

	switch (dr_mode) {
		case USB_DR_MODE_PERIPHERAL:
		case USB_DR_MODE_OTG:
			driver_name = "dwc3-imx8mq-peripheral";
			break;
		case USB_DR_MODE_HOST:
			break;
		default:
			return -EINVAL;
	}

	ret = device_bind_driver_to_node(dev, driver_name, ofnode_get_name(node),
						 node, &reg_dev);

	return ret;
}

static int dwc3_imx8mq_clk_init(struct udevice *dev,
			      struct dwc3_imx8mq_data *data)
{
	int ret;

	ret = clk_get_bulk(dev, &data->clks);
	if (ret == -ENOSYS || ret == -ENOENT)
		return 0;
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable_bulk(&data->clks);
	if (ret) {
		clk_release_bulk(&data->clks);
		return ret;
	}
#endif

	return 0;
}

static int dwc3_imx8mq_probe(struct udevice *dev)
{
	struct dwc3_imx8mq_data *data = dev_get_plat(dev);
	int ret;

	ret = dwc3_imx8mq_clk_init(dev, data);
	if (ret) {
		printf("Failed to init clocks !\n");
		return ret;
	}

	return 0;
}

static int dwc3_imx8mq_remove(struct udevice *dev)
{
	printf("%s\n", __func__);
	return 0;
}

static const struct udevice_id dwc3_imx8mq_ids[] = {
	{ .compatible = "fsl,imx8mq-dwc3" },
	{ }
};

U_BOOT_DRIVER(dwc3_imx8mq_wrapper) = {
	.name	= "dwc3-imx8mq-wrapper",
	.id	= UCLASS_NOP,
	.of_match = dwc3_imx8mq_ids,
	.bind = dwc3_imx8mq_bind,
	.probe = dwc3_imx8mq_probe,
	.remove = dwc3_imx8mq_remove,
	.plat_auto	= sizeof(struct dwc3_imx8mq_data),
};
