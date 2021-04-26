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
#include <linux/pid.h>
#include <asm/pgtable.h>
#include <linux/mm_types.h>
#include <linux/ioctl.h>

#define IOCTL_GET_PFN _IOWR('p', 1, unsigned long)


#define TRUE 1
#define FALSE 0



struct pid *pid;
struct task_struct *pid_struct;
struct mm_struct *pid_mm_struct;
struct vm_area_struct *vma;



pgd_t *pgd;
p4d_t *p4d;
pud_t *pud;
pmd_t *pmd;
pte_t *pte;


unsigned long vaddr;

static int process_id = 0;
module_param(process_id, int, S_IRUGO);


static int openPage(struct inode *inode, struct file *file)
{
    printk("Opening Page\n");
    return 0;
}

static ssize_t readPage(struct file *filep, char *buffer, size_t len, loff_t *offset){
    printk("reading page\n");
    return 0;
}

static int closePage(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "page walk: Device successfully closed\n");
	return 0;
}


static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    int error_count;
    printk("inside myioctl");
    unsigned long virt_addr;
    unsigned long phy_addr;
    switch (cmd)
    {
        case IOCTL_GET_PFN:
            copy_from_user(&virt_addr,(unsigned long*)arg,sizeof(unsigned long));
            printk("virtual address %lx",virt_addr);
            pid = find_get_pid (process_id);
            pid_struct = pid_task(pid, PIDTYPE_PID);
            pid_mm_struct = pid_struct->mm;
             pgd = pgd_offset(pid_struct->mm, virt_addr);
                if(pgd_present(*pgd) == TRUE){
                    p4d =p4d_offset(pgd, virt_addr);
                }
                if(p4d_present(*p4d) == TRUE){
                    pud =  pud_offset(p4d, virt_addr);
                }
                if(pud_present(*pud) == TRUE){
                    pmd = pmd_offset(pud, virt_addr);
                }
                if(pmd_present(*pmd) == TRUE){
                    pte= pte_offset_map(pmd,virt_addr);
                }
                if (pte_present(*pte) == TRUE) {
                   phy_addr = pte_pfn(*pte);
                }

            
                printk(KERN_ALERT "Page Frame Number= %lx\n", phy_addr);
            //}

            error_count = copy_to_user((unsigned long*)arg,&phy_addr,sizeof(unsigned long));
            if (error_count==0){ 
                printk("success");
                return arg;
            }
            else {
				return -EACCES;
			}
            // printk("physical address : %lx",phy_addr);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}
 


static const struct file_operations page_ops = {
    .owner			= THIS_MODULE,
    .open           = openPage,
    .read           = readPage,
	.release 		= closePage,
    .unlocked_ioctl = my_ioctl
};

struct miscdevice page_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "page_walk",
    .fops = &page_ops,
};

static int __init misc_init(void)
{
    int error;

    error = misc_register(&page_device);
    if (error) {
        pr_err("can't misc_register :(\n");
        return error;
    }

    if(process_id == 0){
        printk("Note: PageWalk module is successfully loaded.\n\tIf you want to check page frame number\n\t for running process pass process_id greater than 0 \n\tto check page frame for particular process id.");
    }else{
        pid = find_get_pid(process_id);
        pid_struct = pid_task(pid, PIDTYPE_PID);
        pid_mm_struct = pid_struct->mm;


        for (vma = pid_mm_struct->mmap; vma; vma = vma->vm_next) {
             unsigned long page_frame_number;
            printk(KERN_ALERT "start address :  %lx and end address :  %lx\n", vma->vm_start,  vma->vm_end);
            for(vaddr = vma->vm_start; vaddr < vma->vm_end; vaddr++){
                pgd = pgd_offset(pid_struct->mm, vaddr);
                if(pgd_present(*pgd) == TRUE){
                    p4d =p4d_offset(pgd, vaddr);
                }
                if(p4d_present(*p4d) == TRUE){
                    pud =  pud_offset(p4d, vaddr);
                }
                if(pud_present(*pud) == TRUE){
                    pmd = pmd_offset(pud, vaddr);
                }
                if(pmd_present(*pmd) == TRUE){
                    pte= pte_offset_map(pmd,vaddr);
                }
                if (pte_present(*pte) == TRUE) {
                   page_frame_number = pte_pfn(*pte);
                }
            }
            printk(KERN_ALERT "Page Frame Number= %lx\n", page_frame_number);
        }
    }
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&page_device);
    printk("I'm out\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_DESCRIPTION("Simple Misc Driver");
MODULE_AUTHOR("Monish Devendran <mdevend1@binghamton.edu>");
MODULE_LICENSE("GPL");