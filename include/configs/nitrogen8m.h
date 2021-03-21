/*
 * Copyright 2018 Boundary Devices
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __NITROGEN8M_H
#define __NITROGEN8M_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K

#ifdef CONFIG_SPL_BUILD

#define CONFIG_SPL_STACK		0x00187FF0
#define CONFIG_SPL_BSS_START_ADDR	0x00180000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K

#define CONFIG_MALLOC_F_ADDR		0x00900000	/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */

#define CONFIG_SYS_SPL_FIT_USE_BUFFER_INPLACE

#endif

/* Link Definitions */
#define CONFIG_LOADADDR			0x40480000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x200000
#define CONFIG_SYS_INIT_SP_OFFSET \
        (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
        (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(128 * SZ_1M)

#define CONFIG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			(CONFIG_DDR_MB * 1024ULL * 1024ULL)

/* Console configuration */
#define CONFIG_BAUDRATE			115200
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE              2048
#define CONFIG_SYS_MAXARGS             64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* I2C Configs */
#define CONFIG_SYS_I2C_SPEED		  100000

#define BD_CONSOLE	"ttymxc0"

#define DISTRO_BOOT_DEV_MMC(func) func(MMC, mmc, 0)

#define BOOT_TARGET_DEVICES(func) \
	DISTRO_BOOT_DEV_MMC(func)

#include <config_distro_bootcmd.h>

#define BD_RAM_BASE	0x80000000
#define BD_RAM_SCRIPT	"40020000"
#define BD_RAM_KERNEL	"40800000"
#define BD_RAM_RAMDISK	"42800000"
#define BD_RAM_FDT	"43000000"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=" BD_CONSOLE "\0" \
	"env_dev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"env_part=" __stringify(CONFIG_SYS_MMC_ENV_PART) "\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"fdt_addr=0x43000000\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"imx_cpu=8MQ\0" \
	"upgradeu=setenv boot_scripts upgrade.scr; boot;" \
		"echo Upgrade failed!; setenv boot_scripts boot.scr\0" \
	"uboot_defconfig=nitrogen8m\0" \
	BOOTENV

#endif
