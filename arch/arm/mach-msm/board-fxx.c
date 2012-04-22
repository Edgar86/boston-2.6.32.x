/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/power_supply.h>


#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/setup.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif

#include <asm/mach/mmc.h>
#include <mach/vreg.h>
#include <mach/mpp.h>
#include <mach/gpio.h>
#include <mach/board.h>
#include <mach/pmic.h>
#include <mach/msm_iomap.h>
#include <mach/msm_rpcrouter.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#include <mach/rpc_pmapp.h>
#include <mach/msm_serial_hs.h>
#include <mach/memory.h>
#include <mach/msm_battery.h>
#include <mach/rpc_server_handset.h>
#include <mach/msm_tsif.h>

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/android_pmem.h>
#include <mach/camera.h>

#include <linux/elan_i2c.h>  //Added for capacitive touch panel, by Stanley++ 2009/05/20
#include <linux/focal_i2c.h>  //Added for capacitive touch panel, by Stanley++ 2010/03/03
#include <linux/switch.h> 
#include "devices.h"
#include "socinfo.h"
#include "clock.h"
#include "msm-keypad-devices.h"
#include <mach/tca6507.h>
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif
#include "pm.h"
#ifdef CONFIG_ARCH_MSM7X27
#include <linux/msm_kgsl.h>
#endif
#include "proc_comm.h"
#ifdef CONFIG_SPI_GPIO  

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/spi/spi_gpio.h>

#endif 
#include <mach/7x27_kybd.h>
#include "proc_comm.h"
#include <linux/msm_kgsl.h>

#include "smd_private.h"
#include <mach/msm_smd.h>
void __init msm_power_register(void);
#define MSM_PMEM_MDP_SIZE	0x92C000
//Modificado para mayor memoria en grabación de videos de la camara
#define MSM_PMEM_ADSP_SIZE	0x986000
#define MSM_FB_SIZE	0x177000
#define PMEM_KERNEL_EBI1_SIZE 0x200000
#define MSM_PMEM_AUDIO_SIZE	0x5B000
#define MSM_GPU_PHYS_SIZE	SZ_2M
#define MSM_PMEM_AUDIO_START_ADDR	0x80000ul
char *board_serial;

#ifdef CONFIG_AR6K
#define WIFI_CONTROL_MASK   0x10000000
#define WIFI_SUSPEND_CONTROL_MASK   0x01000000
static DEFINE_SPINLOCK(wif_bt_lock);
#endif

#if defined(CONFIG_BT) || defined(CONFIG_AR6K)
static int wifi_status = 0;
static int bt_status = 0;
#define MODULE_TURN_ON      0x01
#define MODULE_TURN_OFF     0x02
#endif

#ifdef CONFIG_USB_ANDROID
static char *usb_functions_default[] = {
	"usb_mass_storage",
};

static char *usb_functions_default_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};
static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_RMNET
	"rmnet",
#endif
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif
};

static struct android_usb_product usb_products[] = {
    {
        .product_id = 0xC004,
        .num_functions  = ARRAY_SIZE(usb_functions_default),
        .functions  = usb_functions_default,
    },
    {
        .product_id = 0xC001,
        .num_functions  = ARRAY_SIZE(usb_functions_default_adb),
        .functions  = usb_functions_default_adb,
    },
    {
        .product_id = 0xC008,
        .num_functions  = ARRAY_SIZE(usb_functions_rndis),
        .functions  = usb_functions_rndis,
    },
    {
        .product_id = 0xC007,
        .num_functions  = ARRAY_SIZE(usb_functions_rndis_adb),
        .functions  = usb_functions_rndis_adb,
    },
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns      = 1,
	.vendor     = "Android",
	.product    = "Mass storage",
	.release    = 0x0100,
	.can_stall  = 0,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

static struct usb_ether_platform_data rndis_pdata = {
    /* ethaddr is filled by board_serialno_setup */
    .vendorID   = 0x05C6,
    .vendorDescr    = "Qualcomm Incorporated",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id  = 0x489,
	.product_id = 0xC004,
	.version	= 0x0100,
	.product_name	= "Qualcomm HSUSB Device",
	.manufacturer_name = "Qualcomm Incorporated",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.serial_number = "1234567890ABCDEF",
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

static int __init board_serialno_setup(char *serialno)
{
	int i;
	char *src = serialno;

	/* create a fake MAC address from our serial number.
	 * first byte is 0x02 to signify locally administered.
	 */
	rndis_pdata.ethaddr[0] = 0x02;
	for (i = 0; *src; i++) {
		/* XOR the USB serial across the remaining bytes */
		rndis_pdata.ethaddr[i % (ETH_ALEN - 1) + 1] ^= *src++;
	}

	android_usb_pdata.serial_number = serialno;
	return 1;
}

#endif

#ifdef CONFIG_USB_EHCI_MSM
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	if (on)
		msm_hsusb_vbus_powerup();
	else
		msm_hsusb_vbus_shutdown();
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info       = (USB_PHY_INTEGRATED | USB_PHY_MODEL_65NM),
};

static void __init msm7x2x_init_host(void)
{
	if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa())
		return;

	msm_add_host(0, &msm_usb_host_pdata);
}
#endif

static int hsusb_rpc_connect(int connect)
{
	if (connect)
		return msm_hsusb_rpc_connect();
	else
		return msm_hsusb_rpc_close();
}

struct vreg *vreg_3p3;
static int msm_hsusb_ldo_init(int init)
{
    if (init) {
        /*
         * PHY 3.3V analog domain(VDDA33) is powered up by
         * an always enabled power supply (LP5900TL-3.3).
         * USB VREG default source is VBUS line. Turning
         * on USB VREG has a side effect on the USB suspend
         * current. Hence USB VREG is explicitly turned
         * off here.
         */
        vreg_3p3 = vreg_get(NULL, "usb");
        if (IS_ERR(vreg_3p3))
            return PTR_ERR(vreg_3p3);
        vreg_enable(vreg_3p3);         
        vreg_disable(vreg_3p3);  
        vreg_put(vreg_3p3);
    }

    return 0;
}

static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init)
{
    int ret;

    if (init) {
        ret = msm_pm_app_rpc_init(callback);
    } else {
        msm_pm_app_rpc_deinit(callback);
        ret = 0;
    }
    return ret;
}

#ifdef CONFIG_BATTERY_FIH_ZEUS
#define ZEUS_GPIO_BATTERY_USBSET 123

extern void notify_usb_connected(int);
void charger_connected(enum chg_type chgtype)
{
    notify_usb_connected(chgtype);
    gpio_set_value(ZEUS_GPIO_BATTERY_USBSET, (chgtype != USB_CHG_TYPE__INVALID));
    hsusb_chg_connected(chgtype);
}

#endif

static struct msm_otg_platform_data msm_otg_pdata = {
    .rpc_connect    = hsusb_rpc_connect,
    .pmic_vbus_notif_init         = msm_hsusb_pmic_notif_init,
    .chg_vbus_draw       = hsusb_chg_vbus_draw,
#ifdef CONFIG_BATTERY_FIH_ZEUS
    .chg_connected       = charger_connected,
#endif
#ifdef CONFIG_USB_EHCI_MSM
    .vbus_power = msm_hsusb_vbus_power,
#endif
    .chg_init        = hsusb_chg_init,
    .ldo_init       = msm_hsusb_ldo_init,
    .pclk_required_during_lpm = 1,
    .pclk_src_name          = "ebi1_usb_clk",
};

static struct msm_hsusb_gadget_platform_data msm_gadget_pdata;

#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET, 0),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(TTY_HEADSET, 8),
	SND(TTY_VCO, 9),
	SND(TTY_HCO, 10),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(HEADSET_AND_SPEAKER, 26),
	SND(HEADSET_WITH_INNER_MIC, 27),
	SND(CURRENT, 29),
};
#undef SND

