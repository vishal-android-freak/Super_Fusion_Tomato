/* Copyright (c) 2015, Varun Chitre <varun.chitre15@gmail.com>
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
 * A simple hotplugging driver optimized for Octa Core CPUs modified for Super Fusion
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/powersuspend.h>
#include <linux/cpufreq.h>

#define THUNDERPLUG "thunderplug"
#define MAX_FREQ_SCREENOFF (5330000)
#define MIN_FREQ_SCREENOFF (4000000)
#define MAX_FREQ_SCREENON (1267200)

static unsigned int max_freq_screenoff = MAX_FREQ_SCREENOFF;
static unsigned int min_freq_screenoff = MIN_FREQ_SCREENOFF;
static unsigned int max_freq_screenon = MAX_FREQ_SCREENON; 

struct cpufreq_cpuinfo cpu_frequency;

static inline void cpus_online_all(void)
{
	unsigned int cpu;

	for (cpu = 1; cpu < 8; cpu++) {
		if (cpu_is_offline(cpu))
			cpu_up(cpu);
	}

        for_each_possible_cpu(cpu) {
                cpu_frequency.max_freq = max_freq_screenon;
                cpu_frequency.min_freq = max_freq_screenon;
        }
        cpufreq_update_policy(cpu);

	pr_info("%s: all cpus were onlined\n", THUNDERPLUG);
}

static inline void cpus_offline_all(void)
{
	unsigned int cpu;

	for (cpu = 7; cpu > 0; cpu--) {
		if (cpu_online(cpu))
			cpu_down(cpu);
	}

        if (cpu == 8) {
                cpu_frequency.max_freq = max_freq_screenoff;
                cpu_frequency.min_freq = min_freq_screenoff;
        }
        cpufreq_update_policy(cpu);

	pr_info("%s: all cpus were offlined\n", THUNDERPLUG);
}

static void thunderplug_suspend(struct power_suspend *h)
{
	cpus_offline_all();

	pr_info("%s: suspend\n", THUNDERPLUG);
}

static void __ref thunderplug_resume(struct power_suspend *h)
{
	cpus_online_all();

	pr_info("%s: resume\n", THUNDERPLUG);
}

static struct power_suspend thunderplug_power_suspend_handler = 
	{
		.suspend = thunderplug_suspend,
		.resume = thunderplug_resume,
	};

static int __init thunderplug_init(void)
{
	int ret = 0;
	register_power_suspend(&thunderplug_power_suspend_handler);

	pr_info("%s: init\n", THUNDERPLUG);

	return ret;
}

static void __exit thunderplug_exit(void)
{
	unregister_power_suspend(&thunderplug_power_suspend_handler);
}

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Varun Chitre <varun.chitre15@gmail.com>");
MODULE_DESCRIPTION("Hotplug driver for OctaCore CPU");
late_initcall(thunderplug_init);
module_exit(thunderplug_exit);
