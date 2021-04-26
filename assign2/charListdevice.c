#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h> 
#define  DEVICE_NAME "processList"    ///< The device will appear at /dev/processList using this value
#define  CLASS_NAME  "procList"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Prathamesh Walke");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("Kernel Modules and Character Device to list all processes");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users


struct task_struct *task;        /*    Structure defined in sched.h for tasks/processes    */
struct task_struct *task_child;        /*    Structure needed to iterate through task children    */
struct list_head *list;            /*    Structure needed to iterate through the list in each task->children struct    */

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[25600] = {0};           ///< Memory for the string that is passed from userspace
static struct class*  procListcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* procListcharDevice = NULL; ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
const char *stateIdParsing(long int stateid);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */

static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.release = dev_release,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */

static int __init procListchar_init(void){
	printk(KERN_INFO "ProcListChar: Initializing the ProcListChar LKM\n");

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "ProcListChar failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "ProcListChar: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	procListcharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(procListcharClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(procListcharClass);          // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "ProcListChar: device class registered correctly\n");

	// Register the device driver
	procListcharDevice = device_create(procListcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(procListcharDevice)){               // Clean up if there is an error
		class_destroy(procListcharClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(procListcharDevice);
	}
	printk(KERN_INFO "ProcListChar: device class created correctly\n"); // Made it! device was initialized
	return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit procListchar_exit(void){
	device_destroy(procListcharClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(procListcharClass);                          // unregister the device class
	class_destroy(procListcharClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "ProcListChar: Goodbye from the LKM!\n");
}


/** @brief This is a state id parser function and returns the state of the process as a string
 * This will take long int stateid as input and return string result
 *@param stateid is the long int value from task_struct
 */

const char * stateIdParsing(long int stateid){
	char * msg;

	if(stateid == TASK_RUNNING){
		msg = "TASK_RUNNING";
	}else if(stateid == TASK_INTERRUPTIBLE){
		msg = "TASK_INTERRUPTIBLE";
	}else if(stateid == TASK_UNINTERRUPTIBLE ){
		msg = "TASK_UNINTERRUPTIBLE";
	}else if(stateid == __TASK_STOPPED){
		msg = "__TASK_STOPPED";
	}else if(stateid == __TASK_TRACED){
		msg = "__TASK_TRACED";
	}else if(stateid == TASK_PARKED){
		msg = "TASK_PARKED";
	}else if(stateid == TASK_DEAD){
		msg = "TASK_DEAD";
	}else if(stateid == TASK_WAKEKILL){
		msg = "TASK_WAKEKILL";
	}else if(stateid == TASK_WAKING){
		msg = "TASK_WAKING";
	}else if(stateid == TASK_NOLOAD){
		msg = "TASK_NOLOAD";
	}else if(stateid == TASK_NEW){
		msg = "TASK_NEW";
	}else if(stateid == TASK_KILLABLE){
		msg = "TASK_WAKEKILL,TASK_UNINTERRUPTIBLE";
	}else if(stateid == TASK_STOPPED){
		msg = "TASK_WAKEKILL, __TASK_STOPPED";
	}else if(stateid == TASK_TRACED){
		msg = "TASK_WAKEKILL,__TASK_TRACED";
	}else if(stateid == TASK_IDLE){
		msg = "TASK_UNINTERRUPTIBLE,TASK_NOLOAD";
	}else if(stateid == TASK_NORMAL){
		msg = "TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE";
	}else if(stateid == TASK_REPORT){
		msg = "TASK_RUNNING ,TASK_INTERRUPTIBLE ,TASK_UNINTERRUPTIBLE ,__TASK_STOPPED ,__TASK_TRACED,EXIT_DEAD,EXIT_ZOMBIE,TASK_PARKED";
	}else if(stateid == TASK_STATE_MAX){
		msg = "TASK_STATE_MAX";
	}else{
		msg = "INVALID";
	}

	return msg;
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){

	/*numberOpens++;
	  printk(KERN_INFO "ProcListChar: Device has been opened %d time(s)\n", numberOpens);*/
	return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
	int error_count = 0;
	char * pointer = message;
	for_each_process( task ){ /*    for_each_process() MACRO for iterating through each task in the os located in linux\sched\signal.h    */
		pointer+=sprintf(pointer ,"\nPPID: %d PID: %d CPU: %u STATE: %s",task->real_parent->pid, task->pid, task->cpu, stateIdParsing(task->state));
		/*log parent id/executable name/state    */
		//list_for_each(list, &task->children){                        /*    list_for_each MACRO to iterate through task->children    */

		//	task_child = list_entry( list, struct task_struct, sibling );    /*    using list_entry to declare all vars in task_child struct    */
		//	pointer+=sprintf(pointer ,"\nPID: %d PARENT PID: %d CPU: %u STATE: %s",task_child->pid, task->pid, task_child->cpu ,stateIdParsing(task_child->state));/*    log parent id/executable name/state    */
		//}
	}

	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	error_count = copy_to_user(buffer, message, strlen(message));

	if (error_count==0){            // if true then have success
		printk(KERN_INFO "ProcListChar: Sent %ld characters to the user\n", strlen(message));
		return strlen(message);  // clear the position to the start and return 0
	}
	else {
		printk(KERN_INFO "ProcListChar: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to ani[200~
 *  inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "ProcListChar: Device successfully closed\n");
	return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(procListchar_init);
module_exit(procListchar_exit);