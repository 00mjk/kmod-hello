#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ima.h>
#include <linux/integrity.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <uapi/linux/hash_info.h>

MODULE_AUTHOR("u2i");
MODULE_LICENSE("GPL v2");

struct ima_digest_data {
    u8 algo;
    u8 length;
    union {
            struct {
                    u8 unused;
                    u8 type;
            } sha1;
            struct {
                    u8 type;
                    u8 algo;
            } ng;
            u8 data[2];
    } xattr;
    u8 digest[0];
} __packed;

int (*ima_calc_file_hash_p)(struct file *file, struct ima_digest_data *hash);

#define IMA_MAX_DIGEST_SIZE     64

static void calc_hash(void)
{
    struct file *fp;
    char *filename = "/usr/lib64/libc.so.6";
    // struct ima_digest_data hash;
    // unsigned long ima_calc_file_hash_addr;
    struct {
        struct ima_digest_data hdr;
        char digest[IMA_MAX_DIGEST_SIZE];
    } hash;

    memset(&hash.digest, 0, sizeof(hash.digest));

    fp = filp_open(filename, O_RDONLY, 0);

//  ima_calc_file_hash_addr = kallsyms_lookup_name("ima_calc_file_hash");

//    ima_calc_file_hash_p = (int (*)(struct file *, struct ima_digest_data *))ima_calc_file_hash_addr;

    hash.hdr.algo = HASH_ALGO_SHA256;

    ima_calc_file_hash_p = (void*)kallsyms_lookup_name("ima_calc_file_hash");
    ima_calc_file_hash_p(fp, &hash.hdr);

    printk(KERN_INFO "[ito-h] filename : %s\n", filename);
    // printk(KERN_INFO "addr : %lx\n", ima_calc_file_hash_addr);
    // printk(KERN_INFO "addr2 : %lx\n", ima_calc_file_hash_p);
    printk(KERN_INFO "[ito-h] digest : %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", 
        hash.hdr.digest[0]  & 0xff, hash.hdr.digest[1]  & 0xff, hash.hdr.digest[2]  & 0xff, hash.hdr.digest[3]  & 0xff, 
        hash.hdr.digest[4]  & 0xff, hash.hdr.digest[5]  & 0xff, hash.hdr.digest[6]  & 0xff, hash.hdr.digest[7]  & 0xff,
        hash.hdr.digest[8]  & 0xff, hash.hdr.digest[9]  & 0xff, hash.hdr.digest[10] & 0xff, hash.hdr.digest[11] & 0xff, 
        hash.hdr.digest[12] & 0xff, hash.hdr.digest[13] & 0xff, hash.hdr.digest[14] & 0xff, hash.hdr.digest[15] & 0xff,
        hash.hdr.digest[16] & 0xff, hash.hdr.digest[17] & 0xff, hash.hdr.digest[18] & 0xff, hash.hdr.digest[19] & 0xff,
        hash.hdr.digest[20] & 0xff, hash.hdr.digest[21] & 0xff, hash.hdr.digest[22] & 0xff, hash.hdr.digest[23] & 0xff,
        hash.hdr.digest[24] & 0xff, hash.hdr.digest[25] & 0xff, hash.hdr.digest[26] & 0xff, hash.hdr.digest[27] & 0xff, 
        hash.hdr.digest[28] & 0xff, hash.hdr.digest[29] & 0xff, hash.hdr.digest[30] & 0xff, hash.hdr.digest[31] & 0xff);

}

static int __init test_init(void)
{
    printk(KERN_INFO "ima_test_init\n");
    calc_hash();
    return 0;
}

static void __exit test_exit(void)
{
    printk(KERN_INFO "ima_test_exit\n");
}

module_init(test_init);
module_exit(test_exit);

