/*
 * Amlogic Meson HDMI Transmitter Driver
 * hdmitx driver-----------HDMI_TX
 * Copyright (C) 2013 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <mach/am_regs.h>
#include <mach/clock.h>
#include <mach/power_gate.h>
#include <linux/clk.h>
#include <mach/clock.h>
#include <linux/amlogic/vout/vinfo.h>
#include <linux/amlogic/vout/enc_clk_config.h>
#include <linux/amlogic/hdmi_tx/hdmi_tx_compliance.h>

typedef struct {
	char *ReceiverBrandName;
	char *ReceiverProductName;
	unsigned char blk0_chksum;
	unsigned char audio_N;
}special_tv;

static special_tv special_audio_N_tv_tab[]= {
	/*SONY KDL-32R300B*/
	{
		.ReceiverBrandName="SNY",
		.ReceiverProductName="SONY",
		.blk0_chksum=0xf8,
		.audio_N=COMP_AUDIO_SET_N_6144x2,
	},
	/*TCL L19F3270B*/
	{
		.ReceiverBrandName="TCL",
		.ReceiverProductName="MST6M16",
		.blk0_chksum=0xa9,
		.audio_N=COMP_AUDIO_SET_N_6144x2,
	},
	/*Panasonic TH-32A400C*/
	{
		.ReceiverBrandName="MEI",
		.ReceiverProductName="Panasonic-TV",
		.blk0_chksum=0x28,
		.audio_N=COMP_AUDIO_SET_N_6144x2,
	},
	/*32PFL3390/T3*/
	{
		.ReceiverBrandName="PHL",
		.ReceiverProductName="32PFL3390/T3",
		.blk0_chksum=0x84,
		.audio_N=COMP_AUDIO_SET_N_6144x2,
	},

	{
/*
TV: HISENSE LED24K316

EDID block:
00 ff ff ff ff ff ff 00 20 a3 29 00 01 00 00 00
23 12 01 03 80 73 41 78 0a f3 30 a7 54 42 aa 26
0f 50 54 25 c8 00 31 59 45 59 61 4f 01 01 01 01
01 01 01 01 01 01 02 3a 80 18 71 38 2d 40 58 2c
45 00 80 88 42 00 00 1e 8c 0a d0 8a 20 e0 2d 10
10 3e 96 00 80 88 42 00 00 18 00 00 00 fc 00 48
44 4d 49 20 20 20 20 0a 20 20 20 20 00 00 00 fd
00 32 55 1f 45 0f 00 0a 20 20 20 20 20 20 01 00

02 03 1d 72 23 09 07 07 47 90 05 04 03 11 14 1f
83 01 00 00 68 03 0c 00 30 00 38 2d 00 f3 39 80
18 71 38 2d 40 58 2c 45 00 c4 8e 21 00 00 1e d6
09 80 a0 20 e0 2d 10 08 60 22 00 12 8e 21 08 08
18 01 1d 00 bc 52 d0 1e 20 b8 28 55 40 c4 8e 21
00 00 1e 8c 0a d0 90 20 40 31 20 0c 40 55 00 c4
8e 21 00 00 18 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 85
*/
		.ReceiverBrandName="HEC",
		.ReceiverProductName="HDMI",
		.blk0_chksum=0x00,
		.audio_N=COMP_AUDIO_SET_N_6144x4,
	},

	{
/*
TV: TCL L19P21

EDID block:
00 ff ff ff ff ff ff 00 50 6c 00 00 00 00 00 00
05 14 01 03 80 3a 20 78 0a 0d c9 a0 57 47 98 27
12 48 4c 23 08 00 81 80 01 01 01 01 01 01 01 01
01 01 01 01 01 01 66 21 50 b0 51 00 1b 30 40 70
36 00 44 40 21 00 00 18 0e 1f 00 80 51 00 1e 30
40 80 37 00 10 4e 42 00 00 1e 00 00 00 fc 00 4d
54 32 33 4c 0a 20 20 20 20 20 20 20 00 00 00 fd
00 32 3c 0e 46 10 00 0a 20 20 20 20 20 20 01 85

02 03 24 70 4f 14 13 12 11 16 15 05 04 03 02 07
06 01 1f 10 23 09 07 07 83 01 00 00 67 03 0c 00
10 00 b8 2d 01 1d 80 d0 72 1c 16 20 10 2c 25 80
44 40 21 00 00 9e 01 1d 80 18 71 1c 16 20 58 2c
25 00 44 40 21 00 00 98 01 1d 00 bc 52 d0 1e 20
b8 28 55 40 44 40 21 00 00 1e 01 1d 00 72 51 d0
1e 20 6e 28 55 00 44 40 21 00 00 1e 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6b

*/
		.ReceiverBrandName="TCL",
		.ReceiverProductName="MT23L",
		.blk0_chksum=0x85,
		.audio_N=COMP_AUDIO_SET_N_6144x4,
	},

	{
/*
TV: SKYWORTH 43E6200/49E6200/55E6200

EDID block:
00 ff ff ff ff ff ff 00 4d 79 01 00 01 01 01 01
22 18 01 03 81 00 00 78 0a ee 9d a3 54 4c 99 26
0f 47 4a a1 08 00 d1 c0 8b c0 61 40 45 40 31 40
01 01 01 01 01 01 02 3a 80 18 71 38 2d 40 58 2c
45 00 00 00 00 00 00 1e 66 21 50 b0 51 00 1b 30
40 70 36 00 00 00 00 00 00 1e 00 00 00 fc 00 53
6b 79 77 6f 72 74 68 20 54 56 0a 20 00 00 00 fd
00 32 4b 1e 50 17 00 0a 20 20 20 20 20 20 01 8e

02 03 2c f2 4d 01 03 04 05 07 90 12 13 14 16 9f
20 22 26 09 07 07 11 17 50 83 01 00 00 6e 03 0c
00 10 00 b8 44 20 00 80 01 02 03 04 8c 0a d0 8a
20 e0 2d 10 10 3e 96 00 00 00 00 00 00 18 8c 0a
d0 90 20 40 31 20 0c 40 55 00 00 00 00 00 00 18
01 1d 00 bc 52 d0 1e 20 b8 28 55 40 00 00 00 00
00 1e 01 1d 80 d0 72 1c 16 20 10 2c 25 80 00 00
00 00 00 9e 00 00 00 00 00 00 00 00 00 00 00 49
*/
		.ReceiverBrandName="SKY",
		.ReceiverProductName="Skyworth",
		.blk0_chksum=0x8e,
		.audio_N=COMP_AUDIO_SET_N_5120x2,
	},

	{
/*
TV: SKYWORTH 65S9500

EDID block:
00 ff ff ff ff ff ff 00 4d 79 01 00 01 01 01 01
0e 18 01 03 81 00 00 78 0a ee 9d a3 54 4c 99 26
0f 47 4a a1 08 00 d1 c0 8b c0 61 40 45 40 31 40
01 01 01 01 01 01 02 3a 80 18 71 38 2d 40 58 2c
45 00 00 00 00 00 00 1e 66 21 50 b0 51 00 1b 30
40 70 36 00 00 00 00 00 00 1e 00 00 00 fc 00 53
6b 79 77 6f 72 74 68 20 54 56 0a 20 00 00 00 fd
00 32 4b 1e 50 17 00 0a 20 20 20 20 20 20 01 a2

02 03 34 f2 4d 01 03 04 05 07 90 12 13 14 16 9f
20 22 26 09 07 07 11 17 50 83 01 00 00 72 03 0c
00 40 00 b8 44 20 c0 84 01 02 03 04 01 41 00 00
e3 05 03 01 8c 0a d0 8a 20 e0 2d 10 10 3e 96 00
00 00 00 00 00 18 8c 0a d0 90 20 40 31 20 0c 40
55 00 00 00 00 00 00 18 01 1d 00 bc 52 d0 1e 20
b8 28 55 40 00 00 00 00 00 1e 01 1d 80 d0 72 1c
16 20 10 2c 25 80 00 00 00 00 00 9e 00 00 00 1b
*/
		.ReceiverBrandName="SKY",
		.ReceiverProductName="Skyworth",
		.blk0_chksum=0xa2,
		.audio_N=COMP_AUDIO_SET_N_5120x2,
	},
};

