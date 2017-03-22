
#include <mach/am_regs.h>
#include <mach/power_gate.h>
#include <linux/amlogic/tvin/tvin_v4l2.h>
#include "../../../../drivers/amlogic/tvin/tvin_frontend.h"
#include "mhl_linuxdrv.h"
#include "../../driver/cra_drv/si_cra.h"
#include "sii5293_interface.h"

#define SII9293_VDIN_PORT TVIN_PORT_DVIN0;
// 0 for vdin0, 1 for vdin1
#define SII9293_VDIN_INDEX	0

#ifdef HDMIIN_FRAME_SKIP_MECHANISM
extern unsigned int flag_skip_status ;
extern unsigned int flag_skip_enable ;
extern sii9293_frame_skip_t sii9293_skip;

#endif

extern int start_tvin_service(int no ,vdin_parm_t *para);
extern int stop_tvin_service(int no);
extern void set_invert_top_bot(bool invert_flag);

void enable_vdin_pinmux(void)
{

	// disable LCD pinmux
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_0, READ_CBUS_REG(PERIPHS_PIN_MUX_0) & 
	 ( ~(1       | // LCD_R 2/3/4/5/6/7
		(1 << 1)  | // LCD_R 0/1
		(1 << 2)  | // LCD_G 2/3/4/5/6/7
		(1 << 3)  | // LCD_G 0/1
		(1 << 4)  | // LCD_B 2/3/4/5/6/7
		(1 << 5)  | // LCD_B 0/1
		(1 << 18) | // LCD_HS
		(1 << 19)) )    // LCD_VS
		);

	// disable TCON pinmux
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_8, READ_CBUS_REG(PERIPHS_PIN_MUX_8) & 
	 ( ~((1 << 19) | // TCON_VCOM
		(1 << 20) | // TCON_CPH3
		(1 << 21) | // TCON_CPH2
		(1 << 22) | // TCON_CPH1
		(1 << 23) | // TCON_STH1
		(1 << 24) | // TCON_STV1
		(1 << 25) | // TCON_CPV
		(1 << 26) | // TCON_VCOM
		(1 << 27) | // TCON_OEV
		(1 << 28)) )   // TCON_OEH
		);

	// disable ENC pinmux
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_7, READ_CBUS_REG(PERIPHS_PIN_MUX_7) & 
	 ( ~(1     | // ENC_0
		(1 << 1)  | // ENC_1
		(1 << 2)  | // ENC_2
		(1 << 3)  | // ENC_3
		(1 << 4)  | // ENC_4
		(1 << 5)  | // ENC_5
		(1 << 6)  | // ENC_6
		(1 << 7)  | // ENC_7
		(1 << 8)  | // ENC_8
		(1 << 9)  | // ENC_9
		(1 << 10) | // ENC_10
		(1 << 11) | // ENC_11
		(1 << 12) | // ENC_12
		(1 << 13) | // ENC_13
		(1 << 14) | // ENC_14
		(1 << 15) | // ENC_15
		(1 << 16) | // ENC_16(eNet_0)
		(1 << 17)) )    // ENC_17(eNet_1)
		);

	// disable PWM pinmux
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_3, READ_CBUS_REG(PERIPHS_PIN_MUX_3) & 
	 ( ~((1 << 24) | // PWM_C
		(1 << 25) | // PWM_C
		(1 << 26)))    // PWM_D
		);
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_7, READ_CBUS_REG(PERIPHS_PIN_MUX_7) & 
	 ( ~((1 << 26) | // PWM_VS
		(1 << 27) | // PWM_VS
		(1 << 28)))    // PWM_VS
		);

	// disable VGA pinmux
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_0, READ_CBUS_REG(PERIPHS_PIN_MUX_0) & 
	 ( ~((1 << 20) | // VGA_HS
		(1 << 21)))    // VGA_VS
		);

	// enable DVIN pinmux
	WRITE_CBUS_REG(PERIPHS_PIN_MUX_0, READ_CBUS_REG(PERIPHS_PIN_MUX_0) | 
	 ( (1 << 6)  | // DVIN R/G/B 0/1/2/3/4/5/6/7
		(1 << 7)  | // DVIN_CLK
		(1 << 8)  | // DVIN_HS
		(1 << 9)  | // DVIN_VS
		(1 << 10))    // DVIN_DE
		);
}

