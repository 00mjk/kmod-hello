#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

MODULE_AUTHOR("u2i");
MODULE_LICENSE("GPL");

unsigned long (*kallsyms_lookup_name_addr)(const char *name);
static void **syscall_table;

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

static int
systemcall_hook_init(void)
{
    register_kprobe(&kp);
    printk("kp.addr: Found at %px \n", kp.addr);
    kallsyms_lookup_name_addr = (void *) kp.addr;
    printk("kallsyms_lookup_name: Found at %px \n", kallsyms_lookup_name_addr);
    unregister_kprobe(&kp);

    syscall_table = (void *)kallsyms_lookup_name_addr("sys_call_table");
    printk("sys_call_table: Found at %px \n", syscall_table);

    return 0;
}

static void
systemcall_hook_cleanup(void)
{
    pr_info("cleanup");
}

module_init(systemcall_hook_init);
module_exit(systemcall_hook_cleanup);
