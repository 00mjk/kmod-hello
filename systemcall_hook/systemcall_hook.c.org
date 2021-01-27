#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <asm/pgtable.h>
#include <linux/utsname.h>

MODULE_DESCRIPTION("system call replace test module");
MODULE_AUTHOR("u2i");
MODULE_LICENSE("GPL");

typedef unsigned long (*kallsyms_lookup_name_t)(const char *);
typedef void (*sys_call_ptr_t)(void);
typedef asmlinkage long (*orig_uname_t)(struct new_utsname *);

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

void
hexdump(unsigned char *addr, unsigned int length)
{
    unsigned int i;
    for (i = 0; i < length; i++) {
        if (!((i + 1) % 16)) {
            printk("%02x\n", *(addr + i));
        } else {
            if (!((i + 1) % 4)) {
                printk("%02x  ", *(addr + i));
            } else {
                printk("%02x ", *(addr + i));
            }
        }
    }

    if (!((length + 1) % 16)) {
        printk("\n");
    }
}

kallsyms_lookup_name_t kallsyms_lookup_name_p;
orig_uname_t orig_uname = NULL;
sys_call_ptr_t *_sys_call_table = NULL;

static void
save_original_syscall_address(void)
{
    orig_uname = (orig_uname_t) _sys_call_table[__NR_uname];
}

static void
change_page_attr_to_rw(pte_t *pte)
{
    set_pte_atomic(pte, pte_mkwrite(*pte));
}

static void
change_page_attr_to_ro(pte_t *pte)
{
    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

static void
replace_system_call(void *new)
{
    unsigned int level = 0;
    pte_t *pte;
    orig_uname_t uname_p_before;
    orig_uname_t uname_p_after;

    pte = lookup_address((unsigned long) _sys_call_table, &level);
    /* Need to set r/w to a page which syscall_tabel is in. */
    change_page_attr_to_rw(pte);
    printk("+ unprotected kernel memory page containing sys_call_table\n");
    uname_p_before = (orig_uname_t) _sys_call_table[__NR_uname];
    _sys_call_table[__NR_uname] = (sys_call_ptr_t) new;
    uname_p_after = (orig_uname_t) _sys_call_table[__NR_uname];
    pr_info("sys_call_table[__NR_uname]: %px -> %px\n", uname_p_before, uname_p_after);
    /* set back to read only */
    change_page_attr_to_ro(pte);
}

asmlinkage long
hooked_uname(struct new_utsname *name)
{
    orig_uname(name);

    pr_info("call original uname system call\n");
    pr_info("name->sysname: %s\n", name->sysname);
    
    return 0;
}

static int
syscall_replace_init(void)
{
    printk("+ Loading module\n");

    register_kprobe(&kp);
    printk("kp.addr: Found at %px \n", kp.addr);
    kallsyms_lookup_name_p = (void *) kp.addr;
    printk("kallsyms_lookup_name: Found at %px\n", kallsyms_lookup_name_p);
    unregister_kprobe(&kp);

    _sys_call_table = (void *)kallsyms_lookup_name_p("sys_call_table");
    pr_info("sys_call_table address is 0x%px\n", _sys_call_table);

    save_original_syscall_address();
    pr_info("original uname's address is %px\n", orig_uname);

    replace_system_call(hooked_uname);

    pr_info("system call replaced\n");
    return 0;
}

static void
syscall_replace_cleanup(void)
{
    pr_info("cleanup");

    if (orig_uname)
        replace_system_call(orig_uname);

}

module_init(syscall_replace_init);
module_exit(syscall_replace_cleanup);
