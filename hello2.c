#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("u2ih1r05h1");
MODULE_DESCRIPTION("*** Hello world u2i's kernel module ***");

static int mykernelmodule_init(void) {
	printk(KERN_ALERT "Hello World!!!!! part2\n");
	return 0;
}

static void mykernelmodule_exit(void) {
	printk(KERN_ALERT "good bye...\n");
}

module_init(mykernelmodule_init);
module_exit(mykernelmodule_exit);
