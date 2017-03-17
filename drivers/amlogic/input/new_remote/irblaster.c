/*
 * IR BLASTER Driver
 *
 * Copyright (C) 2013 Amlogic Corporation
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * author :   platform-beijing
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/am_regs.h>
#include <linux/slab.h>
#include <linux/major.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <mach/pinmux.h>
#include <mach/gpio.h>
#include <linux/irq.h>
#include "irblaster.h"


#define DEVICE_NAME "meson-irblaster"
#define DEIVE_COUNT 32
#define BUFFER_SIZE 4096
static dev_t irblaster_id;
static struct class *irblaster_class;
static struct device *irblaster_dev;
static struct cdev irblaster_device;
static struct blaster_window rec_win, send_win;
static int rec_idx;
static unsigned int last_jiffies;
static char logbuf[4096];
static unsigned int temp_value,temp_value1;
static void aml_irblaster_tasklet(void);
static DEFINE_MUTEX(irblaster_file_mutex);
static void ir_hardware_release(void);
static void ir_hardware_init(void);
static int debug_enable = 0x00;

static int irblaster_dbg(const char* fmt, ...)
{
	va_list args;
	int r;

	if (!debug_enable)
		return 0;
	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);
	return r;
}

static int send_bit(unsigned int hightime, unsigned int lowtime)
{
	unsigned int count_delay;
	uint32_t val;

	//[11:10] = 2'b01,then set the timebase 10us.
	//[9:0] = 10'd,the timecount = N+1;
	count_delay = (((hightime*10-1)*38)/1000)&0x3ff;
	val = (0x10000 & ~(1<<12)) | (3<<10) | (count_delay<<0);
	aml_write_reg32( P_AO_IR_BLASTER_ADDR2, val);

	//timeleve = 1;
	//[11:10] = 2'b11,then set the timebase 26.5us.
	//[9:0] = 10'd,the timecount = N+1;
	count_delay = (lowtime-1) & 0x3ff;
	val = (0x10000 | (1<<12)) | (1<<10) | (count_delay<<0);
	aml_write_reg32( P_AO_IR_BLASTER_ADDR2, val);

	return 0;
}

static void send_all_frame(struct blaster_window * cw)
{
	int i,k;
	int exp = 0x00;
	unsigned int* pData;

	irblaster_dbg("cw->winNum = %d\n", cw->winNum);
	irblaster_dbg("cw->winArray = ");
	for ( i = 0; i < cw->winNum; i++ ) {
		irblaster_dbg("%d,", cw->winArray[i]);
		if ( i % 10 == 9 )
			irblaster_dbg("\n");
	}
	irblaster_dbg("\n");

	aml_set_reg32_mask(P_AO_RTI_GEN_CNTL_REG0, (1<<23));
	udelay(2);
	aml_clr_reg32_mask(P_AO_RTI_GEN_CNTL_REG0, (1<<23));
	/*set the modulator_tb = 2'10; 1us*/
	aml_set_reg32_mask(P_AO_IR_BLASTER_ADDR0, (2<<12));
	/*set mod_high_count = 13;*/
	aml_set_reg32_mask(P_AO_IR_BLASTER_ADDR1, (12<<16));
	/*set mod_low_count = 13;*/
	aml_set_reg32_mask(P_AO_IR_BLASTER_ADDR1, (12<<0));
	aml_set_reg32_mask(P_AO_IR_BLASTER_ADDR0, (1<<2));
