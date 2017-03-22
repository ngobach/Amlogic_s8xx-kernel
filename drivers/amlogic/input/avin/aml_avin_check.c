#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/gpio.h>
#include <uapi/linux/input.h>
#include <linux/major.h>
#include <asm/uaccess.h>
#include <linux/of.h>
#include <linux/amlogic/aml_gpio_consumer.h>
#include <asm/uaccess.h>
#include "linux/amlogic/input/common.h"
#include "aml_avin_check.h"


#ifndef CONFIG_OF
#define CONFIG_OF
#endif

#define AVIN_NAME  "avin_detect"
#define DEBUG_DEF  0
#define ABS_AVIN 0

static irqreturn_t avin_detect_handler(int irq, void *data)
{
    struct aml_sysavin_dev *avdev=(struct aml_sysavin_dev *)data;
	avdev->irq_falling_times[avdev->detect_times] ++;
	disable_irq_nosync(INT_GPIO_0 + avdev->irq);
    return IRQ_HANDLED;
}

static enum hrtimer_restart avin_timer_sr(struct hrtimer *timer)
{
	struct aml_sysavin_dev *avdev = container_of(timer, struct aml_sysavin_dev, timer);

	if (avdev->first_time_into_loop == 0) {
		avdev->first_time_into_loop = 1;
		enable_irq(INT_GPIO_0 + avdev->irq);
	} else {
		if (++avdev->detect_times <= avdev->set_detect_times) {
			if (avdev->irq_falling_times[avdev->detect_times - 1] == 0) {
				disable_irq_nosync(INT_GPIO_0 + avdev->irq);
				//disable_irq(INT_GPIO_0 + avdev->irq);
			}

			if (avdev->detect_times != avdev->set_detect_times) {
				enable_irq(INT_GPIO_0 + avdev->irq);
			} else {
				avdev->detect_times = 0;
				avdev->first_time_into_loop = 0;
				schedule_work(&(avdev->work_update));
			}
		}
	}

	/* start timer: 100 ms */
	hrtimer_start(&avdev->timer,
		ktime_set(avdev->detect_interval_length / 1000, (avdev->detect_interval_length % 1000) * 1000000),
		HRTIMER_MODE_REL_PINNED);

	return HRTIMER_NORESTART;
}

static void kp_work(struct aml_sysavin_dev *avin_data)
{
	int i=0;
	int num=0;

	#if DEBUG_DEF
	pr_info("av-in low times = ");
	for (i=0; i<avin_data->set_detect_times; i++) {
		printk("%d ",avin_data->irq_falling_times[i]);
	}
	printk("\n");
	#endif

	for (i=0; i<avin_data->set_detect_times; i++) {
		if (avin_data->irq_falling_times[i] == 0) {
			num ++;
		}
		avin_data->irq_falling_times[i] = 0;
	}

	mutex_lock(&avin_data->lock);
	if (num >= (avin_data->set_detect_times - avin_data->set_fault_tolerance)) {
		avin_data->current_status = enum_status_out;
		input_report_abs(avin_data->input_dev, ABS_AVIN,enum_status_out);
		#if DEBUG_DEF
		pr_info("avin current_status out!\n");
		#endif
	} else if (num <= avin_data->set_fault_tolerance) {
		avin_data->current_status = enum_status_in;
		input_report_abs(avin_data->input_dev, ABS_AVIN,enum_status_in);
		#if DEBUG_DEF
		pr_info("avin current_status in!\n");
		#endif
	} else {
		//keep current status
	}
	mutex_unlock(&avin_data->lock);
}


static void update_work_func(struct work_struct *work)
{
    struct aml_sysavin_dev *avin_data = container_of(work, struct aml_sysavin_dev, work_update);

    kp_work(avin_data);
}


static int avin_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct aml_sysavin_dev *avindev;

	avindev = container_of(inode->i_cdev, struct aml_sysavin_dev, avin_cdev);
	file->private_data = avindev;

	return ret;
}

static ssize_t avin_read(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
	unsigned long ret;
	struct aml_sysavin_dev *avin_data = (struct aml_sysavin_dev *)file->private_data;

	mutex_lock(&avin_data->lock);
	ret = copy_to_user(buf,(void *)&(avin_data->current_status),sizeof(enum aml_avin_status_enum));
	mutex_unlock(&avin_data->lock);
	return 0;
}

