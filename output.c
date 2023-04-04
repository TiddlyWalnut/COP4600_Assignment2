// output_module.c

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define BUFFER_SIZE 1024

extern char *shared_buffer;
extern struct mutex buffer_mutex;

static int output_device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Output device opened\n");
    return 0;
}

static int output_device_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Output device closed\n");
    return 0;
}

static ssize_t output_device_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    int bytes_to_write;
    int bytes_written = 0;

    if(mutex_lock_interruptible(&buffer_mutex)) {
        return -ERESTARTSYS;
    }

    while (len > 0) {
        bytes_to_write = min_t(int, len, BUFFER_SIZE - bytes_written);

        if (copy_from_user(&shared_buffer[bytes_written], buf, bytes_to_write)) {
            mutex_unlock(&buffer_mutex);
            return -EFAULT;
        }

        bytes_written += bytes_to_write;
        len -= bytes_to_write;
    }

    mutex_unlock(&buffer_mutex);
    printk(KERN_INFO "Output device wrote %d bytes\n", bytes_written);
    return bytes_written;
}

static struct file_operations output_fops = {
    .owner = THIS_MODULE,
    .open = output_device_open,
    .release = output_device_close,
    .write = output_device_write,
};

static int __init output_module_init(void) {
    printk(KERN_INFO "Initializing output module\n");
    return register_chrdev(0, "output_device", &output_fops);
}

static void __exit output_module_exit(void) {
    unregister_chrdev(0, "output_device");
    printk(KERN_INFO "Output module exited\n");
}

module_init(output_module_init);
module_exit(output_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Output module for shared memory device driver");