//	udelay(1);
//	aml_clr_reg32_mask(P_AO_IR_BLASTER_ADDR0, (1<<2));
	aml_set_reg32_mask(P_AO_IR_BLASTER_ADDR0, (1<<0));

	k = cw->winNum;
	exp = cw->winNum / 10;
	pData = cw->winArray;

	while (exp) {
#define SEND_BIT_NUM 5
		for ( i = 0; i < SEND_BIT_NUM; i++ ) {
			send_bit( *pData, *(pData+1));
			irblaster_dbg("%d,%d\n", *pData, *(pData+1));
			pData += 2;
		}

		while (!(aml_read_reg32(P_AO_IR_BLASTER_ADDR0) & (1<<24))) ;
		while (aml_read_reg32(P_AO_IR_BLASTER_ADDR0) & (1<<26)) ;
#if 0
		aml_clr_reg32_mask(P_AO_IR_BLASTER_ADDR0, (1<<0));
		udelay(1);
		aml_set_reg32_mask(P_AO_IR_BLASTER_ADDR0, (1<<0));
#endif
		aml_set_reg32_mask(P_AO_RTI_GEN_CNTL_REG0, (1<<23));
		udelay(2);
		aml_clr_reg32_mask(P_AO_RTI_GEN_CNTL_REG0, (1<<23));
		exp--;
		irblaster_dbg("exp=%d\n",exp);
	}
	exp = (cw->winNum % 10) & (~(1));
	irblaster_dbg("exp2=%d\n",exp);
	for ( i = 0; i < exp; ) {
		send_bit(*pData, *(pData+1));
		pData += 2;
		i += 2;
	}
	irblaster_dbg("The all frame finished !!\n");
}

