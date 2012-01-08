/*
 *     USI wireless module (Chipset:Atheros AR6002)initial power control & reset
 *     Reference: Page:26 Figure 7-2 Power-Up/-Down Timing
 *                             while asserting SYS_RST_L
 *
 *     [WIFI] Enable Wi-Fi Support, Please reference KernelConfig ->
 *     Device Drivers -> Network device support -> Wireless LAN ->
 *     Atheros AR6K... AND "HELP".
 */

/* [WIFI] Wi-Fi Support */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <mach/vreg.h>
#include <mach/gpio.h>

/* GPIO pin number */
#define GPIO_WLAN_RST_L	26
#define GPIO_WLAN_WAKES_MSM	29
#define GPIO_WLAN_CHIP_PWD_L	81

/* GPIO pin level */
#define GPIO_WLAN_HIGH		1
#define GPIO_WLAN_LOW		0

static char	isWlanState = 0;		// Wlan state : 0=unused	1=used

static unsigned wlan_config_power_on[] = {
        /* WLAN_RST_L */
        GPIO_CFG(GPIO_WLAN_RST_L, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
        /* WLAN_WAKES_MSM */
        GPIO_CFG(GPIO_WLAN_WAKES_MSM, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
        /* CHIP_PWD_L */
        GPIO_CFG(GPIO_WLAN_CHIP_PWD_L, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA)
};

static unsigned wlan_config_power_off[] = {
        /* WLAN_RST_L */
        GPIO_CFG(GPIO_WLAN_RST_L, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
        /* WLAN_WAKES_MSM */
        GPIO_CFG(GPIO_WLAN_WAKES_MSM, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
        /* CHIP_PWD_L */
        GPIO_CFG(GPIO_WLAN_CHIP_PWD_L, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA)
};


/*
 *	Get WLAN status.
 *
 *	Argument:
 *		VOID
 *	Return value:
 *		0		: WLAN state is OFF.
 *		1		: WLAN state is ON.
 *		other	: WLAN error.
 */
char getWlanState(void)
{
	return isWlanState;
}



/*
 *	Switch WLAN power
 *
 *	Argument:
 *		isPowerON	: Power ON/OFF.
 *			0		: OFF
 *			Other	: ON
 *		vreg_wlan	: pointer get from vreg_get.
 *	Return value:
 *		0		: Successful;
 *		other	: Fail.
 */
char wlan_power_switch(unsigned char isPowerON, struct vreg *vreg_wlan)
{
	char				rc = -ENOENT, state[1]={0xFF};
	struct file		*filp = (struct file *)-ENOENT;
	mm_segment_t 	oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	
	do {
		filp = filp_open("/sys/class/rfkill/rfkill0/state", O_RDONLY, S_IRUSR);
		if (IS_ERR(filp) || !filp->f_op) {
			printk(KERN_ERR "%s: rfkill BT state file filp_open error %d\n", __FUNCTION__, (int)filp);
			break;
		}
	
		if((rc=filp->f_op->read(filp, state, (size_t)1, &filp->f_pos)) < 0) {
			printk(KERN_ERR "%s: Read status from rfkill BT state file error %d\n", __FUNCTION__, rc);
			break;
		}

		switch(state[0]) {
		case '0':		/* Bluetooth state : OFF */
			if(isPowerON)
				rc = vreg_enable(vreg_wlan);        /* 1.2V Power UP! */
			else
				rc = vreg_disable(vreg_wlan);        /* 1.2V Power DOWN! */
		
			break;
		default:		/* If Bluetooth wroking, Doesn't touch power */
			rc = 0;
			break;
		}
		
		if (rc) {
			printk(KERN_ERR "%s: ERROR vreg_wlan %s failed (%d)\n",  __func__, isPowerON ? "Enable" : "Disable", rc);
			break;
		}
	} while(0);
	
	if (!IS_ERR(filp))
		filp_close(filp, NULL);
	
	set_fs(oldfs);
	isWlanState = (0 == rc) ? (isPowerON ? 1 : 0) : rc;
	return	rc;
}



/*
 *	WLAN power controller
 *
 *	Argument:
 *		isPowerON	: Power ON/OFF.
 *			0		: OFF
 *			Other	: ON
 *	Return value:
 *		0		: Successful;
 *		other	: Fail.
 */
int wlan_power_ctrl(unsigned char isPowerON)
{
        struct vreg *vreg_wlan;
        int pin, rc;

        printk(KERN_DEBUG "\n%s, Power=%s\n", __func__, isPowerON ? "ON" : "OFF");

        /* vreg_get parameter 1 (struct device *) is ignored */
        vreg_wlan = vreg_get(NULL, "msme2"); /* A&DVDD12 */

        if (IS_ERR(vreg_wlan)) {
                printk(KERN_ERR "%s: ERROR vreg_wlan get failed (%ld)\n",
                       __func__, PTR_ERR(vreg_wlan));
                isWlanState = PTR_ERR(vreg_wlan); 
                return PTR_ERR(vreg_wlan);
        }

        if (isPowerON) {
                /* Power ON */
                for (pin = 0; pin < ARRAY_SIZE(wlan_config_power_on); pin++) {
                        rc = gpio_tlmm_config(wlan_config_power_on[pin],
                                              GPIO_CFG_ENABLE);

/* rc always -EIO in MSM7X27_QRD, But work fine. */
/*
                        if (rc) {
                                printk(KERN_ERR
                                       "%s: ERROR gpio_tlmm_config(%#x)=%d, Power=%d\n",
                                       __func__, wlan_config_power_on[pin], rc, isPowerON);
                                isWlanState = -EIO;
                                return -EIO;
                        }
*/

                }

                /* WLAN_RST & CHIP_PWD_L = H */
                gpio_set_value(GPIO_WLAN_RST_L, GPIO_WLAN_HIGH);
                gpio_set_value(GPIO_WLAN_CHIP_PWD_L, GPIO_WLAN_HIGH);
				
		  rc = wlan_power_switch(isPowerON, vreg_wlan);
                if(rc)
			return rc;
				  
                mdelay(1);       /* Wait for 1.2V power stability AND Timing: Te */
                /* WLAN_RST = L */
                gpio_set_value(GPIO_WLAN_RST_L, GPIO_WLAN_LOW);
                udelay(5);                /* Time: Tf */
                /* WLAN_RST = H */
                gpio_set_value(GPIO_WLAN_RST_L, GPIO_WLAN_HIGH);
        } else {     
                /* Power OFF */
                for (pin = 0; pin < ARRAY_SIZE(wlan_config_power_off); pin++) {
                        rc = gpio_tlmm_config(wlan_config_power_off[pin],
                                              GPIO_CFG_ENABLE);
/* rc always -EIO in MSM7X27_QRD, But work fine. */
/*
                        if (rc) {
                                printk(KERN_ERR
                                       "%s: ERROR gpio_tlmm_config(%#x)=%d, Power=%d\n",
                                       __func__, wlan_config_power_off[pin], rc, isPowerON);
                                isWlanState = -EIO;
                                return -EIO;
                        }
*/

                }

                /* WLAN_RST = H, because pull-up in module */
                gpio_set_value(GPIO_WLAN_RST_L, GPIO_WLAN_HIGH);
                /* CHIP_PWD_L = L */
                gpio_set_value(GPIO_WLAN_CHIP_PWD_L, GPIO_WLAN_LOW);

		  rc = wlan_power_switch(isPowerON, vreg_wlan);
                if(rc)
			return rc;
        }
        return 0;
}

EXPORT_SYMBOL(wlan_power_ctrl);
EXPORT_SYMBOL(getWlanState);
/* [WIFI] Wi-Fi Support */