static int sii5293_tvin_support(struct tvin_frontend_s *fe, enum tvin_port_e port)
{
	if(port == TVIN_PORT_DVIN0)
		return 0;
	else
		return -1;
}
static int sii5293_tvin_open(tvin_frontend_t *fe, enum tvin_port_e port)
{
	sii5293_vdin *devp = container_of(fe,sii5293_vdin,tvin_frontend);

	/*copy the vdin_parm_s to local device parameter*/
	if( !memcpy(&devp->vdin_parm,fe->private_data,sizeof(vdin_parm_t)) )
	{
		printk("[%s] copy vdin parm error.\n",__func__);
	}

	enable_vdin_pinmux();

	WRITE_MPEG_REG_BITS(VDIN_ASFIFO_CTRL2, 0x39, 2, 6); 

	return 0;
}

static void sii5293_tvin_close(struct tvin_frontend_s *fe)
{
	return ;          
}
static void sii5293_tvin_start(struct tvin_frontend_s *fe, enum tvin_sig_fmt_e fmt)
{
	return ;
}
static void sii5293_tvin_stop(struct tvin_frontend_s *fe, enum tvin_port_e port)
{
	return ;      
}
static int sii5293_tvin_isr(struct tvin_frontend_s *fe, unsigned int hcnt64)
{
#ifdef HDMIIN_FRAME_SKIP_MECHANISM
	static unsigned int cnt = 0;
	unsigned int max = 0;

	if( flag_skip_status >= SKIP_STATUS_MAX )
		return 0;

	if( SKIP_STATUS_NORMAL == flag_skip_status )
		max = sii9293_skip.skip_num_normal;
	else if( SKIP_STATUS_STANDBY == flag_skip_status )
		max = sii9293_skip.skip_num_standby;
	else if( SKIP_STATUS_CABLE == flag_skip_status )
		max = sii9293_skip.skip_num_cable;

	if( flag_skip_enable == 1 )
	{
		flag_skip_enable = 0;
		cnt = 0;
	}

	if( cnt < max )
	{
		cnt ++;
		printk("sii9293 skip type = %d, cnt = %d, max = %d\n", flag_skip_status, cnt, max);
		return TVIN_BUF_SKIP;
	}
	else if( cnt == max )
	{
		cnt = 0xffffffff;
		flag_skip_status = SKIP_STATUS_NORMAL;
	}
	
#endif

	return 0;
}

static struct tvin_decoder_ops_s sii5293_tvin_dec_ops = {
				.support 		= sii5293_tvin_support,
				.open 			= sii5293_tvin_open,
				.close 			= sii5293_tvin_close,
				.start  		= sii5293_tvin_start,
				.stop  			= sii5293_tvin_stop,
				.decode_isr 	= sii5293_tvin_isr, 
};

static void sii5293_tvin_get_sig_propery(struct tvin_frontend_s *fe, struct tvin_sig_property_s *prop)
{
	prop->color_format = TVIN_RGB444;
	prop->dest_cfmt = TVIN_YUV422;
	prop->decimation_ratio = 0;

	return ;
}

static struct tvin_state_machine_ops_s sii5293_tvin_sm_ops = {
		.get_sig_propery = sii5293_tvin_get_sig_propery,
};

int sii5293_register_tvin_frontend(struct tvin_frontend_s *frontend)
{
	int ret = 0;

	ret = tvin_frontend_init(frontend, &sii5293_tvin_dec_ops, &sii5293_tvin_sm_ops, 0);
	if( ret != 0 )
	{
		printk("[%s] init tvin frontend failed = %d\n", __FUNCTION__, ret);
		return -1;
	}

	ret = tvin_reg_frontend(frontend);
	if( ret != 0 )
	{
		printk("[%s] register tv in frontend failed = %d\n", __FUNCTION__, ret);
		return -2;
	}

	return 0;
}

