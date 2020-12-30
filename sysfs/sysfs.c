#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("Dual BSD/GPL");

static int value;

static ssize_t show_name(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "sysfs-module\n");
}

static ssize_t show_value(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", value);
}

static ssize_t store_value(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count)
{
    int res = kstrtoint(buf, 10, &value);
    if (res < 0) {
        return res;
    }
    return count;
}

static struct kobject *hello_kobj;

static struct kobj_attribute name_attribute =
    __ATTR(name, 0444, show_name, NULL);
static struct kobj_attribute value_attribute =
    __ATTR(name, 0644, show_value, store_value);

static struct attribute *attrs[] = {
    &name_attribute.attr,
    &value_attribute.attr,
    NULL
};
static struct attribute_group attribute_group = {
    .attrs = attrs
};

static int hellosysfs_init(void)
{
    int ret;

    printk(KERN_INFO "hello sysfs module\n");

    hello_kobj = kobject_create_and_add("fooattrs", &THIS_MODULE->mkobj.kobj);
    if (!hello_kobj) {
        return -ENOMEM;
    }

    ret = sysfs_create_group(hello_kobj, &attribute_group);
    if (ret) {
        /* hello_kobjの参照カウントを「減らす」 */
        kobject_put(hello_kobj);
    }

    return ret;
}

static void hellosysfs_exit(void)
{
    printk(KERN_INFO "exiting hello sysfs module...\n");

    /* hello_kobjの参照カウントを「減らす」 */
    kobject_put(hello_kobj);
}

module_init(hellosysfs_init);
module_exit(hellosysfs_exit);

