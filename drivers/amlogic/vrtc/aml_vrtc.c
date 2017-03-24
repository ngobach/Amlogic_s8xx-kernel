/*
 * drivers/amlogic/rtc/aml_vrtc.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
*/


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/pm_wakeup.h>
#include <linux/jiffies.h>
#include <mach/am_regs.h>

static void __iomem *alarm_reg_vaddr;

static int aml_vrtc_read_time(struct device *dev, struct rtc_time *tm)
{
	u64 cur_seconds;
	u32 time_t;
	cur_seconds = jiffies_64_to_clock_t(get_jiffies_64());
	time_t = (u32)(cur_seconds + 305);

	rtc_time_to_tm(time_t, tm);

	return 0;
}

static int set_wakeup_time(unsigned long time)
{
	int ret = -1;
	if (alarm_reg_vaddr) {
		writel(time, alarm_reg_vaddr);
		ret = 0;
		pr_info("set_wakeup_time: %lu\n", time);
	}
	return ret;
}

static int aml_vrtc_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	unsigned long alarm_secs, cur_secs;
	int ret;
	struct	rtc_device *vrtc;

	struct rtc_time cur_rtc_time;

	vrtc = dev_get_drvdata(dev);

	if (alarm->enabled) {
		ret = rtc_tm_to_time(&alarm->time, &alarm_secs);
		if (ret)
			return ret;
		aml_vrtc_read_time(dev, &cur_rtc_time);
		ret = rtc_tm_to_time(&cur_rtc_time, &cur_secs);
		if (alarm_secs >= cur_secs) {
			alarm_secs = alarm_secs - cur_secs;
			ret = set_wakeup_time(alarm_secs);
			if (ret < 0)
				return ret;
		}
	}
	return 0;
}

static const struct rtc_class_ops aml_vrtc_ops = {
	.read_time = aml_vrtc_read_time,
	.set_alarm = aml_vrtc_set_alarm,
};

static int aml_vrtc_probe(struct platform_device *pdev)
{
	struct	rtc_device *vrtc;
	int ret;
	u32 paddr = 0;

	ret = of_property_read_u32(pdev->dev.of_node,
			"alarm_reg_addr", &paddr);
	if (!ret) {
		pr_debug("alarm_reg_paddr: 0x%x\n", paddr);
		alarm_reg_vaddr = ioremap(paddr, 0x4);
	}

	device_init_wakeup(&pdev->dev, 1);
	vrtc = rtc_device_register("aml_vrtc", &pdev->dev,
		&aml_vrtc_ops, THIS_MODULE);
	if (!vrtc)
		return -1;
	platform_set_drvdata(pdev, vrtc);

	return 0;
}

static int aml_vrtc_remove(struct platform_device *dev)
{
	struct rtc_device *vrtc = platform_get_drvdata(dev);
	rtc_device_unregister(vrtc);

	return 0;
}

static const struct of_device_id aml_vrtc_dt_match[] = {
	{ .compatible = "amlogic, aml_vrtc"},
	{},
};

struct platform_driver aml_vrtc_driver = {
	.driver = {
		.name = "aml_vrtc",
		.owner = THIS_MODULE,
		.of_match_table = aml_vrtc_dt_match,
	},
	.probe = aml_vrtc_probe,
	.remove = aml_vrtc_remove,
};

static int  __init aml_vrtc_init(void)
{
	return platform_driver_register(&aml_vrtc_driver);
}

static void __init aml_vrtc_exit(void)
{
	return platform_driver_unregister(&aml_vrtc_driver);
}

module_init(aml_vrtc_init);
module_exit(aml_vrtc_exit);

MODULE_DESCRIPTION("Amlogic internal vrtc driver");
MODULE_LICENSE("GPL");