#if 0
static void send_nec_frame(struct blaster_window * creat_window){
	int i;
	// set init_high valid and enable the ir_blaster
	irblaster_dbg("Enable the ir_blaster, and create a NEC format !! \n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0)| (1<<2) | (1<<0));
	udelay(1);
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0)| (1<<2) | (1<<0));
	if(creat_window->winNum){
		irblaster_dbg("Create a 9ms low pulse\n");
		aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))    //timeleve = 0;
				| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
				| (899<<0)    //[9:0] = 10'd899,the timecount = N+1;
			       );
		irblaster_dbg("Create a 4.5ms high pulse\n");
		aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000  | (1<<12))     //timeleve = 1;
				| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
				| (449<<0)    //[9:0] = 10'd449,the timecount = N+1;
			       );
		udelay(1);
		for(i =0; i<creat_window->winNum;i++) {
			if (creat_window->winArray[i] != 1) {
				aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))    //timeleve = 0;
						| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
						| (55<<0)     //[9:0] = 10'd55,the timecount = N+1;
					       );

				aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 | (1<<12))     //timeleve = 1;
						| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
						| (55<<0)     //[9:0] = 10'd55,the timecount = N+1;
					       );
			}
			else {
				aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))    //timeleve = 0;
						| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
						| (55<<0)    //[9:0] = 10'd449,the timecount = N+1;
					       );

				aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 | (1<<12))     //timeleve = 1;
						| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
						| (167<<0)    //[9:0] = 10'd449,the timecount = N+1;
					       );
			}
		}
	}
	else{
		// if creat_window.winNum =  this is  repeat.
		irblaster_dbg("Create a 9ms low pulse\n");
		aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))    //timeleve = 0;
				| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
				| (899<<0)    //[9:0] = 10'd899,the timecount = N+1;
			       );
		irblaster_dbg("Create a 2.5ms high pulse\n");
		aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000  | (1<<12))     //timeleve = 1;
				| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
				| (249<<0)    //[9:0] = 10'd449,the timecount = N+1;
			       );
		udelay(1);
	}
	irblaster_dbg("The NEC frame finished !!\n");

}
static void send_sony_frame(struct blaster_window *creat_window){
	int i;
	irblaster_dbg("Enable the ir_blaster, and create a SONY format !! \n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0)| (1<<2) | (1<<0));
	udelay(1);
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0) & ~(1<<2) | (1<<0));
	// -----------------------------------------------
	// configure the parameter of modulator
	// -----------------------------------------------
	irblaster_dbg("Configure the parameter of modulator !! \n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0)| (1<<12));     //set the modulator_tb = 2'01;
	aml_write_reg32( P_AO_IR_BLASTER_ADDR1, aml_read_reg32(P_AO_IR_BLASTER_ADDR1)| (34<<16));    //set mod_high_count = 34;
	aml_write_reg32( P_AO_IR_BLASTER_ADDR1, aml_read_reg32(P_AO_IR_BLASTER_ADDR1)| (34<<0));     //set mod_low_count = 34;

	// -----------------------------------------------
	// write data into the fifo of IR_blaster
	// -----------------------------------------------
	irblaster_dbg("Create a 2.4ms pulse\n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))     //timeleve = 0;
			| (3<<10)       //[11:10] = 2'b01,then set the timebase 10us.
			| (273<<0)      //[9:0] = 10'd899,the timecount = N+1;
		       );

	for(i =0; i<creat_window->winNum;i++) {
		if (creat_window->winArray[i] != 1) {
			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))     //timeleve = 0;
					| (1<<10)       //[11:10] = 2'b01,then set the timebase 10us.
					| (59<<0)       //[9:0] = 10'd59,the timecount = N+1;
				       );

			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 | (1<<12))      //timeleve = 1;
					| (3<<10)       //[11:10] = 2'b01,then set the timebase 10us.
					| (68<<0)       //[9:0] = 10'd68,the timecount = N+1;
				       );
		}
		else {
			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))    //timeleve = 0;
					| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
					| (59<<0)    //[9:0] = 10'd59,the timecount = N+1;
				       );

			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 | (1<<12))     //timeleve = 1;
					| (3<<10)     //[11:10] = 2'b01,then set the timebase 10us.
					| (136<<0)    //[9:0] = 10'd449,the timecount = N+1;
				       );
		}
	}

	irblaster_dbg("The SONY frame finished !!\n");

}
static void send_duokan_frame(struct blaster_window *creat_window){
	int i;
	irblaster_dbg("Enable the ir_blaster, and create a DUOKAN format !! \n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0)| (1<<2) | (1<<0));
	udelay(1);
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0) & ~(1<<2) | (1<<0));
	// -----------------------------------------------
	// configure the parameter of modulator
	// -----------------------------------------------
	irblaster_dbg("Configure the parameter of modulator !! \n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR0, aml_read_reg32(P_AO_IR_BLASTER_ADDR0)| (1<<12));     //set the modulator_tb = 2'01;
	aml_write_reg32( P_AO_IR_BLASTER_ADDR1, aml_read_reg32(P_AO_IR_BLASTER_ADDR1)| (34<<16));    //set mod_high_count = 34;
	aml_write_reg32( P_AO_IR_BLASTER_ADDR1, aml_read_reg32(P_AO_IR_BLASTER_ADDR1)| (34<<0));     //set mod_low_count = 34;

	// -----------------------------------------------
	// write data into the fifo of IR_blaster
	// -----------------------------------------------
	irblaster_dbg("Create a 2.4ms pulse\n");
	aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))     //timeleve = 0;
			| (3<<10)       //[11:10] = 2'b01,then set the timebase 10us.
			| (273<<0)      //[9:0] = 10'd899,the timecount = N+1;
		       );

	for(i =0; i<creat_window->winNum;i++) {
		if (creat_window->winArray[i] != 1) {
			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))     //timeleve = 0;
					| (1<<10)       //[11:10] = 2'b01,then set the timebase 10us.
					| (59<<0)       //[9:0] = 10'd59,the timecount = N+1;
				       );

			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 | (1<<12))      //timeleve = 1;
					| (3<<10)       //[11:10] = 2'b01,then set the timebase 10us.
					| (68<<0)       //[9:0] = 10'd68,the timecount = N+1;
				       );
		}
		else {
			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 & ~(1<<12))    //timeleve = 0;
					| (1<<10)     //[11:10] = 2'b01,then set the timebase 10us.
					| (59<<0)    //[9:0] = 10'd59,the timecount = N+1;
				       );

			aml_write_reg32( P_AO_IR_BLASTER_ADDR2, (0x10000 | (1<<12))     //timeleve = 1;
					| (3<<10)     //[11:10] = 2'b01,then set the timebase 10us.
					| (136<<0)    //[9:0] = 10'd449,the timecount = N+1;
				       );
		}
	}

	irblaster_dbg("The SONY frame finished !!\n");

}

#endif

static ssize_t show_debug(struct device* dev,
	struct device_attribute* attr, char* buf)
{
	if (debug_enable)
		sprintf(buf, "debug=enable\n");
	else
		sprintf(buf, "debug=disable\n");
	return strlen(buf);
}

