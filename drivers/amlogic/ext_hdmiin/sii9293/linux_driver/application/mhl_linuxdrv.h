/*
 * SiIxxxx <Firmware or Driver>
 *
 * Copyright (C) 2011 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
*/

/**
 * @file mhl_driver.h
 *
 * @brief Main header file of the MHL Tx driver.
 *
 * $Author: Dave Canfield
 * $Rev: $
 * $Date: Jan 20, 2011
 *
 *****************************************************************************/


#if !defined(MHL_DRIVER_H)
#define MHL_DRIVER_H

#include <linux/amlogic/tvin/tvin_v4l2.h>
#include "../../../../../../../hardware/tvin/tvin_frontend.h"

#include "sii_hal.h"
#include <linux/device.h>
#include <mach/gpio.h>




#ifdef __cplusplus 
extern "C" { 
#endif  /* _defined (__cplusplus) */

/***** macro definitions *****************************************************/
#if defined(MAKE_5293_DRIVER)

#define MHL_DRIVER_NAME "sii5293drv"
#define MHL_DEVICE_NAME "sii-5293"
//#define CLASS_NAME	"video"
#define CLASS_NAME  "sii9293"
#define DEVNAME		"sii5293"
#define MHL_DEVNAME	"mhl"

#define NUMBER_OF_DEVS	2

#define DEVICE_EVENT	"DEVICE_EVENT"
#define MHL_EVENT		"MHL_EVENT"

/* Device events */
#define DEV_CONNECTION_CHANGE_EVENT "connection_change"
#define DEV_INPUT_VIDEO_MODE_EVENT "input_video_stable"
#define DEV_INPUT_AUDIO_STABLE_EVENT "input_audio_stable"

/* MHL events */
#define MHL_CONNECTED_EVENT	"connected"
#define MHL_DISCONNECTED_EVENT	"disconnected"
#define MHL_RAP_RECEIVED_EVENT	"received_rap"
#define MHL_RAP_ACKED_EVENT	"received_rapk"
#define MHL_RCP_RECEIVED_EVENT	"received_rcp"
#define MHL_RCP_ACKED_EVENT	"received_rcpk"
#define MHL_RCP_ERROR_EVENT	"received_rcpe"
#define MHL_UCP_RECEIVED_EVENT	"received_ucp"
#define MHL_UCP_ACKED_EVENT	"received_ucpk"
#define MHL_UCP_ERROR_EVENT	"received_ucpe"

#else   

#error "Need to add name and description strings for new drivers here!"

#endif


#define MHL_DRIVER_MINOR_MAX   1


/***** public type definitions ***********************************************/

typedef enum
{
    MHL_CONN = 1,   //default value is 0, so that when comes the notify first time will always effective.
    HDMI_CONN,
    NO_CONN,
} SourceConnection_t;

typedef struct {
    uint8_t chip_revision;      // chip revision
    bool_t  pwr5v_state;        // power 5v state
    bool_t  mhl_cable_state;    // mhl cable state
    SourceConnection_t connection_state;
    uint8_t input_video_mode;       // last determined video mode

    // "input_video_mode_other" is for vesa video mode idx,
    // i don't want to change "input_video_mode" from uint8_t to uint32_t
    // because the original sii5923 driver has too many association with the type "uint8_t input_video_mode"
    // so if input_video_mode is evaluated to SI_VIDEO_MODE_PC_OTHER, then "input_video_mode_other"
    // will has the index of vesa mode in array VideoModeTableOther[].
    uint32_t input_video_mode_other;

    uint8_t debug_i2c_address;
    uint8_t debug_i2c_offset;
    uint8_t debug_i2c_xfer_length;
    uint8_t devcap_remote_offset;   // last Device Capability register 
    uint8_t devcap_local_offset;    // last Device Capability register
    uint8_t rap_in_keycode;         // last RAP key code received.
    uint8_t rap_out_keycode;        // last RAP key code transmitted.
    uint8_t rap_out_statecode;      // last RAP state code transmitted
    uint8_t rcp_in_keycode;         // last RCP key code received.
    uint8_t rcp_out_keycode;        // last RCP key code transmitted.
    uint8_t rcp_out_statecode;      // last RCP state code transmitted
    uint8_t ucp_in_keycode;         // last UCP key code received.
    uint8_t ucp_out_keycode;        // last UCP key code transmitted.
    uint8_t ucp_out_statecode;      // last UCP state code transmitted
} MHL_DRIVER_CONTEXT_T, *PMHL_DRIVER_CONTEXT_T;

struct mhl_device_info {
	dev_t devnum;
	struct cdev *cdev;
	struct device *device;

};

typedef struct
{
    /* data */
    unsigned int    i2c_bus_index;
    gpio_t          gpio_reset;
    gpio_t          gpio_intr;
}sii5293_config;

struct device_info {
	dev_t devnum;
	struct cdev *cdev;
	struct device *device;
	struct class *dev_class;

	struct mhl_device_info *mhl;
    sii5293_config config;

	uint8_t my_rap_input_device;
	uint8_t my_rcp_input_device;
	uint8_t my_ucp_input_device;

	struct workqueue_struct *wq;
	struct delayed_work work_stable_video;
	struct delayed_work work_stable_audio;

};

typedef struct
{
	unsigned int 		cur_height;
	unsigned int 		cur_width;
	unsigned int 		cur_frame_rate;
	/* data */
}vdin_info_t;

typedef struct
{
	tvin_frontend_t tvin_frontend;
	vdin_parm_t	vdin_parm;
	vdin_info_t		vdin_info;
	unsigned int	vdin_started;
}sii5293_vdin;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// for video mode of hdmiin, it maybe not wholly same with sii9293 chip and vdin driver.
typedef enum
{
	HDMIIN_CEA_480P60	= 2,
	HDMIIN_CEA_720P60	= 4,
	HDMIIN_CEA_1080I60	= 5,
	HDMIIN_CEA_480I60	= 6,
	HDMIIN_CEA_1080P60	= 16,
	HDMIIN_CEA_576P50	= 17,
	HDMIIN_CEA_720P50	= 19,
	HDMIIN_CEA_1080I50	= 20,
	HDMIIN_CEA_576I50	= 21,
	HDMIIN_CEA_1080P50	= 31,
	HDMIIN_CEA_1080P24	= 32,
	HDMIIN_CEA_1080P30	= 34,
	HDMIIN_CEA_MAX 		= 60,

	HDMIIN_640X480_P_60		= 100,
	HDMIIN_800X600_P_60,
	HDMIIN_1024X768_P_60,
	HDMIIN_1152X864_P_60,
	HDMIIN_1280X768_P_60,
	HDMIIN_1280X800_P_60,
	HDMIIN_1280X960_P_60,
	HDMIIN_1280X1024_P_60,
	HDMIIN_1366X768_P_60,
	HDMIIN_1600X1200_P_60,

	HDMIIN_VMODE_MAX
}sii9293_vmode_e;

typedef struct
{
	char*				name;
	sii9293_vmode_e		vmode;

	unsigned int		h_active;
	unsigned int		h_total;
	unsigned int		v_active;
	unsigned int		v_total;
	unsigned int		frame_rate;
	unsigned int		interlaced;

	// Invert HS polarity, for HW regards HS active high.
	unsigned int hs_pol_inv;

	// Invert VS polarity, for HW regards VS active high.
	unsigned int vs_pol_inv;

	// Invert DE polarity, for HW regards DE active high.
	unsigned int de_pol_inv;

	// Invert FIELD polarity, for HW regards odd field when high.
	unsigned int field_pol_inv;

	// FIELD source select:
	// 1=Use external FIELD signal, ignore internal FIELD detection result;
	// 0=Use internal FIELD detection result, ignore external input FIELD signal.
	unsigned int ext_field_sel;

	// DE mode control:
	// 0=Ignore input DE signal, use internal detection to to determine active pixel;
	// 1=Rsrv;
	// 2=During internal detected active region, if input DE goes low, replace input data with the last good data;
	// 3=Active region is determined by input DE, no internal detection.
	unsigned int de_mode;

	// Map input data to form YCbCr.
	// Use 0 if input is YCbCr;
	// Use 1 if input is YCrCb;
	// Use 2 if input is CbCrY;
	// Use 3 if input is CbYCr;
	// Use 4 if input is CrYCb;
	// Use 5 if input is CrCbY;
	// 6,7=Rsrv.
	unsigned int data_comp_map;

	// 422 to 444 conversion control:
	// 0=No convertion; 1=Rsrv;
	// 2=Convert 422 to 444, use previous C value;
	// 3=Convert 422 to 444, use average C value.
	unsigned int mode_422to444;

	// Invert dvin_clk_in for ease of data capture.
	unsigned int dvin_clk_inv;

	// Controls which edge of HS/VS (post polarity control) the active pixel/line is related:
	// Bit 0: HS and active pixel relation.
	//  0=Start of active pixel is counted from the rising edge of HS;
	//  1=Start of active pixel is counted from the falling edge of HS;
	// Bit 1: VS and active line relation.
	//  0=Start of active line is counted from the rising edge of VS;
	//  1=Start of active line is counted from the falling edge of VS.
	unsigned int vs_hs_tim_ctrl;

	// For internal FIELD detection:
	// Minimum clock cycles allowed for HS active edge to lead before VS active edge in odd field. Failing it the field is even.
	unsigned int hs_lead_vs_odd_min;

	// For internal FIELD detection:
	// Maximum clock cycles allowed for HS active edge to lead before VS active edge in odd field. Failing it the field is even.
	unsigned int hs_lead_vs_odd_max;

	// Number of clock cycles between HS active edge to first active pixel, in even field.
	unsigned int active_start_pix_fe;

	// Number of clock cycles between HS active edge to first active pixel, in odd field.
	unsigned int active_start_pix_fo;

	// Number of clock cycles between VS active edge to first active line, in even field.
	unsigned int active_start_line_fe;

	// Number of clock cycles between VS active edge to first active line, in odd field.
	unsigned int active_start_line_fo;

	tvin_sig_fmt_t		tvin_mode;
}sii_video_timming_link;

typedef struct
{
	tvin_frontend_t	tvin_frontend;
	vdin_parm_t		vdin_param;
	unsigned int	vdin_started;
	sii9293_vmode_e	vmode;
}sii9293_tvin_t;

typedef struct
{
    unsigned int        user_cmd; // 0 to disable from user
                                  // 1 to enable, driver will trigger to vdin-stop
                                  // 2 to enable, driver will trigger to vdin-start
                                  // 3 to enable, driver will trigger to vdin-start/vdin-stop
                                  // 4 to enable, driver will not trigger to vdin-start/vdin-stop
                                  // 0xff to enable, and driver will NOT trigger on signal-lost/vdin-stop, singal-get/vdin-start
    unsigned int        cable_status; // 1 for cable plug in, 0 for cable plug out
    unsigned int        signal_status; // external hdmi cable is insert or not
}sii9293_info_t;

#define HDMIIN_FRAME_SKIP_MECHANISM 1

#ifdef HDMIIN_FRAME_SKIP_MECHANISM
// frame skip configuration is needed as:
//     for following status: standby/powerup, cable plug out/in, etc
//     we need drop some frame for HDMIIN device will still keep old frames
// the skip num maybe different in each status.

#define FRAME_SKIP_NUM_NORMAL	1
#define FRAME_SKIP_NUM_STANDBY	1
#define FRAME_SKIP_NUM_CABLE	1

typedef enum
{
	SKIP_STATUS_NORMAL 	= 0,
	SKIP_STATUS_STANDBY = 1,
	SKIP_STATUS_CABLE 	= 2,
	SKIP_STATUS_MAX
}skip_status_e;

typedef struct
{
	unsigned char skip_num_normal;
	unsigned char skip_num_standby;
	unsigned char skip_num_cable;
}sii9293_frame_skip_t;

#endif

/***** global variables ********************************************/

extern MHL_DRIVER_CONTEXT_T gDriverContext;
extern struct device_info *devinfo;

/***** public function prototypes ********************************************/
/**
 * \defgroup driver_public_api Driver Public API
 * @{
 */
int send_sii5293_uevent(struct device *device, const char *event_cat,
			const char *event_type, const char *event_data);

void SiiConnectionStateNotify(bool_t connect);

#ifdef __cplusplus
}
#endif  /* _defined (__cplusplus) */

#endif /* _defined (MHL_DRIVER_H) */