static struct msm_snd_endpoints msm_device_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

static struct platform_device msm_device_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_snd_endpoints
	},
};

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_WAV)|(1<<MSM_ADSP_CODEC_ADPCM)| \
	(1<<MSM_ADSP_CODEC_YADPCM)|(1<<MSM_ADSP_CODEC_QCELP)| \
	(1<<MSM_ADSP_CODEC_MP3))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_WAV)|(1<<MSM_ADSP_CODEC_ADPCM)| \
	(1<<MSM_ADSP_CODEC_YADPCM)|(1<<MSM_ADSP_CODEC_QCELP)| \
	(1<<MSM_ADSP_CODEC_MP3))

#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_WAV)|(1<<MSM_ADSP_CODEC_ADPCM)| \
	(1<<MSM_ADSP_CODEC_YADPCM)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DMA)), 0,
	0, 0, 0,

	/* Concurrency 1 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	 /* Concurrency 2 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 3 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 4 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 5 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
        0, 0, 0, 0,

        /* Concurrency 7 */
        (DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 6 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 5),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 5),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 4),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

static struct android_pmem_platform_data android_pmem_kernel_ebi1_pdata = {
	.name = PMEM_KERNEL_EBI1_DATA_NAME,
	/* if no allocator_type, defaults to PMEM_ALLOCATORTYPE_BITMAP,
	 * the only valid choice at this time. The board structure is
	 * set to all zeros by the C runtime initialization and that is now
	 * the enum value of PMEM_ALLOCATORTYPE_BITMAP, now forced to 0 in
	 * include/linux/android_pmem.h.
	 */
	.cached = 0,
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};

static struct platform_device android_pmem_kernel_ebi1_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_kernel_ebi1_pdata },
};

static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

static struct platform_device mtb_platform_device = {
	.name = "mtb",
};

static struct gpio_switch_platform_data headset_sensor_device_data = {
	.name = "h2w",
	.gpio = 40,
	.name_on = "",
	.name_off = "",
	.state_on = "0",
	.state_off = "1",
};
	
static struct platform_device headset_sensor_device = {
	.name = "switch_gpio",
	.id	= -1,
	.dev = { .platform_data = &headset_sensor_device_data },
};

static struct platform_device ALSPS_sensor_device = {
	.name = "cm3602_alsps",

};

/* TSIF begin */
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)

#define TSIF_B_SYNC      GPIO_CFG(87, 5, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_DATA      GPIO_CFG(86, 3, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_EN        GPIO_CFG(85, 3, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)
#define TSIF_B_CLK       GPIO_CFG(84, 4, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA)

static const struct msm_gpio tsif_gpios[] = {
	{ .gpio_cfg = TSIF_B_CLK,  .label =  "tsif_clk", },
	{ .gpio_cfg = TSIF_B_EN,   .label =  "tsif_en", },
	{ .gpio_cfg = TSIF_B_DATA, .label =  "tsif_data", },
	{ .gpio_cfg = TSIF_B_SYNC, .label =  "tsif_sync", },
};

static struct msm_tsif_platform_data tsif_platform_data = {
	.num_gpios = ARRAY_SIZE(tsif_gpios),
	.gpios = tsif_gpios,
	.tsif_clk = "tsif_clk",
	.tsif_pclk = "tsif_pclk",
	.tsif_ref_clk = "tsif_ref_clk",
};
#endif /* defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE) */
/* TSIF end   */

#define LCDC_CONFIG_PROC          21
#define LCDC_UN_CONFIG_PROC       22
#define LCDC_API_PROG             0x30000066
#define LCDC_API_VERS             0x00010001

#define GPIO_OUT_132    132
#define GPIO_OUT_131    131
#define GPIO_OUT_103    103
#define GPIO_OUT_102    102
#define GPIO_OUT_88     88

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	int ret = -EPERM;

	return ret;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
	.mddi_prescan = 1,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resource[] = {
        {
            .flags  = IORESOURCE_MEM,
        }
};

static struct platform_device ram_console_device = {
        .name = "ram_console",
        .id = -1,
        .num_resources  = ARRAY_SIZE(ram_console_resource),
        .resource       = ram_console_resource,
};
#endif

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd);

static int msm_read_serial_number_from_nvitem(void)
{
	uint32_t smem_proc_comm_oem_cmd1 = PCOM_CUSTOMER_CMD1;
	uint32_t smem_proc_comm_oem_data1 = SMEM_PROC_COMM_OEM_PRODUCT_ID_READ;
	uint32_t smem_proc_comm_oem_data2 = 0;
	uint32_t product_id[40];	
	board_serial = kzalloc(64, GFP_KERNEL);

	if(msm_proc_comm_oem(smem_proc_comm_oem_cmd1, &smem_proc_comm_oem_data1, product_id, &smem_proc_comm_oem_data2) == 0)
	{      
		memcpy(board_serial, product_id, 16);
	} 	
#ifdef CONFIG_USB_ANDROID
	board_serialno_setup(board_serial);
#endif
	return 1;
}

#ifdef CONFIG_AR6K
/* FIH, JamesKCTung, 2009/05/11 { */
static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd);
static uint32_t msm_ar6k_sdcc_setup_power(struct device *dv, unsigned int vdd);
int static ar6k_wifi_suspend(int dev_id);
int static ar6k_wifi_resume(int dev_id);
static void (*ar6k_wifi_status_cb)(int card_present, void *dev_id);
static void *ar6k_wifi_status_cb_devid;
static unsigned int  wifi_power_on = 0;

static int ar6k_wifi_status_register(void (*callback)(int card_present, void *dev_id), void *dev_id)
{
	if (ar6k_wifi_status_cb)
		return -EAGAIN;
	ar6k_wifi_status_cb = callback;
	ar6k_wifi_status_cb_devid = dev_id;
	return 0;
}

static unsigned int ar6k_wifi_status(struct device *dev)
{
	return wifi_power_on;
}

#endif

#ifdef CONFIG_BT
static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
};

enum {
	BT_WAKE,
	BT_RFR,
	BT_CTS,
	BT_RX,
	BT_TX,
	BT_PCM_DOUT,
	BT_PCM_DIN,
	BT_PCM_SYNC,
	BT_PCM_CLK,
	BT_HOST_WAKE,
};

