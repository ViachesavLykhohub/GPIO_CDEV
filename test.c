// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

MODULE_AUTHOR("Viaheslav Lykhohub <viacheslav.lykhohub@globallogic.com>");
MODULE_DESCRIPTION("...");
MODULE_LICENSE("GPL");


static int __init module_init(void)
{
	return 0;
}

static void __exit module_exit(void)
{
	
}

module_init(module_init);
module_exit(module_exit);

