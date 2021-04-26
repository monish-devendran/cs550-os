#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

//allocated memory for char buf.
char *buf;
static int sample_open(struct inode *inode, struct file *file)
{
    printk("I have been awoken\n");
    return 0;
}

static int sample_close(struct inode *inodep, struct file *filp)
{
    printk("Sleepy time\n");
    return 0;
}

static ssize_t sample_write(struct file *file, const char __user *buf,
		       size_t len, loff_t *ppos)
{
    printk("Yummy - I just ate %ld bytes\n", len);
    return len; /* But we don't actually do anything with the data */
}

static ssize_t sample_read(struct file *file, char __user * out, size_t size, loff_t * off)
{

sprintf(buf, "Hello World\n");
copy_to_user(out, buf, strlen(buf)+1);
if (!access_ok(VERIFY_READ, out, sizeof(out)))
   return -1;
return strlen(buf)+1;
}

static const struct file_operations sample_fops = {
    .owner			= THIS_MODULE,
    .write			= sample_write,
    .open			= sample_open,
    .release		= sample_close,
    .llseek 		= no_llseek,
    .read           = sample_read
};

struct miscdevice sample_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "sample_misc",
    .fops = &sample_fops,
};

static int __init misc_init(void)
{
    int error;

    error = misc_register(&sample_device);
    if (error) {
        pr_err("can't misc_register :(\n");
        return error;
    }

    printk("I'm in\n");
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&sample_device);
    printk("I'm out\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_DESCRIPTION("Simple Misc Driver");
MODULE_AUTHOR("Monish Devendran <mdevend1@binghamton.edu>");
MODULE_LICENSE("GPL");