static ssize_t store_debug(struct device * dev,
	struct device_attribute *attr, const char * buf, size_t count)
{
	if (!strncmp(buf, "enable", 1)) {
		debug_enable = 1;
		printk("enable debug\n");
	}
	else if(!strncmp(buf, "disable", 1)) {
		debug_enable = 0;
		printk("disable debug\n");
	}
	return strlen(buf);;
}

static ssize_t show_key_value(struct device * dev,
	struct device_attribute *attr, char * buf)
{
	int i = 0;
	char tmp[64];
	memset(buf, 0, PAGE_SIZE);
	sprintf(tmp, "codelenth=\"%d\" code=\"", rec_win.winNum);
	strcat(buf, tmp);
	for ( i=0; i < rec_win.winNum; i++) {
		sprintf(tmp, "%d,", rec_win.winArray[i]);
		strcat(buf, tmp);
	}
	strcat(buf, "\n");
	return strlen(buf);
}

static ssize_t show_log(struct device * dev,
	struct device_attribute *attr, char * buf)
{
	memset(buf, 0, PAGE_SIZE);
	strcpy(buf, logbuf);
	return strlen(buf);
}

static ssize_t show_send_value(struct device * dev,
	struct device_attribute *attr, char * buf)
{
	int i = 0;
	char tmp[64];
	memset(buf, 0, PAGE_SIZE);
	sprintf(tmp, "codelenth=%d\n", send_win.winNum);
	strcat(buf, tmp);
	for ( i = 0; i < send_win.winNum; i++) {
		sprintf(tmp, "%d,", send_win.winArray[i]);
		strcat(buf, tmp);
	}
	strcat(buf, "\n");
	return strlen(buf);

}

static ssize_t store_key_code(struct device * dev,
	struct device_attribute *attr, const char * buf, size_t count)
{
	int i;
	static struct blaster_window win;
	static	int f[] = {
		1901, 4453, 625, 1614, 625, 1588, 625, 1614, 625, 442,
		625, 442, 625, 468, 625, 442, 625, 494, 572, 1614,
		625, 1588, 625, 1614, 625, 494, 572, 442, 651, 442,
		625, 442, 625, 442, 625, 1614, 625, 1588, 651, 1588,
		625, 442, 625, 494, 598, 442, 625, 442, 625, 520,
		572, 442, 625, 442, 625, 442, 651, 1588, 625, 1614,
		625, 1588, 625, 1614, 625, 1588, 625, 48958
	};

	win.winNum = sizeof(f)/sizeof(f[0]);
	for ( i = 0; i < win.winNum; i++)
		win.winArray[i] = f[i];
	send_all_frame(&win);
	return count;
}

static ssize_t store_hard_init(struct device* dev,
	struct device_attribute* attr, const char* buf, size_t count)

{
	printk("ir_blaster hardware init\n");
	ir_hardware_init();
	return strlen(buf);
}

static DEVICE_ATTR(debug, S_IWUSR | S_IRUGO, show_debug, store_debug);
static DEVICE_ATTR(keyvalue, S_IWUSR | S_IRUGO, show_key_value, NULL);
static DEVICE_ATTR(log, S_IWUSR | S_IRUGO, show_log, NULL);
static DEVICE_ATTR(sendvalue, S_IWUSR | S_IRUGO, show_send_value, NULL);
static DEVICE_ATTR(key_code, S_IWUSR | S_IRUGO, NULL, store_key_code);
static DEVICE_ATTR(hard_init,S_IWUSR | S_IRUGO, NULL,  store_hard_init);

static int aml_ir_irblaster_open(struct inode *inode, struct file *file)
{
	irblaster_dbg("aml_ir_irblaster_open()\n");
	irblaster_dbg("ir_blaster & ir rev start()\n");
	ir_hardware_init();
	return 0;
}

