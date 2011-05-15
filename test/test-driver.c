
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("My kernel module");
MODULE_AUTHOR("Steven Joseph (stevenjose@gmail.com)");
MODULE_LICENSE("$LICENSE$");

static int test_init_module(void)
{
	printk( KERN_DEBUG "Module test init\n" );
	return 0;
}

static void test_exit_module(void)
{
	printk( KERN_DEBUG "Module test exit\n" );
}

module_init(test_init_module);
module_exit(test_exit_module);
