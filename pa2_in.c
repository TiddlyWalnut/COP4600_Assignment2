
/**
 * File:	pa2_in.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 * By: Alan Rugeles, Dillon Garcia, John Ashley
 */

#include <linux/init.h>       // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#include <linux/mutex.h>	  // Required for the mutex functionality

#define DEVICE_NAME "pa2_in" // Device name.
#define CLASS_NAME "char"	  ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("John Aedo");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("pa2_in Kernel Module"); ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *pa2_inClass = NULL;	///< The device-driver class struct pointer
static struct device *pa2_inDevice = NULL; ///< The device-driver device struct pointer

char   message[1025] = {0};           ///< Memory for the string that is passed from userspace
EXPORT_SYMBOL(message);               //< Makes it visible to the kernel and other modules
short  size_of_message = 0;           ///< Used to remember the size of the string stored
EXPORT_SYMBOL(size_of_message);       //< Makes it visible to the kernel and other modules
static int    numberOpens = 0;        ///< Counts the number of times the device is opened

DEFINE_MUTEX(ebbchar_mutex);	    ///< Macro to declare a new mutex
EXPORT_SYMBOL(ebbchar_mutex);		//< Makes it visible to the kernel and other modules



/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
// static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		// .read = read,
		.write = write,
};

/**
 * Initializes module at installation
 */
int init_mod(void)
{
	printk(KERN_INFO "pa2_in: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "pa2_in could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "pa2_in: registered correctly with major number %d\n", major_number);

	// Register the device class
	pa2_inClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(pa2_inClass))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(pa2_inClass); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "pa2_in: device class registered correctly\n");

	// Register the device driver
	pa2_inDevice = device_create(pa2_inClass, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(pa2_inDevice))
	{								 // Clean up if there is an error
		class_destroy(pa2_inClass); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(pa2_inDevice);
	}
	printk(KERN_INFO "pa2_in: device class created correctly\n"); // Made it! device was initialized
	mutex_init(&ebbchar_mutex);    
	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void exit_module(void)
{
	printk(KERN_INFO "pa2_in: removing module.\n");
	mutex_destroy(&ebbchar_mutex);                        // destroy the dynamically-allocated mutex
	device_destroy(pa2_inClass, MKDEV(major_number, 0)); // remove the device
	class_unregister(pa2_inClass);						  // unregister the device class
	class_destroy(pa2_inClass);						     // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "pa2_in: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	if(!mutex_trylock(&ebbchar_mutex)){                  // Try to acquire the mutex (returns 0 on fail)
		printk(KERN_ALERT "EBBChar: Device in use by another process");
		return -EBUSY;
   	}
   	
	numberOpens++;
   	printk(KERN_INFO "pa2_in: device opened.\n");
   	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&ebbchar_mutex);    
	printk(KERN_INFO "pa2_in: device closed.\n");
	return 0;
}

/*
 * Reads from device, displays in userspace, and deletes the read data
 * Copies from message to buffer
 * @param filep A pointer to a file object (defined in linux/fs.h)
 * @param buffer The pointer to the buffer to which this function writes the data
 * @param len The length of the buffer
 * @param offset The offset if required
 */
// static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
// {
// 	int error_count = 0;
// 	int i, j;
	
// 	//if read operation request is more than size of message
// 	//only service up to the amount available
// 	if(len > size_of_message) {
// 		len = size_of_message;
// 	}

//    	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
//    	error_count = copy_to_user(buffer, message, len);

// 	//if true then have success
//    	if (error_count == 0) {           
//       		printk(KERN_INFO "Sent %zu characters to the user: %s\n", len, buffer);
			
// 			//delete read data 
// 			for(i = 0, j = len; j < size_of_message; i++, j++) {
// 				message[i] = message[j];
// 			}
// 			size_of_message = size_of_message - len;  //update size of message
// 			message[size_of_message] = '\0';		  //add null terminator to mark end of string

//       		return 0;  								 //Success -- return 0 
//    	}
//    	else {
//       		printk(KERN_INFO "Failed to send %d characters to the user\n", error_count);
//       		return -EFAULT;              //Failed -- return a bad address message (i.e. -14)
//    	}
// }

/*
 * Writes to the device
 * Appends buffer to message
 * @param filep A pointer to a file object
 * @param buffer The buffer to that contains the string to write to the device
 * @param len The length of the array of data that is being passed in the const char buffer
 * @param offset The offset if required
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	mutex_lock(&ebbchar_mutex);
	//check if there is enough space left in message (including null terminator)
	int maxSize = (int)(sizeof(message)) - size_of_message - 1;
	if (len > maxSize)
	{
		len = maxSize;
	}
	// sprintf(message, "%s(%zu letters)", buffer, len);                  	 
	strncat(message, buffer, len);					//appends received string with length to message (with null terminator)
	size_of_message = strlen(message);				//update the size of message
   	printk(KERN_INFO "Received %zu characters from the user: %s\n", len, buffer);
   	mutex_unlock(&ebbchar_mutex);
   	return len;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(init_mod);
module_exit(exit_module);