static special_tv samsung_future_tv_tab[]= {
    /*UA50HU7000JXXZ*/
    {
        .ReceiverBrandName="SAM",
        .ReceiverProductName="SAMSUNG",
    },
};


/*
 * # cat /sys/class/amhdmitx/amhdmitx0/edid
 * Receiver Brand Name: GSM
 * Receiver Product Name: LG
 * blk0 chksum: 0xe7
 *
 * recoginze_tv()
 * parameters:
 *      brand_name: the name of "Receiver Brand Name"
 *      prod_name: the name of "Receiver Product Name"
 *      blk0_chksum: the value of blk0 chksum
 */
static int recoginze_tv(hdmitx_dev_t* hdev, char *brand_name, char *prod_name, unsigned char blk0_chksum)
{
    if ((strncmp(hdev->RXCap.ReceiverBrandName, brand_name, strlen(brand_name)) == 0) && \
        (strncmp(hdev->RXCap.ReceiverProductName, prod_name, strlen(prod_name)) == 0) && \
        (hdev->RXCap.blk0_chksum == blk0_chksum))
        return 1;
    else
        return 0;
}

static int is_special_tv = 0;
int hdmitx_is_special_tv(void)
{
    if (is_special_tv)
        return 1;
    else
        return 0;
}

static int is_support_4k_60(rx_cap_t *pRXCap)
{
    int i;

    for (i = 0 ; i < pRXCap->VIC_count; i++) {
        switch (pRXCap->VIC[i]) {
        case HDMI_3840x2160p60_16x9:
        case HDMI_3840x2160p50_16x9:
        case HDMI_4096x2160p50_256x135:
        case HDMI_4096x2160p60_256x135:
            printk("support 4k 60/50\n");
            return 1;
            break;
        default:
            break;
        }
    }
    return 0;
}