static unsigned bt_config_init[] = {
	GPIO_CFG(43, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* RFR */
	GPIO_CFG(44, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* CTS */
	GPIO_CFG(45, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Rx */
	GPIO_CFG(46, 3, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Tx */

	GPIO_CFG(76, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* 3.3V */
	GPIO_CFG(77, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* 1.5V */
	GPIO_CFG(34, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* 1.2V */
	GPIO_CFG(27, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* BT_RST */
	GPIO_CFG(37, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* HOST_WAKE_BT */
	GPIO_CFG(42, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),        /* BT_WAKE_HOST */
	GPIO_CFG(68, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_DOUT */
	GPIO_CFG(69, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),       /* PCM_DIN */
	GPIO_CFG(70, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_SYNC */
	GPIO_CFG(71, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/* PCM_CLK */
	GPIO_CFG(77, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),     /* 1.5V */
	GPIO_CFG(62, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),     /* sd2 */
	GPIO_CFG(63, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),       /* sd2 */
	GPIO_CFG(64, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),       /* sd2 */
	GPIO_CFG(65, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),       /* sd2 */
	GPIO_CFG(66, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),       /* sd2 */
	GPIO_CFG(67, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),       /* sd2 */
	GPIO_CFG(49, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),      /* WLAN_INT_HOST */
	GPIO_CFG(96, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),   	/* WLAN_PWD */
	GPIO_CFG(35, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),  	/* WLAN_RESET */
	GPIO_CFG(35, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),   	/* WLAN_RESET */
};
static void init_Bluetooth_gpio_table(void)
{
	int pin,rc;

	printk(KERN_INFO "Config Bluetooth GPIO\n");
	
		for (pin = 0; pin < ARRAY_SIZE(bt_config_init); pin++) {
			rc = gpio_tlmm_config(bt_config_init[pin],GPIO_CFG_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_init[pin], rc);
			}
			msleep(10);
		}
    rc = gpio_request(96, "WIFI_PWD");
    if (rc)	printk(KERN_ERR "%s: WIFI_PWD 96 setting failed! rc = %d\n", __func__, rc);
    rc = gpio_request(76, "3.3V");
    if (rc)	printk(KERN_ERR "%s: 3.3V 76 setting failed! rc = %d\n", __func__, rc);
    rc = gpio_request(77, "1.8V");
    if (rc)	printk(KERN_ERR "%s: 1.8V 77 setting failed! rc = %d\n", __func__, rc);
    rc = gpio_request(34, "1.2V");
    if (rc)	printk(KERN_ERR "%s: 1.2V 34 setting failed! rc = %d\n", __func__, rc);
    rc = gpio_request(35, "WIFI_RST");
    if (rc)	printk(KERN_ERR "%s: WIFI_RST 35 setting failed! rc = %d\n", __func__, rc);
    rc = gpio_request(27, "BT_RST");
    if (rc)	printk(KERN_ERR "%s: BT_RST 27 setting failed! rc = %d\n", __func__, rc);
/* } FIH, JamesKCTung, 2009/06/30 */
}

static int bluetooth_power(int on)
{
    int module_status=0,prev_status=0;
    bool bConfigWIFI;
/* FIH, SimonSSChang, 2010/02/26 { */
/* let ar6000 driver to turn on/off power when enter suspend/resume */
#ifdef CONFIG_FIH_FXX
    bool bConfigWIFI_suspend;
#endif
/* } FIH, SimonSSChang, 2010/02/26 */
    int value = 0;

	gpio_tlmm_config(GPIO_CFG(43, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* RFR */
	gpio_tlmm_config(GPIO_CFG(44, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* CTS */
	gpio_tlmm_config(GPIO_CFG(45, 2, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* Rx */
	gpio_tlmm_config(GPIO_CFG(46, 3, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* Tx */
	gpio_tlmm_config(GPIO_CFG(37, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);  /* HOST_WAKE_BT */
	gpio_tlmm_config(GPIO_CFG(42, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);     /* BT_WAKE_HOST */
	gpio_tlmm_config(GPIO_CFG(27, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);   /* BT_RST */
	gpio_tlmm_config(GPIO_CFG(76, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);   /* 3.3V */
	gpio_tlmm_config(GPIO_CFG(77, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);   /* 1.5V */
	gpio_tlmm_config(GPIO_CFG(34, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);   /* 1.2V */

	spin_lock(&wif_bt_lock);

    bConfigWIFI = (on & WIFI_CONTROL_MASK);

/* let ar6000 driver to turn on/off power when enter suspend/resume */
#ifdef CONFIG_FIH_FXX
    bConfigWIFI_suspend = (on & WIFI_SUSPEND_CONTROL_MASK);
    printk(KERN_INFO "on = 0x%08x", on);
#endif

    if(bConfigWIFI)
    {
        prev_status = wifi_status;
/* let ar6000 driver to turn on/off power when enter suspend/resume */
#ifdef CONFIG_FIH_FXX
        wifi_status = on & ~(WIFI_CONTROL_MASK | WIFI_SUSPEND_CONTROL_MASK); 
#endif
        if( wifi_status == prev_status )
        {
            printk(KERN_ERR "%s: WIFI already turn %s\n", __func__,  (wifi_status?"ON":"OFF") );
            spin_unlock(&wif_bt_lock);
            return 0;
        }
        if(wifi_status && !bt_status)
            module_status = MODULE_TURN_ON;
        else if(!wifi_status && !bt_status)
            module_status = MODULE_TURN_OFF;

    }else {
        prev_status = bt_status;
        bt_status = on;
        if( bt_status == prev_status )
        {
            printk(KERN_ERR "%s: BT already turn %s\n", __func__,  (bt_status?"ON":"OFF") );
            spin_unlock(&wif_bt_lock);
            return 0;
        }
        if(bt_status && !wifi_status)
            module_status = MODULE_TURN_ON;
        else if(!wifi_status && !bt_status)
            module_status = MODULE_TURN_OFF;
    }

    //power control before module on/off
    if(!bConfigWIFI &&  !bt_status) {     //Turn BT off
        printk(KERN_DEBUG "%s : Turn BT off.\n", __func__);
		gpio_direction_output(27,0);    
    }else if(!bConfigWIFI &&  bt_status){     //Turn BT on        
        printk(KERN_DEBUG "%s : Turn BT on.\n", __func__);
    }else if(bConfigWIFI && wifi_status) {  //Turn WIFI on
        printk(KERN_DEBUG "%s : Turn WIFI on.\n", __func__);
        gpio_direction_output(96,0);
        gpio_direction_output(35,0);
    }else if(bConfigWIFI && !wifi_status) {  //Turn WIFI OFF
        printk(KERN_DEBUG "%s : Turn WIFI off.\n", __func__);

/* let ar6000 driver to turn on/off power when enter suspend/resume */
#ifdef CONFIG_FIH_FXX
        if(!bConfigWIFI_suspend) {
        if(ar6k_wifi_status_cb) {
            wifi_power_on=0;
            ar6k_wifi_status_cb(0,ar6k_wifi_status_cb_devid);
        }else
            printk(KERN_ERR "!!!wifi_power Fail:  ar6k_wifi_status_cb_devid is NULL \n");

        gpio_direction_output(96,0);
        gpio_direction_output(35,0);
    }
}
#endif
/* } FIH, SimonSSChang, 2010/02/26 */

    //Turn module on/off
    if(module_status == MODULE_TURN_ON) {   //turn module on
        printk(KERN_DEBUG "%s : Turn module(A22) on.\n", __func__);
        //FIH_ADQ.B.1741 turn on BT is too bad
        gpio_direction_output(76,1);
        value = 0;
        value = gpio_get_value(76);
        printk(KERN_DEBUG "%s : GPIO 76 is %d.\n", __func__, value);
        gpio_direction_output(77,1);
        value = 0;
        value = gpio_get_value(77);
        printk(KERN_DEBUG "%s : GPIO 77 is %d.\n", __func__, value);
        gpio_direction_output(34,1);
        value = 0;
        value = gpio_get_value(34);
        printk(KERN_DEBUG "%s : GPIO 34 is %d.\n", __func__, value);
    }else if(module_status == MODULE_TURN_OFF) { //turn module off
        printk(KERN_DEBUG "%s : Turn module(A22) off.\n", __func__);
        gpio_direction_output(34,0);
        gpio_direction_output(77,0);
        gpio_direction_output(76,0);
    }

    if(!bConfigWIFI &&  !bt_status) {  //Turn BT off
/* FIH, WilsonWHLee, 2009/07/30 { */
/* [FXX_CR], re-configure GPIO when BT turn on/off */
#if CONFIG_FIH_FXX  
	gpio_tlmm_config(GPIO_CFG(43, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* RFR */
	gpio_tlmm_config(GPIO_CFG(44, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* CTS */
	gpio_tlmm_config(GPIO_CFG(45, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* Rx */
	gpio_tlmm_config(GPIO_CFG(46, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE);	/* Tx */
	gpio_tlmm_config(GPIO_CFG(37, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE); 	/* HOST_WAKE_BT */
	gpio_tlmm_config(GPIO_CFG(42, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),GPIO_CFG_ENABLE); 	/* BT_WAKE_HOST */
#endif
    }else if(!bConfigWIFI &&  bt_status){    //Turn BT on
        gpio_direction_output(27,0);
        mdelay(10);
        gpio_direction_output(27,1);
        value = 0;
        value = gpio_get_value(27);
        printk(KERN_DEBUG "%s : GPIO 27 is %d.\n", __func__, value);
        mdelay(10);
    }else if(bConfigWIFI && wifi_status) { //Turn WIFI on
        gpio_direction_output(96,1);
        value = 0;
        value = gpio_get_value(96);
        printk(KERN_DEBUG "%s : GPIO 96 is %d.\n", __func__, value);
        mdelay(10);
        gpio_direction_output(35,1);
        value = 0;
        value = gpio_get_value(35);
        printk(KERN_DEBUG "%s : GPIO 35 is %d.\n", __func__, value);

/* FIH, SimonSSChang, 2010/02/26 { */
/* let ar6000 driver to turn on/off power when enter suspend/resume */
#ifdef CONFIG_FIH_FXX
        if(!bConfigWIFI_suspend) {
        if(ar6k_wifi_status_cb) {
            wifi_power_on=1;
            ar6k_wifi_status_cb(1,ar6k_wifi_status_cb_devid);
        }else
            printk(KERN_ERR "!!!wifi_power Fail:  ar6k_wifi_status_cb_devid is NULL \n");
}
    }else if(bConfigWIFI && !wifi_status) {  //Turn WIFI OFF        
    }

    spin_unlock(&wif_bt_lock);

	return 0;
}
/* FIH, SimonSSChang, 2010/02/26 { */
/* let ar6000 driver to turn on/off power when enter suspend/resume */
#ifdef CONFIG_FIH_FXX
int wifi_power(int on)
{
    int ret;
    ret = bluetooth_power(on);
    printk(KERN_INFO "wifi_power ret = %d\n", ret);
    return ret;
}
EXPORT_SYMBOL(wifi_power);
#endif
/* } FIH, SimonSSChang, 2010/02/26 */

/* } FIH, JamesKCTung, 2009/06/30 */

static void __init bt_power_init(void)
{
	msm_bt_power_device.dev.platform_data = &bluetooth_power;
}
#else
#define bt_power_init(x) do {} while (0)
#endif

static struct resource bluesleep_resources[] = {
	{
		.name	= "gpio_host_wake",
		.start	= 42,
		.end	= 42,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "gpio_ext_wake",
		.start	= 37,
		.end	= 37,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "host_wake",
		.start	= MSM_GPIO_TO_INT(42),
		.end	= MSM_GPIO_TO_INT(42),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_bluesleep_device = {
	.name = "bluesleep",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(bluesleep_resources),
	.resource	= bluesleep_resources,
};

static struct resource kgsl_resources[] = {
	{
		.name = "kgsl_reg_memory",
		.start = 0xA0000000,
		.end = 0xA001ffff,
		.flags = IORESOURCE_MEM,
	},
	{
		.name   = "kgsl_phys_memory",
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "kgsl_yamato_irq",
		.start = INT_GRAPHICS,
		.end = INT_GRAPHICS,
		.flags = IORESOURCE_IRQ,
	},
};

static struct kgsl_platform_data kgsl_pdata;

static struct platform_device msm_device_kgsl = {
	.name = "kgsl",
	.id = -1,
	.num_resources = ARRAY_SIZE(kgsl_resources),
	.resource = kgsl_resources,
	.dev = {
		.platform_data = &kgsl_pdata,
	},
};

static struct platform_device msm_device_pmic_leds = {
	.name   = "pmic-leds",
	.id = -1,
};

static struct tca6507_platform_data tca6507_data = {
	.tca6507_reset = 84,
};
static struct elan_i2c_platform_data elan8232_device = {
    .abs_x_min= -1,
    .abs_x_max= 1793,
    .abs_y_min= -1,
    .abs_y_max= 2817,
    .intr_gpio= 89,
};
#if 1
static struct focal_i2c_platform_data focal5202_device = {
    .abs_x_min= 0,
    .abs_x_max= 511,
    .abs_y_min= 0,
    .abs_y_max= 639,
    .intr_gpio= 89,
};
#endif
#endif
//Added for capacitive touch panel, by Stanley-- 2009/06/03

static struct i2c_board_info i2c_devices[] = {
    /* Backlight */
	{
		I2C_BOARD_INFO("max8831", 0x9a>>1),
	},
#ifdef CONFIG_FIH_FXX
{
    I2C_BOARD_INFO("ft5202", 0x72 >> 1),
    .irq            = MSM_GPIO_TO_INT(89),
    .platform_data  = &focal5202_device,
},
#endif
#ifdef CONFIG_FIH_FXX
        {
                I2C_BOARD_INFO("MS3C", 0x2e),
        },
        {
                I2C_BOARD_INFO("SMB380", 0x38),
        },
#endif
#ifdef CONFIG_OV3642
	{
		/* Fake address for multi sensors with the same adress */
		I2C_BOARD_INFO("ov3642", 0xFE),
	},
#endif
#ifdef CONFIG_OV5642AF
	{
		I2C_BOARD_INFO("ov5642af", 0xFD),
	},
#endif
#ifdef CONFIG_OV5642
	{
		I2C_BOARD_INFO("ov5642", 0x3C),
	},
#endif
#ifdef CONFIG_FLASH_DRIVER_IC_AAT1272
	{
		I2C_BOARD_INFO("aat1272", 0x6F >> 1),	
	},
#endif
#ifdef CONFIG_BACKLIGHT_LED_TCA6507
	{
		I2C_BOARD_INFO("tca6507", 0x8A >> 1), // 0x45
		.platform_data = &tca6507_data,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_ELAN_I2C_8232
	{
		I2C_BOARD_INFO("bi8232", 0x10),
		.irq            = MSM_GPIO_TO_INT(89),
		.platform_data  = &elan8232_device,
	},
#endif
};

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

#ifdef CONFIG_FIH_F9xx_GPIO_KEYPAD
static uint32_t keypad_gpio_table[] = {
	GPIO_CFG(41,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), /* Volume Up Key    */
	GPIO_CFG(36,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), /* Volume Down Key  */
	GPIO_CFG(28,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), /* Send key         */
	GPIO_CFG(19,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), /* End Key          */
};
#endif /* CONFIG_FIH_F9xx_GPIO_KEYPAD */

#ifdef CONFIG_MSM_CAMERA
static uint32_t camera_off_gpio_table[] = {
	GPIO_CFG(2,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
	GPIO_CFG(3,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
	GPIO_CFG(4,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(5,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(6,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(7,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(8,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
	GPIO_CFG(9,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
	GPIO_CFG(10, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT10 */
	GPIO_CFG(11, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT11 */
	GPIO_CFG(12, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* PCLK */
	GPIO_CFG(13, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
	/* parallel CAMERA interfaces */
	GPIO_CFG(2,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
	GPIO_CFG(3,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
	GPIO_CFG(4,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(5,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(6,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(7,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(8,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT8 */
	GPIO_CFG(9,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT9 */
	GPIO_CFG(10, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT10 */
	GPIO_CFG(11, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT11 */
	GPIO_CFG(12, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* PCLK */
	GPIO_CFG(13, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* MCLK */
};

static int config_camera_on_gpios(void)
{
	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));

	return 0;
}

static void config_camera_off_gpios(void)
{
	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

static struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on  = config_camera_on_gpios,
	.camera_gpio_off = config_camera_off_gpios,
	.ioext.mdcphy = MSM_MDC_PHYS,
	.ioext.mdcsz  = MSM_MDC_SIZE,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
};

int pmic_set_flash_led_current(enum pmic8058_leds id, unsigned mA)
{
	int rc;
	rc = pmic_flash_led_set_current(mA);
	return rc;
}

static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_PMIC,
	._fsrc.pmic_src.num_of_src = 1,
	._fsrc.pmic_src.low_current  = 30,
	._fsrc.pmic_src.high_current = 100,
	._fsrc.pmic_src.led_src_1 = 0,
	._fsrc.pmic_src.led_src_2 = 0,
	._fsrc.pmic_src.pmic_set_current = pmic_set_flash_led_current,
};

#ifdef CONFIG_OV5642

static struct msm_camera_sensor_flash_data flash_ov5642 = {
        .flash_type = MSM_CAMERA_FLASH_LED,
        .flash_src  = &msm_flash_src
};
 
static struct msm_camera_sensor_info msm_camera_sensor_ov5642_data = {
	.sensor_name    = "ov5642",
	.sensor_reset   = 0,
	.sensor_pwd     = 31,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
	.flash_data     = &flash_ov5642,
};

static struct platform_device msm_camera_sensor_ov5642 = {
	.name      = "msm_camera_ov5642",
	.dev       = {
		.platform_data = &msm_camera_sensor_ov5642_data,
	},
};
#endif

#ifdef CONFIG_OV5642AF
static struct msm_camera_sensor_info msm_camera_sensor_ov5642af_data = {
	.sensor_name    = "ov5642af",
	.sensor_reset   = 0,
	.sensor_pwd     = 31,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
};

static struct platform_device msm_camera_sensor_ov5642af = {
	.name      = "msm_camera_ov5642af",
	.dev       = {
		.platform_data = &msm_camera_sensor_ov5642af_data,
	},
};
#endif

#ifdef CONFIG_OV3642
static struct msm_camera_sensor_info msm_camera_sensor_ov3642_data = {
	.sensor_name    = "ov3642",
	.sensor_reset   = 0,
	.sensor_pwd     = 31,
	.vcm_pwd        = 0,
	.vcm_enable     = 0,
	.pdata          = &msm_camera_device_data,
};

static struct platform_device msm_camera_sensor_ov3642 = {
	.name      = "msm_camera_ov3642",
	.dev       = {
		.platform_data = &msm_camera_sensor_ov3642_data,
	},
};
#endif
#endif /* CAMERA */
static u32 msm_calculate_batt_capacity(u32 current_voltage);

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design 	= 2800,
	.voltage_max_design	= 4300,
	.avail_chg_sources   	= AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity	= &msm_calculate_batt_capacity,
};

static u32 msm_calculate_batt_capacity(u32 current_voltage)
{
	u32 low_voltage   = msm_psy_batt_data.voltage_min_design;
	u32 high_voltage  = msm_psy_batt_data.voltage_max_design;

	return (current_voltage - low_voltage) * 100
		/ (high_voltage - low_voltage);
}

static struct platform_device msm_batt_device = {
	.name 		    = "msm-battery",
	.id		    = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};
/* Config GPIO for keypad         */
#ifdef CONFIG_FIH_F9xx_GPIO_KEYPAD
static void config_keypad_gpios(void)
{
	config_gpio_table(keypad_gpio_table,
		ARRAY_SIZE(keypad_gpio_table));
}

static struct Q7x27_kybd_platform_data q7x27_kybd_data = {
	.keypad_gpio = config_keypad_gpios,
	.volup_pin = 28,
	.voldn_pin = 19,
	.key_1_pin = 41,
	.key_2_pin = 36,	
	.cam_sw_t_pin = 20,
	.cam_sw_f_pin = 29,
	.hook_sw_pin = 94,
};

static struct platform_device q7x27_kybd_device = {
        .name = "7x27_kybd",
        .dev = {
                .platform_data = &q7x27_kybd_data,
        },
};

static void __init keypad_gpio_init(void)
{
	config_keypad_gpios();
}
#endif /* CONFIG_FIH_F9xx_GPIO_KEYPAD */

// add vibrator
static struct platform_device pmic_rpc_device = {
        .name	= "pmic_rpc",
        .id		= -1,
};

#if (!defined(CONFIG_ARCH_MSM_FLASHLIGHT) && \
	defined(CONFIG_FLASH_DRIVER_IC_AAT1272))
static struct platform_device aat1272_flashlight_device = {
	.name = "flashlight",
	.dev = {
	},
};
#endif

/* lcm_innolux gpio */
#ifdef CONFIG_SPI_GPIO

static unsigned spi_gpio_config_input[] = {
	GPIO_CFG(101, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),  /* clk */
	GPIO_CFG(102, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),  /* cs */
	GPIO_CFG(132, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),  // no use
	GPIO_CFG(131, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),  /* mosi */
};

static int __init spi_gpio_init_fake(void)
{
	int rc = 0, pin;

	return 0; 

	for (pin = 0; pin < ARRAY_SIZE(spi_gpio_config_input); pin++) {
		rc = gpio_tlmm_config(spi_gpio_config_input[pin], GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",__func__, spi_gpio_config_input[pin], rc);
			return -EIO;
		}
	}

	rc = gpio_tlmm_config(GPIO_CFG(85, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	rc = gpio_tlmm_config(GPIO_CFG(103, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	rc = gpio_request(85, "cam_pwr");

	rc = gpio_request(103, "lcd_reset");

	gpio_direction_output(85,1);
	gpio_direction_output(103,1);
	mdelay(500);
	gpio_direction_output(103,0);
	mdelay(500);
	gpio_direction_output(103,1);
	mdelay(50);
    gpio_free(85);
    gpio_free(103);
	rc = gpio_request(101, "gpio_spi");
    rc = gpio_request(102, "gpio_spi");
    rc = gpio_request(131, "gpio_spi");
    rc = gpio_request(132, "gpio_spi");
	gpio_direction_output(101,1);
	gpio_direction_output(102,1);
	gpio_direction_input(132);
	gpio_direction_output(131,1);	
	gpio_free(101);
	gpio_free(102);
	gpio_free(131);
	gpio_free(132);               
	
	return rc;
}

static struct spi_board_info lcdc_spi_devices[] = {
	{
		.modalias = "lcdc_spi",
		.max_speed_hz = 10000000,
		.chip_select = 0,
		.controller_data = (void *) 102,
		.mode           = SPI_MODE_3,
	},
};

struct spi_gpio_platform_data lcdc_spigpio_platform_data = { 
        .sck = 101,
        .mosi = 131,
        .miso = 132,
        .num_chipselect = 1,
};

static struct platform_device lcdc_spigpio_device = {

        .name = "spi_gpio",
        .dev = {
                .platform_data = &lcdc_spigpio_platform_data,
        },
};
#endif  //CONFIG_SPI_GPIO

static int msm_fb_lcdc_config(int on){
    return 0;
}

static char *msm_fb_lcdc_vreg[] = {
	"gp5"
};

#define MSM_FB_LCDC_VREG_OP(name, op) \
	do { \
		vreg = vreg_get(0, name); \
		if (vreg_##op(vreg)) \
		printk(KERN_ERR "%s: %s vreg operation failed \n", \
				(vreg_##op == vreg_enable) ? "vreg_enable" \
				: "vreg_disable", name); \
	} while (0)

static int msm_fb_lcdc_power_save(int on)
{
	struct vreg *vreg;
	int i, rc = 0;

	for (i = 0; i < ARRAY_SIZE(msm_fb_lcdc_vreg); i++) {
		if (on)
			MSM_FB_LCDC_VREG_OP(msm_fb_lcdc_vreg[i], enable);
		else{
			int res=0;
			MSM_FB_LCDC_VREG_OP(msm_fb_lcdc_vreg[i], disable);
			gpio_tlmm_config(GPIO_CFG(88, 0,
						GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			if (!rc) { rc = res; }
			gpio_set_value(88, 0);
			mdelay(15);
			gpio_set_value(88, 1);
			mdelay(15);
		}
	}
	return rc;
}

static struct lcdc_platform_data lcdc_pdata = {
	.lcdc_gpio_config = msm_fb_lcdc_config,
	.lcdc_power_save   = msm_fb_lcdc_power_save,
};

/* ATHENV */
static struct platform_device msm_wlan_ar6000_pm_device = {
	.name		= "wlan_ar6000_pm_dev",
	.id		= 1,
	.num_resources	= 0,
	.resource	= NULL,
};
/* ATHENV */

static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_ANDROID_RAM_CONSOLE
    &ram_console_device,
#endif
	&msm_wlan_ar6000_pm_device,
	&msm_device_smd,
	&msm_device_dmov,
	&msm_device_nand,
	&msm_device_otg,
	&msm_device_gadget_peripheral,
#ifdef CONFIG_USB_ANDROID
	&usb_mass_storage_device,
	&rndis_device,
#ifdef CONFIG_USB_ANDROID_DIAG
	&usb_diag_device,
#endif
#ifdef CONFIG_USB_F_SERIAL
	&usb_gadget_fserial_device,
#endif
	&android_usb_device,
#endif
	&msm_device_i2c,
	&msm_device_tssc,
	&android_pmem_kernel_ebi1_device,
	&android_pmem_device,
	&android_pmem_adsp_device,
	&android_pmem_audio_device,
#ifdef CONFIG_SPI_GPIO
	&lcdc_spigpio_device,
#endif	 //CONFIG_SPI_GPIO
	&msm_fb_device,
	&msm_device_uart_dm1,
#ifdef CONFIG_BT
	&msm_bt_power_device,
#endif
	&msm_device_pmic_leds,
	&msm_device_snd,
	&msm_device_adspdec,
#ifdef CONFIG_FIH_F9xx_GPIO_KEYPAD
	&q7x27_kybd_device,
#endif /* CONFIG_FIH_F9xx_GPIO_KEYPAD */
    &pmic_rpc_device,
#ifdef CONFIG_OV5642
	&msm_camera_sensor_ov5642,
#endif
#ifdef CONFIG_OV5642AF
	&msm_camera_sensor_ov5642af,
#endif
#ifdef CONFIG_OV3642
	&msm_camera_sensor_ov3642,
#endif
	&msm_bluesleep_device,

	&headset_sensor_device, 
	&msm_device_kgsl,
	&ALSPS_sensor_device, 
	&mtb_platform_device,
	&hs_device,
	&msm_batt_device,
};

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
};

static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("pmdh", 0);
	msm_fb_register_device("lcdc", &lcdc_pdata);
	msm_fb_register_device("batt", 0);
}

extern struct sys_timer msm_timer;

static void __init msm7x2x_init_irq(void)
{
	msm_init_irq();
}

static struct msm_acpu_clock_platform_data msm7x2x_clock_data = {
	.acpu_switch_time_us = 50,
	.max_speed_delta_khz = 400000,
	.vdd_switch_time_us = 62,
	.max_axi_khz = 160000,
};

void msm_serial_debug_init(unsigned int base, int irq,
			   struct device *clk_device, int signal_irq);

#ifdef CONFIG_MMC
static void sdcc_gpio_init(void)
{
	/* SDC1 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	if (gpio_request(51, "sdc1_data_3"))
		pr_err("failed to request gpio sdc1_data_3\n");
	if (gpio_request(52, "sdc1_data_2"))
		pr_err("failed to request gpio sdc1_data_2\n");
	if (gpio_request(53, "sdc1_data_1"))
		pr_err("failed to request gpio sdc1_data_1\n");
	if (gpio_request(54, "sdc1_data_0"))
		pr_err("failed to request gpio sdc1_data_0\n");
	if (gpio_request(55, "sdc1_cmd"))
		pr_err("failed to request gpio sdc1_cmd\n");
	if (gpio_request(56, "sdc1_clk"))
		pr_err("failed to request gpio sdc1_clk\n");
#endif
	if (machine_is_msm7x25_ffa())
		return;

	/* SDC2 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	if (gpio_request(62, "sdc2_clk"))
		pr_err("failed to request gpio sdc2_clk\n");
	if (gpio_request(63, "sdc2_cmd"))
		pr_err("failed to request gpio sdc2_cmd\n");
	if (gpio_request(64, "sdc2_data_3"))
		pr_err("failed to request gpio sdc2_data_3\n");
	if (gpio_request(65, "sdc2_data_2"))
		pr_err("failed to request gpio sdc2_data_2\n");
	if (gpio_request(66, "sdc2_data_1"))
		pr_err("failed to request gpio sdc2_data_1\n");
	if (gpio_request(67, "sdc2_data_0"))
		pr_err("failed to request gpio sdc2_data_0\n");
#endif
	if (machine_is_msm7x27_ffa())
		return;
}

static unsigned sdcc_cfg_data[][6] = {
	/* SDC1 configs */
	{
	GPIO_CFG(51, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(52, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(53, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(54, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(55, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(56, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_12MA),
	},
	/* SDC2 configs */
	{
	GPIO_CFG(62, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(63, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(64, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(65, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(66, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(67, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	},
	/* SDC3 configs */
	{
	GPIO_CFG(88, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(89, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(90, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(91, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(92, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(93, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	},
	/* SDC4 configs */
	{
	GPIO_CFG(19, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(20, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(21, 4, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(107, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(108, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(109, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	}
};

/* FIH, BillHJChang, 2009/07/20 { */
/* [FXX_CR], Set power state of SD bus when card remove */
#if	CONFIG_FIH_FXX
static unsigned sdcc_cfg_pulldown[][6] = {
	/* SDC1 configs */
	{
	GPIO_CFG(51, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(52, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(53, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(54, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(55, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(56, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_12MA),
	}
};
#endif	
static unsigned long vreg_sts, gpio_sts;
static struct vreg *vreg_mmc;

static void msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int i, rc;

	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return;

	if (enable)
		set_bit(dev_id, &gpio_sts);
	else
		clear_bit(dev_id, &gpio_sts);

/* FIH, BillHJChang, 2009/07/20 { */
/* [FXX_CR], Set power state of SD bus when card remove */
#ifdef CONFIG_FIH_FXX
// Modified Code
	if (enable)
	{
		for (i = 0; i < ARRAY_SIZE(sdcc_cfg_data[dev_id - 1]); i++) {
			rc = gpio_tlmm_config(sdcc_cfg_data[dev_id - 1][i],
				enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
			if (rc )
				printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
					__func__, sdcc_cfg_data[dev_id - 1][i], rc);
		}	
	}
	else
	{
		for (i = 0; i < ARRAY_SIZE(sdcc_cfg_pulldown[dev_id - 1]); i++) {
			rc = gpio_tlmm_config(sdcc_cfg_pulldown[dev_id - 1][i],
				!enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
			if (rc)
				printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
					__func__, sdcc_cfg_pulldown[dev_id - 1][i], rc);
		}	
	}

#else
// Original Code
	for (i = 0; i < ARRAY_SIZE(sdcc_cfg_data[dev_id - 1]); i++) {
		rc = gpio_tlmm_config(sdcc_cfg_data[dev_id - 1][i],
			enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
		if (rc)
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, sdcc_cfg_data[dev_id - 1][i], rc);
	}
#endif
	/* FIH, MonkyChen, 2010/02/10 { */
    #ifdef CONFIG_FIH_FXX
	// Setup SD card detection interrupt pin
	
	rc = gpio_tlmm_config(GPIO_CFG(18, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
					  			
	if (rc)
	    printk(KERN_ERR "%s: ERROR!! config SD detect pin =%d\n",__func__, rc);
    #endif
    /* FIH, MonkyChen, 2010/02/10 } */
}

/* FIH, JamesKCTung, 2009/07/09 { */
#ifdef CONFIG_FIH_FXX
static int  ar6k_wifi_suspend(int dev_id)
{
    bluetooth_power(WIFI_CONTROL_MASK | 0);  
    return 1;
}

static int  ar6k_wifi_resume(int dev_id)
{
    bluetooth_power(WIFI_CONTROL_MASK | 1);    
    return 1;
}

static uint32_t msm_ar6k_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
    return 0;
}
#endif
/* } FIH, JamesKCTung, 2009/07/09 */
static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	/* FIH, BillHJChang, 2009/07/22 { */
	/* [FXX_CR], 7627 SD card power supply source changed*/
	#ifdef CONFIG_FIH_FXX	
	if (vreg_mmc == NULL)
	{
		int iHwid = 0;
		iHwid = FIH_READ_HWID_FROM_SMEM();

		if(iHwid >= CMCS_7627_EVB1)
		{
			vreg_mmc = vreg_get(NULL, "gp5");
		}
		else
		{
			vreg_mmc = vreg_get(NULL, "mmc");
		}

		if (IS_ERR(vreg_mmc))
			printk(KERN_ERR "%s: vreg get failed (%ld)\n",__func__, PTR_ERR(vreg_mmc));			
	}
	#endif	
	/* } FIH, BillHJChang, 2009/07/22 */	
	
	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	if (vdd == 0) {
		if (!vreg_sts)
			return 0;

		clear_bit(pdev->id, &vreg_sts);

		if (!vreg_sts) {
			rc = vreg_disable(vreg_mmc);
			if (rc)
				printk(KERN_ERR "%s: return val: %d \n",
						__func__, rc);
		}
		return 0;
	}

	if (!vreg_sts) {

		rc = vreg_set_level(vreg_mmc, 2850);
		if (!rc)
			rc = vreg_enable(vreg_mmc);
		if (rc)
			printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
	set_bit(pdev->id, &vreg_sts);
	return 0;
}

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct mmc_platform_data msm7x2x_sdc1_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin   = 144000,
	.msmsdcc_fmid   = 24576000,
	.msmsdcc_fmax   = 49152000,
	.nonremovable   = 0,
};
#endif

#if defined(CONFIG_MMC_MSM_SDC2_SUPPORT) && !defined(CONFIG_AR6K)
static struct mmc_platform_data msm7x2x_sdc2_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	.sdiowakeup_irq = MSM_GPIO_TO_INT(66),
#endif
};
#endif

#if defined(CONFIG_MMC_MSM_SDC2_SUPPORT) && defined(CONFIG_AR6K)
static struct mmc_platform_data ar6k_wifi_data = {
	.ocr_mask	    = MMC_VDD_28_29,
	.translate_vdd	= msm_ar6k_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.status			= ar6k_wifi_status,
	.register_status_notify	= ar6k_wifi_status_register,
	.sdio_suspend = ar6k_wifi_suspend,
	.sdio_resume = ar6k_wifi_resume,
	.dummy52_required = 1,	
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};
#endif

static void __init msm7x2x_init_mmc(void)
{
#ifndef CONFIG_FIH_FXX
	if (!machine_is_msm7x25_ffa() && !machine_is_msm7x27_ffa()) {
		vreg_mmc = vreg_get(NULL, "mmc");
		if (IS_ERR(vreg_mmc)) {
			return;
		}
	}
#endif
	sdcc_gpio_init();
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	msm_add_sdcc(1, &msm7x2x_sdc1_data);
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
#ifdef CONFIG_AR6K
	msm_add_sdcc(2, &ar6k_wifi_data);
#else
	msm_add_sdcc(2, &msm7x2x_sdc2_data);
#endif
#endif

}
#else
#define msm7x2x_init_mmc() do {} while (0)
#endif

/* FIH, SimonSSChang, 2010/03/05 { */
#ifndef CONFIG_FIH_FXX
static unsigned mpp_deep_sleep = 3;
static int __init wifi_power_init(void)
{
	int rc;
	struct vreg *vreg_wlan;

	rc = mpp_config_digital_out(mpp_deep_sleep,
	     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
	     MPP_DLOGIC_OUT_CTRL_LOW));
	if (rc) {
		printk(KERN_ERR "%s: return val:  (%d)\n",
		       __func__, rc);
		return -EIO;
	}

	vreg_wlan = vreg_get(NULL, "wlan");
	if (IS_ERR(vreg_wlan)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
		       __func__, PTR_ERR(vreg_wlan));
		return PTR_ERR(vreg_wlan);
	}

	rc = vreg_set_level(vreg_wlan, 2600);
	if (rc) {
		printk(KERN_ERR "%s: vreg set level failed (%d)\n",
		       __func__, rc);
		return -EIO;
	}

	rc = vreg_enable(vreg_wlan);
	if (rc) {
		printk(KERN_ERR "%s: vreg enable failed (%d)\n",
		       __func__, rc);
		return -EIO;
	}

	return 0;
}
#endif
/* } FIH, SimonSSChang, 2010/03/05 */

static struct msm_pm_platform_data msm7x27_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency = 16000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].residency = 20000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency = 12000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].residency = 20000,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].supported = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency = 2000,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].residency = 0,
};

static void
msm_i2c_gpio_config(int iface, int config_type)
{
	int gpio_scl;
	int gpio_sda;
	gpio_scl = 60;
	gpio_sda = 61;
	if (iface) {
		return;
	}
	if (config_type) {
		gpio_tlmm_config(GPIO_CFG(gpio_scl, 1, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(gpio_sda, 1, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
	} else {
		gpio_tlmm_config(GPIO_CFG(gpio_scl, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(gpio_sda, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
	}
}

static struct msm_i2c_platform_data msm_i2c_pdata = {
	.clk_freq = 100000,
	.rmutex  = 0,
	.pri_clk = 60,
	.pri_dat = 61,
	.aux_clk = 95,
	.aux_dat = 96,
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_init(void)
{
	if (gpio_request(60, "i2c_pri_clk"))
		pr_err("failed to request gpio i2c_pri_clk\n");
	if (gpio_request(61, "i2c_pri_dat"))
		pr_err("failed to request gpio i2c_pri_dat\n");
	msm_i2c_pdata.pm_lat =
		msm7x27_pm_data[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN]
		.latency;

	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

void msm_init_pmic_vibrator(void);

/* FIH_ADQ, lcm_innolux */
extern int  spi_gpio_init(void); //lcm_innolux
//Add for headset detection.
static void __init init_headset_sensor(void)
{	
	gpio_direction_input(40);
}
static ssize_t fxx_virtual_keys_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	/* center: x: home: 55, menu: 185, back: 305, search 425, y: 835 */
	return sprintf(buf,
			__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":48:525:60:60"
			":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":137:525:60:60"
			":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH)   ":228:525:60:60"
			":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":280:525:60:60"
			"\n");
}

static struct kobj_attribute fxx_virtual_keys_attr = {
        .attr = {
                .name = "virtualkeys.Elan BI1050-M32EMAU Touchscreen",
                .mode = S_IRUGO,
        },
        .show = &fxx_virtual_keys_show,
};

static struct attribute *fxx_properties_attrs[] = {
        &fxx_virtual_keys_attr.attr,
        NULL
};

static struct attribute_group fxx_properties_attr_group = {
        .attrs = fxx_properties_attrs,
};

extern void adq_info_init(void);

static void __init msm7x2x_init(void)
{
	struct kobject *properties_kobj;
	fih_smem_alloc_for_host_used();
	fih_smem_alloc();

	if (socinfo_init() < 0)
		BUG();

	msm_clock_init(msm_clocks_7x27, msm_num_clocks_7x27);
#ifdef CONFIG_AR6K
	ar6k_wifi_status_cb=NULL; 
	ar6k_wifi_status_cb_devid=NULL;
#endif

	msm7x2x_clock_data.max_axi_khz = 422400;

	msm_acpu_clock_init(&msm7x2x_clock_data);
	msm_read_serial_number_from_nvitem();

#ifdef CONFIG_MSM_KGSL
	/* This value has been set to 160000 for power savings. */
	/* OEMs may modify the value at their discretion for performance */
	/* The appropriate maximum replacement for 160000 is: */
	/* clk_get_max_axi_khz() */
	kgsl_pdata.high_axi_3d = clk_get_max_axi_khz();

	/* 7x27 doesn't allow graphics clocks to be run asynchronously to */
	/* the AXI bus */
	kgsl_pdata.max_grp2d_freq = 0;
	kgsl_pdata.min_grp2d_freq = 0;
	kgsl_pdata.set_grp2d_async = NULL;
	kgsl_pdata.max_grp3d_freq = 0;
	kgsl_pdata.min_grp3d_freq = 0;
	kgsl_pdata.set_grp3d_async = NULL;
	kgsl_pdata.imem_clk_name = "imem_clk";
	kgsl_pdata.grp3d_clk_name = "grp_clk";
	kgsl_pdata.grp3d_pclk_name = "grp_pclk";
	kgsl_pdata.grp2d0_clk_name = NULL;
	kgsl_pdata.idle_timeout_3d = HZ/5;
	kgsl_pdata.idle_timeout_2d = 0;

#ifdef CONFIG_KGSL_PER_PROCESS_PAGE_TABLE
	kgsl_pdata.pt_va_size = SZ_32M;
	/* Maximum of 32 concurrent processes */
	kgsl_pdata.pt_max_count = 32;
#else
	kgsl_pdata.pt_va_size = SZ_128M;
	/* We only ever have one pagetable for everybody */
	kgsl_pdata.pt_max_count = 1;
#endif

#endif

#ifdef CONFIG_USB_MSM_OTG_72K
	msm_device_otg.dev.platform_data = &msm_otg_pdata;
	msm_otg_pdata.pemp_level =
		PRE_EMPHASIS_WITH_10_PERCENT;
	msm_otg_pdata.drv_ampl = HS_DRV_AMPLITUDE_5_PERCENT;
	msm_otg_pdata.cdr_autoreset = CDR_AUTO_RESET_DISABLE;
	msm_otg_pdata.phy_reset_sig_inverted = 1;


#ifdef CONFIG_USB_GADGET
	msm_otg_pdata.swfi_latency =
		msm7x27_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
	msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;
	msm_gadget_pdata.is_phy_status_timer_on = 1;
#endif

#endif

#ifdef CONFIG_SPI_GPIO
	spi_gpio_init_fake();
#endif    //CONFIG_SPI_GPIO

#ifdef CONFIG_FIH_F9xx_GPIO_KEYPAD
	keypad_gpio_init();
#endif /* CONFIG_FIH_F9xx_GPIO_KEYPAD */

	platform_add_devices(devices, ARRAY_SIZE(devices));
#ifdef CONFIG_MSM_CAMERA
	config_camera_off_gpios(); /* might not be necessary */
#endif
#ifdef CONFIG_SPI_GPIO
	spi_register_board_info(lcdc_spi_devices,ARRAY_SIZE(lcdc_spi_devices));
#endif    //CONFIG_SPI_GPIO	
	msm_device_i2c_init();
	i2c_register_board_info(0, i2c_devices, ARRAY_SIZE(i2c_devices));

	msm_power_register();
	msm_fb_add_devices();
#ifdef CONFIG_USB_EHCI_MSM
	msm7x2x_init_host();
#endif
	msm7x2x_init_mmc();

	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj) {
		if (sysfs_create_group(properties_kobj,
					&fxx_properties_attr_group))
			pr_err("failed to create board_properties\n");
	} else {
		pr_err("failed to create board_properties\n");
	}

	platform_device_register(&aat1272_flashlight_device);

	//Necesario para utilizar los cascos en modo manos libres	
	init_headset_sensor(); 
	msm_init_pmic_vibrator();
	init_Bluetooth_gpio_table();
	bt_power_init();

	msm_pm_set_platform_data(msm7x27_pm_data,ARRAY_SIZE(msm7x27_pm_data));
	adq_info_init();
}

static unsigned pmem_kernel_ebi1_size = PMEM_KERNEL_EBI1_SIZE;

static unsigned pmem_mdp_size = MSM_PMEM_MDP_SIZE;

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static unsigned fb_size = MSM_FB_SIZE;

#ifdef CONFIG_ARCH_MSM7X27
static unsigned gpu_phys_size = MSM_GPU_PHYS_SIZE;
static void __init gpu_phys_size_setup(char **p)
{
	gpu_phys_size = memparse(*p, p);
}
__early_param("gpu_phys_size=", gpu_phys_size_setup);
#endif

static void __init msm_msm7x2x_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = pmem_mdp_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_pdata.start = __pa(addr);
		android_pmem_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for mdp "
			"pmem arena\n", size, addr, __pa(addr));
	}

	size = pmem_adsp_size;
	if (size) {
		addr = alloc_bootmem(size);
		android_pmem_adsp_pdata.start = __pa(addr);
		android_pmem_adsp_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for adsp "
			"pmem arena\n", size, addr, __pa(addr));
	}

	size = MSM_PMEM_AUDIO_SIZE ;
	android_pmem_audio_pdata.start = MSM_PMEM_AUDIO_START_ADDR ;
	android_pmem_audio_pdata.size = size;
	pr_info("allocating %lu bytes (at %lx physical) for audio "
		"pmem arena\n", size , MSM_PMEM_AUDIO_START_ADDR);

	size = fb_size ? : MSM_FB_SIZE;
	addr = alloc_bootmem(size);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
		size, addr, __pa(addr));

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	/* RAM Console can't use alloc_bootmem(), since that zeroes the
         * region */
	size = 128 * SZ_1K;
	ram_console_resource[0].start = msm_fb_resources[0].end+1;
	ram_console_resource[0].end = ram_console_resource[0].start + size - 1;
	pr_info("allocating %lu bytes at (%lx physical) for ram console\n",
		size, (unsigned long)ram_console_resource[0].start);
	/* We still have to reserve it, though */
	reserve_bootmem(ram_console_resource[0].start,size,0);
#endif

	size = pmem_kernel_ebi1_size;
	if (size) {
		addr = alloc_bootmem_aligned(size, 0x100000);
		android_pmem_kernel_ebi1_pdata.start = __pa(addr);
		android_pmem_kernel_ebi1_pdata.size = size;
		pr_info("allocating %lu bytes at %p (%lx physical) for kernel"
			" ebi1 pmem arena\n", size, addr, __pa(addr));
	}
#ifdef CONFIG_ARCH_MSM7X27
	size = gpu_phys_size ? : MSM_GPU_PHYS_SIZE;
	addr = alloc_bootmem(size);
	kgsl_resources[1].start = __pa(addr);
	kgsl_resources[1].end = kgsl_resources[1].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for KGSL\n",
		size, addr, __pa(addr));
#endif
}

static void __init msm7x2x_map_io(void)
{
	msm_map_common_io();
	msm_msm7x2x_allocate_memory_regions();

	if (socinfo_init() < 0)
		BUG();

#ifdef CONFIG_CACHE_L2X0
	if (machine_is_msm7x27_surf() || machine_is_msm7x27_ffa()) {
		/* 7x27 has 256KB L2 cache:
			64Kb/Way and 4-Way Associativity;
			R/W latency: 3 cycles;
			evmon/parity/share disabled. */
		l2x0_init(MSM_L2CC_BASE, 0x00068012, 0xfe000000);
	}
#endif
}

MACHINE_START(MSM7X27_SURF, "QCT MSM7x27 SURF")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io		= msm7x2x_map_io,
	.init_irq	= msm7x2x_init_irq,
	.init_machine	= msm7x2x_init,
	.timer		= &msm_timer,
MACHINE_END

