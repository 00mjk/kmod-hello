#include <linux/module.h>
#include <linux/kallsyms.h>
#include <asm/uaccess.h>
#include <linux/ratelimit.h>

MODULE_LICENSE("GPL");

asmlinkage int (*old_openat)(const struct pt_regs *);
static void **sys_call_table;

static asmlinkage long my_openat(const struct pt_regs *regs)
{
    char buffer[1024];
    if (current->tgid != 251) {
        printk("%s. proc:%s, pid:%d\n", __func__, current->group_leader->comm, current->tgid);
        strncpy_from_user(buffer, regs->si, sizeof buffer);
        printk("pathname: %s\n", buffer);
    }

    return old_openat(regs);
}

void disable_write_protect(void)
{
    unsigned long value;
    asm volatile("mov %%cr0,%0" : "=r" (value));
    if (value & 0x00010000)
    {
        value &= ~0x00010000;
        asm volatile("mov %0,%%cr0": : "r" (value));
    }
}

void enable_write_protect(void)
{
    unsigned long value;
    asm volatile("mov %%cr0,%0" : "=r" (value));
    if (!(value & 0x00010000))
    {
        value |= 0x00010000;
        asm volatile("mov %0,%%cr0": : "r" (value));
    }
}

static int __init test_init(void)
{
    sys_call_table = (void *)kallsyms_lookup_name("sys_call_table");
    old_openat = sys_call_table[__NR_openat];

    printk("[info] %s. old_openat:0x%p\n", __func__, old_openat);

    disable_write_protect();
    sys_call_table[__NR_openat] = my_openat;
    enable_write_protect();

    printk("%s inserted.\n",__func__);

    return 0;
}

static void __exit test_exit(void)
{
    disable_write_protect();
    sys_call_table[__NR_openat] = old_openat;
    enable_write_protect();

    printk("%s removed.\n",__func__);
}

module_init(test_init);
module_exit(test_exit);
