from bcc import BPF

bpf_text = """
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ima.h>
#include <linux/integrity.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <uapi/linux/hash_info.h>

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

int calc_hash(void)
{
    struct file *fp;
    char *filename ="/usr/lib64/libc.so.6";
    struct {
        struct ima_digest_data hdr;
        char digest[IMA_MAX_DIGEST_SIZE];
    } hash;

    memset(&hash.digest, 0, sizeof(hash.digest));

    fp = filp_open(filename, O_RDONLY, 0);

    hash.hdr.algo = HASH_ALGO_SHA256;

    ima_calc_file_hash_p = (void *) kallsyms_lookup_name("ima_calc_file_hash");
    ima_calc_file_hash_p(fp, &hash.hdr);

    return 0;
}

"""

b = BPF(text=bpf_text)