static long aml_ir_irblaster_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	s32 r = 0;
	unsigned long flags;
	void __user *argp = (void __user *)args;
	irblaster_dbg("aml_ir_irblaster_ioctl()\n");

	switch(cmd)
	{
		case IRRECEIVER_IOC_SEND:
			if (copy_from_user(&send_win, argp, sizeof(struct blaster_window)))
				return -EFAULT;
			local_irq_save(flags);
			send_all_frame(&send_win);
			local_irq_restore(flags);
			break;

		case IRRECEIVER_IOC_RECV:
			irblaster_dbg("recv win [%d]\n", rec_win.winNum);
			if(copy_to_user(argp, &rec_win, sizeof(struct blaster_window)))
				return -EFAULT;
			break;

		case IRRECEIVER_IOC_STUDY_S:
			irblaster_dbg("IRRECEIVER_IOC_STUDY_S\n");
			rec_win.winNum = 0;
			break;

		case IRRECEIVER_IOC_STUDY_E:
			irblaster_dbg("IRRECEIVER_IOC_STUDY_E\n");
			break;
		case GET_NUM_CARRIER:
			break;
		case SET_NUM_CARRIER:
			break;

		default:
			r = -ENOIOCTLCMD;
			break;
	}

	return r;
}

static int aml_ir_irblaster_release(struct inode *inode, struct file *file)
{
	irblaster_dbg("aml_ir_irblaster_release()\n");
	file->private_data = NULL;
	ir_hardware_release();
	return 0;

}

static const struct file_operations aml_ir_irblaster_fops = {
	.owner		= THIS_MODULE,
	.open		= aml_ir_irblaster_open,
	.unlocked_ioctl = aml_ir_irblaster_ioctl,
	.release	= aml_ir_irblaster_release,
};

static irqreturn_t irblaster_interrupt(int irq, void *dev_id){
	aml_irblaster_tasklet();
	return IRQ_HANDLED;
}

static void aml_irblaster_tasklet(void)
{
	int pulse_width;
	unsigned int current_jiffies = jiffies;

	if(current_jiffies - last_jiffies > 10)
	{
		rec_idx = 0;
		rec_win.winNum = 0;
		last_jiffies = current_jiffies;
		return;
	}

	last_jiffies = current_jiffies;

	pulse_width = ( (am_remote_read_reg(AM_IR_DEC_REG1)) & 0x1FFF0000 ) >> 16 ;

	rec_idx++;
	if(rec_idx >= MAX_PLUSE)
		return;

	rec_win.winNum = rec_idx;
	rec_win.winArray[rec_idx-1] = (pulse_width);
}

static void ir_hardware_init(void)
{
	unsigned int control_value;
	int ret;
	temp_value = am_remote_read_reg(AM_IR_DEC_REG0);
	temp_value1 = am_remote_read_reg(AM_IR_DEC_REG1);
	rec_idx = 0;
	last_jiffies = 0xffffffff;
	aml_set_reg32_mask(P_AO_RTI_PIN_MUX_REG, (1 << 31));
	aml_set_reg32_mask(AOBUS_REG_ADDR(AO_RTI_PIN_MUX_REG),1<<0);
	control_value = 3<<28|(0x9c40 << 12)|0x9; //base time = 10us
	am_remote_write_reg(AM_IR_DEC_REG0, control_value);
	control_value = 0x8574;
	am_remote_write_reg(AM_IR_DEC_REG1, control_value);
	ret = request_irq(INT_REMOTE, irblaster_interrupt, IRQF_SHARED, "irblaster", (void *)irblaster_interrupt);
	if (ret < 0) {
		printk(KERN_ERR "Irblaster: request_irq failed, ret=%d.\n", ret);
		return;
	}
}

static void ir_hardware_release(void)
{
	//unsigned int control_value;
	am_remote_write_reg(AM_IR_DEC_REG0, temp_value);
	am_remote_write_reg(AM_IR_DEC_REG1, temp_value1);
	free_irq(INT_REMOTE,irblaster_interrupt);
}