extern int debug_level;
void sii5293_config_dvin (unsigned int hs_pol_inv,             // Invert HS polarity, for HW regards HS active high.
						unsigned int vs_pol_inv,             // Invert VS polarity, for HW regards VS active high.
						unsigned int de_pol_inv,             // Invert DE polarity, for HW regards DE active high.
						unsigned int field_pol_inv,          // Invert FIELD polarity, for HW regards odd field when high.
						unsigned int ext_field_sel,          // FIELD source select:
																		  // 1=Use external FIELD signal, ignore internal FIELD detection result;
																		  // 0=Use internal FIELD detection result, ignore external input FIELD signal.
						unsigned int de_mode,                // DE mode control:
																		  // 0=Ignore input DE signal, use internal detection to to determine active pixel;
																		  // 1=Rsrv;
																		  // 2=During internal detected active region, if input DE goes low, replace input data with the last good data;
																		  // 3=Active region is determined by input DE, no internal detection.
						unsigned int data_comp_map,          // Map input data to form YCbCr.
																		  // Use 0 if input is YCbCr;
																		  // Use 1 if input is YCrCb;
																		  // Use 2 if input is CbCrY;
																		  // Use 3 if input is CbYCr;
																		  // Use 4 if input is CrYCb;
																		  // Use 5 if input is CrCbY;
																		  // 6,7=Rsrv.
						unsigned int mode_422to444,          // 422 to 444 conversion control:
																		  // 0=No convertion; 1=Rsrv;
																		  // 2=Convert 422 to 444, use previous C value;
																		  // 3=Convert 422 to 444, use average C value.
						unsigned int dvin_clk_inv,           // Invert dvin_clk_in for ease of data capture.
						unsigned int vs_hs_tim_ctrl,         // Controls which edge of HS/VS (post polarity control) the active pixel/line is related:
																		  // Bit 0: HS and active pixel relation.
																		  //  0=Start of active pixel is counted from the rising edge of HS;
																		  //  1=Start of active pixel is counted from the falling edge of HS;
																		  // Bit 1: VS and active line relation.
																		  //  0=Start of active line is counted from the rising edge of VS;
																		  //  1=Start of active line is counted from the falling edge of VS.
						unsigned int hs_lead_vs_odd_min,     // For internal FIELD detection:
																		  // Minimum clock cycles allowed for HS active edge to lead before VS active edge in odd field. Failing it the field is even.
						unsigned int hs_lead_vs_odd_max,     // For internal FIELD detection:
																		  // Maximum clock cycles allowed for HS active edge to lead before VS active edge in odd field. Failing it the field is even.
						unsigned int active_start_pix_fe,    // Number of clock cycles between HS active edge to first active pixel, in even field.
						unsigned int active_start_pix_fo,    // Number of clock cycles between HS active edge to first active pixel, in odd field.
						unsigned int active_start_line_fe,   // Number of clock cycles between VS active edge to first active line, in even field.
						unsigned int active_start_line_fo,   // Number of clock cycles between VS active edge to first active line, in odd field.
						unsigned int line_width,             // Number_of_pixels_per_line
						unsigned int field_height)           // Number_of_lines_per_field
{
	unsigned int data32;

	if ( debug_level > 0 )
		printk("[%s] config:\n\
		hs_pol_inv = %d\n\
		vs_pol_inv = %d\n\
		de_pol_inv = %d\n\
		field_pol_inv = %d\n\
		ext_field_sel = %d\n\
		de_mode = %d\n\
		data_comp_map = %d\n\
		mode_422to444 = %d\n\
		dvin_clk_inv = %d\n\
		vs_hs_tim_ctrl = %d\n\
		hs_lead_vs_odd_min = %d\n\
		hs_lead_vs_odd_max = %d\n\
		active_start_pix_fe = %d\n\
		active_start_pix_fo = %d\n\
		active_start_line_fe = %d\n\
		active_start_line_fo = %d\n\
		line_width = %d\n\
		field_height = %d\n",
		__FUNCTION__,
		hs_pol_inv,
		vs_pol_inv,
		de_pol_inv,
		field_pol_inv,
		ext_field_sel,
		de_mode,
		data_comp_map,
		mode_422to444,
		dvin_clk_inv,
		vs_hs_tim_ctrl,
		hs_lead_vs_odd_min,
		hs_lead_vs_odd_max,
		active_start_pix_fe,
		active_start_pix_fo,
		active_start_line_fe,
		active_start_line_fo,
		line_width,
		field_height );

	// Program reg DVIN_CTRL_STAT: disable DVIN
	WRITE_MPEG_REG(DVIN_CTRL_STAT, 0);

	// Program reg DVIN_FRONT_END_CTRL
	data32 = 0;
	data32 |= (hs_pol_inv       & 0x1)  << 0;
	data32 |= (vs_pol_inv       & 0x1)  << 1;
	data32 |= (de_pol_inv       & 0x1)  << 2;
	data32 |= (field_pol_inv    & 0x1)  << 3;
	data32 |= (ext_field_sel    & 0x1)  << 4;
	data32 |= (de_mode          & 0x3)  << 5;
	data32 |= (mode_422to444    & 0x3)  << 7;
	data32 |= (dvin_clk_inv     & 0x1)  << 9;
	data32 |= (vs_hs_tim_ctrl   & 0x3)  << 10;
	WRITE_MPEG_REG(DVIN_FRONT_END_CTRL, data32);

	// Program reg DVIN_HS_LEAD_VS_ODD
	data32 = 0;
	data32 |= (hs_lead_vs_odd_min & 0xfff) << 0;
	data32 |= (hs_lead_vs_odd_max & 0xfff) << 16;
	WRITE_MPEG_REG(DVIN_HS_LEAD_VS_ODD, data32);

	// Program reg DVIN_ACTIVE_START_PIX
	data32 = 0;
	data32 |= (active_start_pix_fe & 0xfff) << 0;
	data32 |= (active_start_pix_fo & 0xfff) << 16;
	WRITE_MPEG_REG(DVIN_ACTIVE_START_PIX, data32);

	// Program reg DVIN_ACTIVE_START_LINE
	data32 = 0;
	data32 |= (active_start_line_fe & 0xfff) << 0;
	data32 |= (active_start_line_fo & 0xfff) << 16;
	WRITE_MPEG_REG(DVIN_ACTIVE_START_LINE, data32);

	// Program reg DVIN_DISPLAY_SIZE
	data32 = 0;
	data32 |= ((line_width-1)   & 0xfff) << 0;
	data32 |= ((field_height-1) & 0xfff) << 16;
	WRITE_MPEG_REG(DVIN_DISPLAY_SIZE, data32);

	// Program reg DVIN_CTRL_STAT, and enable DVIN
	data32 = 0;
	data32 |= 1                     << 0;
	data32 |= (data_comp_map & 0x7) << 1;
	WRITE_MPEG_REG(DVIN_CTRL_STAT, data32);
//    printk("[%s] end !\n", __FUNCTION__);
} /* config_dvin */

