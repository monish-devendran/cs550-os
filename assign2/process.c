#include <linux/miscdevice.h>
#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h> 


static char out[25600] = {0}; 

struct task_struct *task;        /*    Structure defined in sched.h for tasks/processes    */
struct task_struct *task_child;        /*    Structure needed to iterate through task children    */
char * buffer;

const char * task_status(long int stateid){
	char * status;

	if(stateid == TASK_RUNNING){
		status = "TASK_RUNNING";
	}else if(stateid == TASK_INTERRUPTIBLE){
		status = "TASK_INTERRUPTIBLE";
	}else if(stateid == TASK_UNINTERRUPTIBLE ){
		status = "TASK_UNINTERRUPTIBLE";
	}else if(stateid == __TASK_STOPPED){
		status = "__TASK_STOPPED";
	}else if(stateid == __TASK_TRACED){
		status = "__TASK_TRACED";
	}else if(stateid == TASK_PARKED){
		status = "TASK_PARKED";
	}else if(stateid == TASK_DEAD){
		status = "TASK_DEAD";
	}else if(stateid == TASK_WAKEKILL){
		status = "TASK_WAKEKILL";
	}else if(stateid == TASK_WAKING){
		status = "TASK_WAKING";
	}else if(stateid == TASK_NOLOAD){
		status = "TASK_NOLOAD";
	}else if(stateid == TASK_NEW){
		status = "TASK_NEW";
	}else if(stateid == TASK_STATE_MAX){
		status = "TASK_STATE_MAX";
	}else if(stateid == TASK_KILLABLE){
		status = "TASK_WAKEKILL,TASK_UNINTERRUPTIBLE";
	}else if(stateid == TASK_STOPPED){
		status = "TASK_WAKEKILL, __TASK_STOPPED";
	}else if(stateid == TASK_TRACED){
		status = "TASK_WAKEKILL,__TASK_TRACED";
	}else if(stateid == TASK_IDLE){
		status = "TASK_UNINTERRUPTIBLE,TASK_NOLOAD";
	}else if(stateid == TASK_NORMAL){
		status = "TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE";
	}else if(stateid == TASK_REPORT){
		status = "TASK_RUNNING ,TASK_INTERRUPTIBLE ,TASK_UNINTERRUPTIBLE ,__TASK_STOPPED ,__TASK_TRACED,EXIT_DEAD,EXIT_ZOMBIE,TASK_PARKED";
	}else{
		status = "INVALID";
	}

	return status;
}




static int openProcess(struct inode *inode, struct file *file)
{

    printk("Opening processes\n");
   buffer = out ;
    for_each_process( task){ /*    for_each_process() MACRO for iterating through each task in the os located in linux\sched\signal.h    */
      // printk("\nPID: %d, PPID: %d, CPU: %d , STATE: %s, id : %d ",task->pid, task->real_parent->pid, task->cpu, task_status(task->state),i);
 
	buffer+=sprintf(buffer,"\nPID: %d, PPID: %d, CPU: %d , STATE: %s \n",task->pid, task->real_parent->pid, task->cpu, task_status(task->state));
  
    }

    return 0;
}

static ssize_t readProcess(struct file *filep, char *buffer, size_t len, loff_t *offset){
    int error_count;
	error_count = copy_to_user(buffer, out, strlen(out));

	if (error_count==0){ 
		printk(KERN_INFO "process_list: Sent %ld characters to the user\n", strlen(out));
		return strlen(out);
	}
	else {
		printk(KERN_INFO "process_list: Failed to send %d characters to the user\n", error_count);
		return -EFAULT; 
	}
}

static int closeProcess(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "process_list: Device successfully closed\n");
	return 0;
}


static const struct file_operations process_ops = {
    .owner			= THIS_MODULE,
    .open           = openProcess,
    .read           = readProcess,
	.release 			= closeProcess
};

struct miscdevice process_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "process_list",
    .fops = &process_ops,
};

static int __init misc_init(void)
{
    int error;

    error = misc_register(&process_device);
    if (error) {
        pr_err("can't misc_register :(\n");
        return error;
    }

    printk("Created character devices for process lists collection.\n");
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&process_device);
    printk("I'm out\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_DESCRIPTION("Simple Misc Driver");
MODULE_AUTHOR("Monish Devendran <mdevend1@binghamton.edu>");
MODULE_LICENSE("GPL");