#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/wait.h>

#define DEVICE_NAME "input_device"
#define BUFFER_SIZE 256

static int major_num;
static char *buffer;
static struct mutex input_mutex;

extern char *shared_buffer;

static int input_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "input: device opened\n");
    return 0;
}

static ssize_t input_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset){
    int bytes_read = 0;
    mutex_lock(&input_mutex);
    while(shared_buffer[0] == '\0'){
        mutex_unlock(&input_mutex);
        if(wait_event_interruptible(waitq, shared_buffer[0] != '\0')){
            return -ERESTARTSYS;
        }
        mutex_lock(&input_mutex);
    }
    if(copy_to_user(buffer, shared_buffer, len)){
        mutex_unlock(&input_mutex);
        return -EFAULT;
    }
    bytes_read = strlen(shared_buffer);
    memset(shared_buffer, '\0', BUFFER_SIZE);
    mutex_unlock(&input_mutex);
    return bytes_read;
}

static struct file_operations fops = {
    .open = input_open,
    .read = input_read,
};

static int __init input_init(void){
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if(major_num < 0){
        printk(KERN_ALERT "input: failed to register a major number\n");
        return major_num;
    }
    buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if(!buffer){
        printk(KERN_ALERT "input: failed to allocate memory\n");
        return -ENOMEM;
    }
    mutex_init(&input_mutex);
    printk(KERN_INFO "input: device created successfully\n");
    return 0;
}

static void __exit input_exit(void){
    mutex_destroy(&input_mutex);
    kfree(buffer);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "input: device exited successfully\n");
}

module_init(input_init);
module_exit(input_exit);