static int avin_config_release(struct inode *inode, struct file *file)
{
    file->private_data=NULL;
    return 0;
}

static const struct file_operations avin_fops = {
    .owner      = THIS_MODULE,
	.open    	= avin_open,
    .read       = avin_read,
    .release    = avin_config_release,
};

static int register_avin_dev(struct aml_sysavin_dev *avin_data)
{
    int ret = 0;

	ret = alloc_chrdev_region(&avin_data->avin_devno, 0, 1, "avin_detect_region");
	if (ret < 0) {
		pr_err("avin: failed to allocate major number\n");
		return -ENODEV;
	}

	/* connect the file operations with cdev */
	cdev_init(&avin_data->avin_cdev, &avin_fops);
	avin_data->avin_cdev.owner = THIS_MODULE;
	/* connect the major/minor number to the cdev */
	ret = cdev_add(&avin_data->avin_cdev, avin_data->avin_devno, 1);
	if (ret) {
		pr_err("avin: failed to add device\n");
		return -ENODEV;
	}

    strcpy(avin_data->config_name, "avin_detect");
    avin_data->config_class=class_create(THIS_MODULE,avin_data->config_name);
    avin_data->config_dev=device_create(avin_data->config_class, NULL,
		avin_data->avin_devno, NULL, avin_data->config_name);
	if (IS_ERR(avin_data->config_dev)) {
		 pr_err("avin: failed to create device node\n");
		 ret = PTR_ERR(avin_data->config_dev);
		 return ret;
	 }

    return ret;
}


static int aml_sysavin_dt_parse(struct platform_device *pdev)
{
	int ret=0;
	const char *str;
	struct aml_sysavin_dev *avdev;

	avdev = platform_get_drvdata(pdev);
	ret = of_property_read_string(pdev->dev.of_node, "avin_pin", &str);
	if (ret) {
		pr_err("avin:faild to get avin_pin\n");
		return -ENODEV;
	}
	avdev->pin = amlogic_gpio_name_map_num(str);

	ret = of_property_read_u32(pdev->dev.of_node,"avin_irq",&(avdev->irq));
    if (ret) {
        pr_err("Failed to get irq number from dts.\n");
        goto get_avin_param_failed;
	}

	ret = of_property_read_u32(pdev->dev.of_node,"detect_time_length",&(avdev->detect_time_length));
    if (ret) {
        pr_err("Failed to get detect_time_lengthr from dts.\n");
        goto get_avin_param_failed;
	}

	ret = of_property_read_u32(pdev->dev.of_node,"detect_interval_length",&(avdev->detect_interval_length));
    if (ret) {
        pr_err("Failed to get detect_interval_length from dts.\n");
        goto get_avin_param_failed;
	}

	ret = of_property_read_u32(pdev->dev.of_node,"set_detect_times",&(avdev->set_detect_times));
    if (ret) {
        pr_err("Failed to get detect_interval_length from dts.\n");
        goto get_avin_param_failed;
	}

	ret = of_property_read_u32(pdev->dev.of_node,"set_fault_tolerance",&(avdev->set_fault_tolerance));
    if (ret) {
        pr_err("Failed to get detect_interval_length from dts.\n");
        goto get_avin_param_failed;
	}

	return 0;

	get_avin_param_failed:
		return -EINVAL;
}


