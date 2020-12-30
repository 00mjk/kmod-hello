#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("5p1n6a11");
MODULE_DESCRIPTION("*** Hello world 5p1n6a11's kernel module ***");

static int mykernelmodule_init(void) {
	printk(KERN_ALERT "Hello World!!!!! part2\n");
	return 0;
}

static void mykernelmodule_exit(void) {
	printk(KERN_ALERT "good bye...\n");
}

module_init(mykernelmodule_init);
module_exit(mykernelmodule_exit);