void sii9293_stop_tvin(sii9293_tvin_t *info)
{
	if( info->vdin_started == 0 )
	  return ;

	stop_tvin_service(SII9293_VDIN_INDEX);
	set_invert_top_bot(false);
	CLK_GATE_OFF(MISC_DVIN);
	info->vdin_started = 0;
	printk("[%s]: stop vdin\n", __FUNCTION__);
	return ;
}


int sii9293_start_tvin(sii9293_tvin_t *info, sii_video_timming_link *timming)
{
	vdin_parm_t para;
	sii_video_timming_link tmp;

	memcpy(&tmp, timming, sizeof(sii_video_timming_link));

	if ( (info == NULL) || (timming == NULL) )
		return -1;

	printk("[%s] start for hdmiin mode %d\n", __FUNCTION__, timming->vmode);

	if(info->vdin_started)
	{
		if ( info->vmode != timming->vmode )
		{
			stop_tvin_service(SII9293_VDIN_INDEX);
			info->vdin_started=0;
			printk("[%s]: stop vdin\n", __FUNCTION__);
		}
	}

	CLK_GATE_ON(MISC_DVIN);

	if ( info->vdin_started == 0 )
	{
		sii5293_config_dvin(tmp.hs_pol_inv,
							tmp.vs_pol_inv,
							tmp.de_pol_inv,
							tmp.field_pol_inv,
							tmp.ext_field_sel,
							tmp.de_mode,
							tmp.data_comp_map,
							tmp.mode_422to444,
							tmp.dvin_clk_inv,
							tmp.vs_hs_tim_ctrl,
							tmp.hs_lead_vs_odd_min,
							tmp.hs_lead_vs_odd_max,
							tmp.active_start_pix_fe,
							tmp.active_start_pix_fo,
							tmp.active_start_line_fe,
							tmp.active_start_line_fo,
							tmp.h_total,
							tmp.v_total );

		memset( &para, 0, sizeof(vdin_parm_t));
		para.port  = SII9293_VDIN_PORT;
		para.frame_rate = tmp.frame_rate;
		para.h_active = tmp.h_active;
		para.v_active = tmp.v_active;
		para.fmt = tmp.tvin_mode;

		if ( tmp.interlaced == 1 )
		{
			if ( (tmp.vmode != HDMIIN_CEA_1080I50) && (tmp.vmode != HDMIIN_CEA_1080I60) )
				set_invert_top_bot(true);
			para.scan_mode = TVIN_SCAN_MODE_INTERLACED;
		}
		else
		{
			para.scan_mode = TVIN_SCAN_MODE_PROGRESSIVE;
		}

		para.hsync_phase = 1;
		para.vsync_phase = 0;
		//para.hs_bp = 0;
		//para.vs_bp = 2;
		para.cfmt = TVIN_RGB444;
		para.dfmt = TVIN_YUV422;
		para.reserved = 0; //skip_num

		printk("[%s] begin start_tvin_service() !\n",__FUNCTION__);

#ifdef HDMIIN_FRAME_SKIP_MECHANISM
		if( (SKIP_STATUS_NORMAL==flag_skip_status) ||
			(SKIP_STATUS_STANDBY==flag_skip_status) ||
			(SKIP_STATUS_CABLE==flag_skip_status) )
			flag_skip_enable = 1;
#endif
		start_tvin_service(SII9293_VDIN_INDEX,&para);
		info->vdin_started = 1;

	}

	return 0;
}

