#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

int init_module(void)
{
    register_kprobe(&kp);
    printk("Found at %px \n", kp.addr);
    return 0;
}

void cleanup_module(void)
{
    unregister_kprobe(&kp);
}

MODULE_AUTHOR("u2i");
MODULE_LICENSE("GPL");
