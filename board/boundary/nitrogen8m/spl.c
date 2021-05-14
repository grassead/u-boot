/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <spl.h>
#include <errno.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/ddr.h>
#include <power/regulator.h>
#include <dm.h>
#include <asm/mach-imx/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

static inline unsigned int current_el(void)
{
	unsigned long el;

	asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
	return 3 & (el >> 2);
}

#define I2C_PAD_CTRL    (PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
        .scl = {
                .i2c_mode = IMX8MQ_PAD_I2C1_SCL__I2C1_SCL | PC,
                .gpio_mode = IMX8MQ_PAD_I2C1_SCL__GPIO5_IO14 | PC,
                .gp = IMX_GPIO_NR(5, 14),
        },
        .sda = {
                .i2c_mode = IMX8MQ_PAD_I2C1_SDA__I2C1_SDA | PC,
                .gpio_mode = IMX8MQ_PAD_I2C1_SDA__GPIO5_IO15 | PC,
                .gp = IMX_GPIO_NR(5, 15),
        },
};

void ddr_voltage_init(void)
{
	int ret;
	struct udevice *reg;

	ret = uclass_get_device_by_name(UCLASS_REGULATOR, "reg-arm-dram@60", &reg);
	if (ret) {
		printf("Failed to get reg-arm-dram regulator\n");
		return;
	}
	printf("device name = %s\n", reg->name);
	ret = regulator_set_value(reg, 1000000);
	if (ret) {
		printf("Failed to set reg-arm-dram regulator value\n");
		return;
	}

	ret = uclass_get_device_by_name(UCLASS_REGULATOR, "reg-dram-1p1v@60", &reg);
	if (ret) {
		printf("Failed to get reg-dram-1p1v regulator\n");
		return;
	}

	printf("device name = %s\n", reg->name);
	ret = regulator_set_value(reg, 1100000);
	if (ret) {
		printf("Failed to set reg-dram-1p1v value\n");
		return;
	}

	ret = uclass_get_device_by_name(UCLASS_REGULATOR, "reg-soc-gpu-vpu@60", &reg);
	if (ret) {
		printf("Failed to get reg-soc-gpu-vpu regulator\n");
		return;
	}

	printf("device name = %s\n", reg->name);
	ret = regulator_set_value(reg, 900000);
	if (ret) {
		printf("Failed to set reg-soc-gpu-vpu value\n");
		return;
	}
}

int power_init_board(void)
{
	dm_dump_all();
	/* Nitrogen8M I2C write */
	ddr_voltage_init();
	return 0;
}

void spl_board_init(void)
{
	printf("Current EL is %u\n", current_el());
	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	pr_debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

static void spl_dram_init(void)
{
	struct dram_timing_info *dt = &dram_timing;
	printf("ddr_init ...\n");
	int ret = ddr_init(dt);

	if (ret) {
		printf("%s:error\n", __func__);
		//hang();
	}
	printf("ddr_init done\n");
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	arch_cpu_init();

	init_uart_clk(0);

	board_early_init_f();

	timer_init();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	printf("Current EL is %u\n", current_el());

	printf("spl_early_init ...\n");
	ret = spl_early_init();
	if (ret) {
		printf("spl_init() failed: %d\n", ret);
		hang();
	}

	printf("spl_early_init done\n");

	enable_tzc380();

	printf("enable_tzc380 done\n");

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	printf("setup_i2c done\n");

	power_init_board();

	printf("power_init_board done\n");

	/* DDR initialization */
	spl_dram_init();

	printf("spl_dram_init done\n");

	board_init_r(NULL, 0);
}
