/*
 * Copyright (C) 2017, Boundary Devices <info@boundarydevices.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <asm/arch-imx8m/imx8mq_pins.h>
#include <asm/global_data.h>
#include <asm/mach-imx/iomux-v3.h>
#include <errno.h>
#include <common.h>
#include <dwc3-uboot.h>
#include <linux/usb/dwc3.h>
#include <usb.h>
#include <asm/io.h>
#include <dm.h>
#include <power-domain.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL (PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static iomux_v3_cfg_t const init_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};


int board_init(void)
{
	printf("Current EL is %u\n", current_el());
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(init_pads, ARRAY_SIZE(init_pads));
	return 0;
}


