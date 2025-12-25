#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/wait.h>

#define DEVICE_NAME "prakhar"
#define CLASS_NAME  "prakhar_class"


#define SET_SIZE_OF_QUEUE _IOW('p', 1, int)
#define PUSH_DATA         _IOW('p', 2, struct data_packet)
#define POP_DATA          _IOR('p', 3, struct data_packet)

struct data_packet {
    int length;
    char __user *data;
};

struct queue_node {
    char *data;
    int length;
};

static int major_number;
static struct class*  prakhar_class  = NULL;
static struct device* prakhar_device = NULL;

static struct queue_node *queue = NULL;
static int queue_size = 0;
static int head = 0;
static int tail = 0;
static int count = 0;

static wait_queue_head_t read_queue;

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "prakhar: device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "prakhar: device closed\n");
    return 0;
}


static long prakhar_ioctl(struct file *filep,
                          unsigned int cmd,
                          unsigned long arg)
{
    switch (cmd) {

    case SET_SIZE_OF_QUEUE: {
        int size, i;

        if (copy_from_user(&size, (void __user *)arg, sizeof(int)))
            return -EFAULT;

        if (size <= 0)
            return -EINVAL;

        if (queue) {
            for (i = 0; i < queue_size; i++)
                kfree(queue[i].data);
            kfree(queue);
        }

        queue = kmalloc(sizeof(struct queue_node) * size, GFP_KERNEL);
        if (!queue)
            return -ENOMEM;

        for (i = 0; i < size; i++) {
            queue[i].data = NULL;
            queue[i].length = 0;
        }

        queue_size = size;
        head = 0;
        tail = 0;
        count = 0;

        printk(KERN_INFO "prakhar: queue initialized, size=%d\n", size);
        break;
    }

    case PUSH_DATA: {
        struct data_packet pkt;

        if (!queue)
            return -EINVAL;

        if (count == queue_size)
            return -ENOSPC;

        if (copy_from_user(&pkt, (void __user *)arg, sizeof(pkt)))
            return -EFAULT;

        queue[tail].data = kmalloc(pkt.length, GFP_KERNEL);
        if (!queue[tail].data)
            return -ENOMEM;

        if (copy_from_user(queue[tail].data, pkt.data, pkt.length))
            return -EFAULT;

        queue[tail].length = pkt.length;
        tail = (tail + 1) % queue_size;
        count++;

        wake_up_interruptible(&read_queue);

        printk(KERN_INFO "prakhar: PUSH_DATA, count=%d\n", count);
        break;
    }

    case POP_DATA: {
        struct data_packet pkt;

        if (!queue)
            return -EINVAL;

        wait_event_interruptible(read_queue, count > 0);

        if (copy_from_user(&pkt, (void __user *)arg, sizeof(pkt)))
            return -EFAULT;

        if (pkt.length < queue[head].length)
            return -EINVAL;

        if (copy_to_user(pkt.data, queue[head].data, queue[head].length))
            return -EFAULT;

        kfree(queue[head].data);
        queue[head].data = NULL;

        head = (head + 1) % queue_size;
        count--;

        printk(KERN_INFO "prakhar: POP_DATA, count=%d\n", count);
        break;
    }

    default:
        return -EINVAL;
    }

    return 0;
}


static struct file_operations fops =
{
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = prakhar_ioctl,
};


static int __init prakhar_init(void)
{
    printk(KERN_INFO "prakhar: initializing driver\n");

    init_waitqueue_head(&read_queue);

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
        return major_number;

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

static void __exit prakhar_exit(void)
{
    int i;

    if (queue) {
        for (i = 0; i < queue_size; i++)
            kfree(queue[i].data);
        kfree(queue);
    }

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
MODULE_DESCRIPTION("Dynamic circular queue char device with blocking IOCTL");
