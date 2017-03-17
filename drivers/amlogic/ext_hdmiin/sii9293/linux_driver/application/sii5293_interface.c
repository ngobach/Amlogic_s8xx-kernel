
#include "mhl_linuxdrv.h"
#include "../../driver/cra_drv/si_cra.h"
#include "../../component/rx/si_rx_video_mode_detection.h"

#define GET_VIDEO_INFO_FROM_TABLE

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void sii_set_standby(int bStandby)
{
	SiiRegWrite(RX_A__PD_TOT, !bStandby);
	return ;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int sii_get_pwr5v_status(void)
{
	char pwr5v;

	pwr5v = SiiRegRead(RX_A__STATE)&RX_M__STATE__PWR5V;

	return (pwr5v==0)?0:1;
}

// audio sampling frequency:
// 0x0 for 44.1 KHz
// 0x1 for Not indicated
// 0x2 for 48 KHz
// 0x3 for 32 KHz
// 0x4 for 22.05 KHz
// 0x6 for 24 kHz
// 0x8 for 88.2 kHz
// 0x9 for 768 kHz (192*4)
// 0xa for 96 kHz
// 0xc for 176.4 kHz
// 0xe for 192 kHz
extern unsigned int get_measured_audio_fs(void);
int sii_get_audio_sampling_freq(void)
{
	unsigned char freq;

	freq = (unsigned char)get_measured_audio_fs();

	return freq;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef GET_VIDEO_INFO_FROM_TABLE
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// sii5293 output signal horizontal parameters

int sii_get_h_active(void)
{
	unsigned char index = 0;

	if (gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER)
	{
		index = gDriverContext.input_video_mode_other;
		if (index >= NMB_OF_VIDEO_OTHER_MODES)
			return -1;
		return VideoModeTableOther[index].Active.H;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Active.H;
}

int sii_get_h_total(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Total.H;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Total.H;
}

int sii_get_hs_width(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].SyncWidth.H;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].SyncWidth.H;
}

int sii_get_hs_frontporch(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].SyncOffset.H;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].SyncOffset.H;
}

int sii_get_hs_backporch(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Blank.H - VideoModeTableOther[index].SyncOffset.H - VideoModeTableOther[index].SyncWidth.H;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Blank.H - VideoModeTable[index].SyncOffset.H - VideoModeTable[index].SyncWidth.H;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// sii5293 output signal vertical parameters

int sii_get_v_active(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Active.V;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Active.V;
}

int sii_get_v_total(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Total.V;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Total.V;
}

int sii_get_vs_width(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].SyncWidth.V;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].SyncWidth.V;
}

int sii_get_vs_frontporch(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].SyncOffset.V;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].SyncOffset.V;
}

int sii_get_vs_backporch(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Blank.V - VideoModeTableOther[index].SyncOffset.V - VideoModeTableOther[index].SyncWidth.V;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Blank.V - VideoModeTable[index].SyncOffset.V - VideoModeTable[index].SyncWidth.V;
}

int sii_get_vs_to_de(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Blank.V - VideoModeTableOther[index].SyncOffset.V;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Blank.V - VideoModeTable[index].SyncOffset.V;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// sii5293 output signal clock parameters

int sii_get_pixel_clock(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].PixClk*10000;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].PixClk*10000;
}

int sii_get_h_freq(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].HFreq*10000;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].HFreq*1000;
}

int sii_get_v_freq(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].VFreq;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].VFreq;
}

int sii_get_interlaced(void)
{
	unsigned char index = 0;

	if ( gDriverContext.input_video_mode == SI_VIDEO_MODE_PC_OTHER )
	{
		index = gDriverContext.input_video_mode_other;
		if ( index >= NMB_OF_VIDEO_OTHER_MODES )
			return -1;
		return VideoModeTableOther[index].Interlaced;
	}

	index = gDriverContext.input_video_mode & 0x7f;
	if ( (index==0) || (index>=NMB_OF_CEA861_VIDEO_MODES) )
		return -1;

	return VideoModeTable[index].Interlaced;
}

extern void SiiDrvRxGetSyncInfo(sync_info_type *p_sync_info);
int sii_get_hs_polar(void)
{
	sync_info_type sync_info;

	SiiDrvRxGetSyncInfo(&sync_info);

	return sync_info.HPol;
}

int sii_get_vs_polar(void)
{
	sync_info_type sync_info;

	SiiDrvRxGetSyncInfo(&sync_info);

	return sync_info.VPol;
}

#else

// !!! read h/v/sync info from 5293 registers, but the value seems not stable.

