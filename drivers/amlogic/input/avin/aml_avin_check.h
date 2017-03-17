#ifndef __AML_AVIN_CHECK_H
#define __AML_AVIN_CHECK_H

#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>

enum aml_avin_status_enum
{
	enum_status_in=0,
	enum_status_out=1,
};

struct aml_sysavin_dev
{
	char config_name[20];
	enum aml_avin_status_enum current_status;
	int irq;
	int timer_flag;
	unsigned int detect_time_length;
	unsigned int detect_interval_length;
	unsigned int pin;
	unsigned int set_detect_times;
	unsigned int set_fault_tolerance;
	unsigned int detect_times;
	unsigned int first_time_into_loop;
	unsigned int *irq_falling_times;
	dev_t  avin_devno;
	struct cdev avin_cdev;
	struct input_dev *input_dev;
	struct device *config_dev;
	struct class *config_class;
	struct hrtimer timer;
	struct mutex lock;
	struct work_struct work_update;
};

#endif