static int  aml_ir_irblaster_probe(struct platform_device *pdev)
{
	int r;
	printk("ir irblaster probe\n");
	r = alloc_chrdev_region(&irblaster_id, 0, DEIVE_COUNT, DEVICE_NAME);
	if (r < 0) {
		printk(KERN_ERR "Can't register major for ir irblaster device\n");
		return r;
	}
	irblaster_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(irblaster_class)) {
		unregister_chrdev_region(irblaster_id, DEIVE_COUNT);
		printk(KERN_ERR "Can't create class for ir irblaster device\n");
		return -1;
	}
	cdev_init(&irblaster_device, &aml_ir_irblaster_fops);
	irblaster_device.owner = THIS_MODULE;
	cdev_add(&(irblaster_device), irblaster_id, DEIVE_COUNT);

	irblaster_dev = device_create(irblaster_class, NULL,
		irblaster_id, NULL, "irblaster%d", 0);

	if (irblaster_dev == NULL) {
		printk("irblaster_dev create error\n");
		class_destroy(irblaster_class);
		return -EEXIST;
	}

	device_create_file(irblaster_dev, &dev_attr_debug);
	device_create_file(irblaster_dev, &dev_attr_keyvalue);
	device_create_file(irblaster_dev, &dev_attr_log);
	device_create_file(irblaster_dev, &dev_attr_sendvalue);
	device_create_file(irblaster_dev, &dev_attr_key_code);
	device_create_file(irblaster_dev, &dev_attr_hard_init);
	return 0;
}

static int aml_ir_irblaster_remove(struct platform_device *pdev)
{
	irblaster_dbg("remove IR Reccateiver\n");

	/* unregister everything */
	/* Remove the cdev */
	device_remove_file(irblaster_dev, &dev_attr_debug);
	device_remove_file(irblaster_dev, &dev_attr_keyvalue);
	device_remove_file(irblaster_dev, &dev_attr_log);
	device_remove_file(irblaster_dev, &dev_attr_sendvalue);
	device_remove_file(irblaster_dev, &dev_attr_key_code);
	device_remove_file(irblaster_dev, &dev_attr_hard_init);

	cdev_del(&irblaster_device);

	device_destroy(irblaster_class, irblaster_id);

	class_destroy(irblaster_class);
	aml_clr_reg32_mask(P_AO_RTI_PIN_MUX_REG, (1 << 31));
	unregister_chrdev_region(irblaster_id, DEIVE_COUNT);
	return 0;
}

static const struct of_device_id irblaster_dt_match[]={
	{	.compatible 	= "amlogic,aml_irblaster",
	},
	{},
};

static struct platform_driver aml_ir_irblaster_driver = {
	.probe		= aml_ir_irblaster_probe,
	.remove		= aml_ir_irblaster_remove,
	.suspend	= NULL,
	.resume		= NULL,
	.driver = {
		.name = "meson-irblaster",
		.owner  = THIS_MODULE,
		.of_match_table = irblaster_dt_match,
	},
};

static struct platform_device* aml_ir_irblaster_device = NULL;

static int __init aml_ir_irblaster_init(void)
{
	irblaster_dbg("IR Receiver Driver Init \n");
	aml_ir_irblaster_device = platform_device_alloc(DEVICE_NAME,-1);
	if (!aml_ir_irblaster_device) {
		irblaster_dbg("failed to alloc aml_ir_irblaster_device\n");
		return -ENOMEM;
	}
	if(platform_device_add(aml_ir_irblaster_device)){
		platform_device_put(aml_ir_irblaster_device);
		irblaster_dbg("failed to add aml_ir_irblaster_device\n");
		return -ENODEV;
	}
	if (platform_driver_register(&aml_ir_irblaster_driver)) {
		irblaster_dbg("failed to register aml_ir_irblaster_driver module\n");
		platform_device_del(aml_ir_irblaster_device);
		platform_device_put(aml_ir_irblaster_device);
		return -ENODEV;
	}

	return 0;
}

static void __exit aml_ir_irblaster_exit(void)
{
	irblaster_dbg("IR Receiver exit \n");
	platform_driver_unregister(&aml_ir_irblaster_driver);
	platform_device_unregister(aml_ir_irblaster_device);
	aml_ir_irblaster_device = NULL;
}

module_init(aml_ir_irblaster_init);
module_exit(aml_ir_irblaster_exit);

MODULE_AUTHOR("platform-beijing");
MODULE_DESCRIPTION("IR BLASTER Driver");
MODULE_LICENSE("GPL");