// offset definitions for registers.
#define MSK__VID_DE_PIXEL_BIT8_11	0x0F
#define MSK__VID_H_RES_BIT8_12		0x1F
#define MSK__VID_HS_WIDTH_BIT8_9	0x03
#define RX_A__VID_HFP2 				0x05a
#define MSK__VID_H_FP_BIT8_9		0x03
#define MSK__VID_DE_LINE_BIT8_10	0x07
#define MSK__VID_V_RES_BIT8_10		0x07
#define MSK__VID_VS_AVT_BIT0_5		0x3F
#define MSK__VID_V_FP_BIT0_5		0x3F

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// sii5293 output signal horizontal parameters

static int sii_get_h_active_reg(void)
{
	unsigned char high,low;

	high = SiiRegRead(RX_A__DE_PIX2)&MSK__VID_DE_PIXEL_BIT8_11;
	low = SiiRegRead(RX_A__DE_PIX1);

	return ( (high<<8) | low );
}

static int sii_get_h_total_reg(void)
{
	unsigned char high,low;

	high = SiiRegRead(RX_A__H_RESH)&MSK__VID_H_RES_BIT8_12;
	low = SiiRegRead(RX_A__H_RESL);

	return ( (high<<8) | low );
}

static int sii_get_hs_width_reg(void)
{
	unsigned char high,low;

	high = SiiRegRead(RX_A__VID_HAW2)&MSK__VID_HS_WIDTH_BIT8_9;
	low = SiiRegRead(RX_A__VID_HAW1);

	return ( (high<<8) | low );
}

static int sii_get_hs_frontporch_reg(void)
{
	unsigned char high,low;

	high = SiiRegRead(RX_A__VID_HFP2)&MSK__VID_H_FP_BIT8_9;
	low = SiiRegRead(RX_A__VID_HFP);

	return ( (high<<8) | low );
}

static int sii_get_hs_backporch_reg(void)
{
	int backporch = 0;

	backporch = sii_get_h_total_reg() - sii_get_h_active_reg() - sii_get_hs_frontporch_reg() - sii_get_hs_width_reg();

	return backporch;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// sii5293 output signal vertical parameters

static int sii_get_v_active_reg(void)
{
	unsigned char high,low;

	high = SiiRegRead(RX_A__DE_LINE2)&MSK__VID_DE_LINE_BIT8_10;
	low = SiiRegRead(RX_A__DE_LINE1);

	return ( (high<<8) | low );
}

static int sii_get_v_total_reg(void)
{
	unsigned char high,low;

	high = SiiRegRead(RX_A__V_RESH)&MSK__VID_V_RES_BIT8_10;
	low = SiiRegRead(RX_A__V_RESL);

	return ( (high<<8) | low );
}

static int sii_get_vs_width_reg(void)
{
	return 0;
}

static int sii_get_vs_frontporch_reg(void)
{
	unsigned char low;

	low = SiiRegRead(RX_A__VID_VFP)&MSK__VID_V_FP_BIT0_5;

	return low;
}

static int sii_get_vs_backporch_reg(void)
{
	return 0;
}

static int sii_get_vs_to_de_reg(void)
{
	unsigned char low;

	low = SiiRegRead(RX_A__VTAVL)&MSK__VID_VS_AVT_BIT0_5;

	return low;
}

extern uint16_t SiiDrvRxGetPixelFreq(void);
extern uint8_t SiiDrvRxGetVideoStatus();
int sii_get_video_timming(sii_video_timming_t *timming)
{
	uint8_t vid_stat_reg = SiiDrvRxGetVideoStatus();

	timming->h_active	= sii_get_h_active_reg();
	timming->h_total	= sii_get_h_total_reg();
	timming->hs_fp		= sii_get_hs_frontporch_reg();
	timming->hs_width	= sii_get_hs_width_reg();
	timming->hs_bp		= sii_get_hs_backporch_reg();

	timming->v_active	= sii_get_v_active_reg();
	timming->v_total	= sii_get_v_total_reg();
	timming->vs_fp		= sii_get_vs_frontporch_reg();
	timming->vs_to_de	= sii_get_vs_to_de_reg();

	timming->pixelfreq	= SiiDrvRxGetPixelFreq();
	timming->interlaced = (vid_stat_reg & RX_M__VID_STAT__INTERLACE) ? INTL : PROG;
	timming->hs_pol = (vid_stat_reg & RX_M__VID_STAT__HSYNC_POL) ? POS : NEG;
	timming->vs_pol = (vid_stat_reg & RX_M__VID_STAT__VSYNC_POL) ? POS : NEG;

	return 0;
}

#endif

