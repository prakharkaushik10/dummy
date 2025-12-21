#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "prakhar"
#define CLASS_NAME  "prakhar_class"

static int major_number;
static struct class*  prakhar_class  = NULL;
static struct device* prakhar_device = NULL;

/* Called when device is opened */
static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "prakhar: device opened\n");
    return 0;
}

/* Called when device is closed */
static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "prakhar: device closed\n");
    return 0;
}

/* File operations structure */
static struct file_operations fops =
{
    .open = dev_open,
    .release = dev_release,
};

/* Module init */
static int __init prakhar_init(void)
{
    printk(KERN_INFO "prakhar: initializing driver\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "prakhar failed to register major number\n");
        return major_number;
    }

    prakhar_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(prakhar_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(prakhar_class);
    }

    prakhar_device = device_create(prakhar_class, NULL,
                        MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(prakhar_device)) {
        class_destroy(prakhar_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(prakhar_device);
    }

    printk(KERN_INFO "prakhar: device created /dev/%s\n", DEVICE_NAME);
    return 0;
}

/* Module exit */
static void __exit prakhar_exit(void)
{
    device_destroy(prakhar_class, MKDEV(major_number, 0));
    class_unregister(prakhar_class);
    class_destroy(prakhar_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "prakhar: driver removed\n");
}

module_init(prakhar_init);
module_exit(prakhar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prakhar");
MODULE_DESCRIPTION("Basic character device skeleton");