/*
 * hdmitx_special_handler_video()
 */
void hdmitx_special_handler_video(hdmitx_dev_t* hdev)
{
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(samsung_future_tv_tab); i++) {
        if ((strncmp(hdev->RXCap.ReceiverBrandName, samsung_future_tv_tab[i].ReceiverBrandName, strlen(samsung_future_tv_tab[i].ReceiverBrandName)) == 0)
            && (strncmp(hdev->RXCap.ReceiverProductName, samsung_future_tv_tab[i].ReceiverProductName, strlen(samsung_future_tv_tab[i].ReceiverProductName)) == 0)
            && (is_support_4k_60(&hdev->RXCap))) {
            is_special_tv = 1;
            return;
        }
    }
    is_special_tv = 0;
}

/*
 * hdmitx_special_handler_audio()
 */
void hdmitx_special_handler_audio(hdmitx_dev_t* hdev)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(special_audio_N_tv_tab); i++) {
		if (recoginze_tv(hdev, special_audio_N_tv_tab[i].ReceiverBrandName, special_audio_N_tv_tab[i].ReceiverProductName, special_audio_N_tv_tab[i].blk0_chksum))
			hdev->HWOp.CntlMisc(hdev, MISC_COMP_AUDIO, special_audio_N_tv_tab[i].audio_N);
	}

#if 0
	// TODO

	if (recoginze_tv(hdev, "SAM", "SAMSUNG", 0x22)) {
        hdev->HWOp.CntlMisc(hdev, MISC_COMP_AUDIO, COMP_AUDIO_SET_N_6144x2);
    }
    if (recoginze_tv(hdev, "GSM", "LG", 0xE7)) {
        hdev->HWOp.CntlMisc(hdev, MISC_COMP_AUDIO, COMP_AUDIO_SET_N_6144x3);
    }
#endif
}