static int avin_check_probe(struct platform_device *pdev)
{
	int ret;
	int err_irq,irq_ret;
	int state=-EINVAL;
	struct aml_sysavin_dev *avdev;

	avdev = kzalloc(sizeof(struct aml_sysavin_dev), GFP_KERNEL);
	if (!avdev) {
		pr_err("kzalloc error\n");
		state = -ENOMEM;
		goto get_param_mem_fail;
	}
	platform_set_drvdata(pdev, avdev);

	ret = aml_sysavin_dt_parse(pdev);
	if (ret) {
		state = ret;
		goto get_dts_dat_fail;
	}

	/* init */
	mutex_init(&avdev->lock);
	avdev->irq_falling_times = kzalloc((sizeof(unsigned int) * avdev->set_detect_times), GFP_KERNEL);
	if (!avdev->irq_falling_times) {
		state = -ENOMEM;
		goto get_param_mem_fail_1;
	}

	/* set gpio */
	err_irq  = amlogic_gpio_request(avdev->pin,AVIN_NAME);
	if (err_irq) {
		pr_err("AVIN: faild to alloc gpio_interrupt!\n");
		state = ERR_GPIO_REQ;
		goto gpio_request_fail;
	}
	amlogic_gpio_direction_input(avdev->pin,AVIN_NAME);
	amlogic_disable_pullup(avdev->pin,AVIN_NAME);
	amlogic_gpio_to_irq(avdev->pin, AVIN_NAME,
                    AML_GPIO_IRQ(avdev->irq, FILTER_NUM7,GPIO_IRQ_FALLING));
    irq_ret = request_irq(INT_GPIO_0 + avdev->irq,avin_detect_handler,IRQF_DISABLED,AVIN_NAME,(void *)avdev);
	if (irq_ret) {
		pr_err("avin_detect_handler request irq failed, ret:%d\n", irq_ret);
		goto irq_request_fail;;
	}
	disable_irq(INT_GPIO_0 + avdev->irq);

	/* set timer */
	hrtimer_init(&avdev->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
	avdev->timer.function = avin_timer_sr;
	hrtimer_start(&avdev->timer,
		ktime_set(avdev->detect_interval_length / 1000, (avdev->detect_interval_length % 1000) * 1000000),
		HRTIMER_MODE_REL_PINNED);

	INIT_WORK(&(avdev->work_update), update_work_func);

	/* register input device */
	avdev->input_dev = input_allocate_device();
	if (!avdev->input_dev) {
        state = -ENOMEM;
        goto allocate_input_fail;
    }

	set_bit(EV_ABS, avdev->input_dev->evbit);
	avdev->input_dev->name = AVIN_NAME;
    //avdev->input_dev->phys = "gpio_keypad/input0";
    avdev->input_dev->dev.parent = &pdev->dev;

    avdev->input_dev->id.bustype = BUS_ISA;
    avdev->input_dev->id.vendor = 0x5f5f;
    avdev->input_dev->id.product = 0x6f6f;
    avdev->input_dev->id.version = 0x7f7f;

	ret = input_register_device(avdev->input_dev);
    if (ret < 0) {
        pr_err("Unable to register avin input device.\n");
		    state = -EINVAL;
		    goto register_input_fail;
    }

	/* register char device */
	ret = register_avin_dev(avdev);

	return 0;

	register_input_fail:
		input_free_device(avdev->input_dev);
	allocate_input_fail:
	irq_request_fail:
		gpio_free(avdev->pin);
	gpio_request_fail:
		kfree(avdev->irq_falling_times);
	get_param_mem_fail_1:
	get_dts_dat_fail:
		kfree(avdev);
	get_param_mem_fail:
		return state;

}

static int avin_check_remove(struct platform_device *pdev)
{
	struct aml_sysavin_dev *avdev = platform_get_drvdata(pdev);
	hrtimer_cancel(&avdev->timer);
	input_unregister_device(avdev->input_dev);
	input_free_device(avdev->input_dev);
	cdev_del(&avdev->avin_cdev);
    unregister_chrdev_region(avdev->avin_devno, 1);
	if (avdev->config_class)
	{
		if (avdev->config_dev)
		device_destroy(avdev->config_class,avdev->avin_devno);
		class_destroy(avdev->config_class);
	}
	cancel_work_sync(&avdev->work_update);
	gpio_free(avdev->pin);
	platform_set_drvdata(pdev, NULL);
	kfree(avdev->irq_falling_times);
	kfree(avdev);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id avin_dt_match[]={
	{	.compatible = "amlogic,avin_detect",
	},
	{},
};
#else
#define avin_dt_match NULL
#endif

static struct platform_driver avin_driver = {
    .probe      = avin_check_probe,
    .remove     = avin_check_remove,
    .driver     = {
        .name   = "avin_check",
        .of_match_table = avin_dt_match,
    },
};



static int __init avin_check_init(void)
{
    pr_info("AVIN check init.\n");
    return platform_driver_register(&avin_driver);
}

static void __exit avin_check_exit(void)
{
    pr_info("AVIN check exit.\n");
    platform_driver_unregister(&avin_driver);
}

module_init(avin_check_init);
module_exit(avin_check_exit);

MODULE_DESCRIPTION("Meson AVIN Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amlogic, Inc.");
