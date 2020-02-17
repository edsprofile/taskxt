#include <linux/module.h> //all modules need this
#include <linux/init.h> //for the init and exit macro
#include <linux/moduleparam.h> //to pass parameter to our module
#include <linux/stat.h> //this is for macros to set permissions on files
#include <linux/kernel.h> //when working on kernel
#include <linux/fs.h> //for file operation
#include <linux/cdev.h> //for stuct cdev
#include <linux/uaccess.h> //for copy to user
#include <linux/errno.h> // for errors
#include <linux/kthread.h> //for kthread_run
#include <linux/sched.h> // for schedule_timeout
#include <linux/string.h> //for memset 
#include <linux/signal.h> // for allow_signal
#include <linux/sched/signal.h> // for signal_pending and for_each_process
#include <linux/timer.h> //for timer_list
#include <linux/ioctl.h> // for ioctl
#include <linux/fs_struct.h> // for fs_struct

#include "taskxt.h" // to include taskx_arg_t

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Edwin Espinoza");
//author and other infomation can be added later



/* PROTOTYPES */
int device_init(void);
void device_exit(void);
static ssize_t device_read(struct file *filep, char *buffer, size_t length, loff_t *offset);
static ssize_t device_write(struct file *filep, const char *buffer, size_t length, loff_t *offset);
static int device_open(struct inode *inodep, struct file *filep);
static int device_release(struct inode *inodep, struct file *filep);
static long device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
static void timer_func(struct timer_list *ptimer);
static int task_active(char *pname);
static int extract_features(void *n);
static ssize_t write_vaddr(void * v, size_t is);
static int setup(void);
static void cleanup(void);
static int write_format_output(void);
static int write_buffer(char *buffer);



/* EXTERNAL DEFINED IN: disk.c */
extern ssize_t write_vaddr_disk(void *, size_t);
extern int setup_disk(void);
extern void cleanup_disk(void);


/* VARIABLES */
static dev_t dev_maj_min;
static struct cdev char_dev;
static struct class *class_object;
static struct device *device_object;
static taskxt_arg_t process; //defined in taskxt.h
static struct timer_list timer;
static char process_name_string[50];
static int sampling = 0;
static int start_jiffies = 0;
static int end_jiffies = 0;
static int delta_jiffies = 0;
task_features tf; //defined in taskxt.h
static char fullFileName[1025];
char *filepath = 0;
static int first_loop = 1;
static int file_open = 0;
static int sample_amount = 0;

/* PASSED IN PARAMETERS */
static char *ppath = 0;
static char *pname = 0;
static int sample_rate = 0;
static int duration = 0;

module_param(ppath, charp, S_IRUGO);
module_param(pname, charp, S_IRUGO);
module_param(sample_rate, int, S_IRUGO);
module_param(duration, int, S_IRUGO);



static struct file_operations module_fops =
{
        .owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
        .unlocked_ioctl = device_ioctl
};



/* MACROS */
module_init(device_init);
module_exit(device_exit);



int __init device_init(void)
{
	int error_code = 0;
	
	printk(KERN_INFO "DEVICE_INIT\n");

	//check if a parameter is missing
	if(!ppath || !pname || !sample_rate || !duration)
	{
		printk(KERN_INFO "PARAMETER NOT SPECIFIED\n");
		return -EINVAL;
	}
	

	
	// alloc since we do not know what numbers we want ahead of time
	error_code = alloc_chrdev_region(&dev_maj_min, 0, 1, "taskxt");
	if(error_code < 0)
	{
		printk(KERN_INFO "COULD NOT ALLOC REGION\n");
		return error_code;
	}
        //major numbers and minor number should be allocated at this point
        
	//initilizing cdev and adding it to the kernel
	cdev_init(&char_dev, &module_fops);
	char_dev.owner = THIS_MODULE;
        error_code = cdev_add(&char_dev, dev_maj_min, 1);
	if(error_code < 0)
	{
		printk(KERN_INFO "COULD NOT ADD DEVICE\n");
		unregister_chrdev_region(dev_maj_min, 1);
		return error_code;
	}

	//creating the class object so the device can be added to the filesystem
	class_object = class_create(THIS_MODULE, "char");
	if(IS_ERR(class_object))
	{
		cdev_del(&char_dev);
		unregister_chrdev_region(dev_maj_min, 1);
		return PTR_ERR(class_object);
	}

	//creating the device in the filesystem
	device_object = device_create(class_object, NULL, dev_maj_min, NULL, "taskxt");
	if(IS_ERR(device_object))
	{
		class_destroy(class_object);
		cdev_del(&char_dev);
		unregister_chrdev_region(dev_maj_min, 1);
		return PTR_ERR(device_object);
	}

        //create the timer
        timer_setup(&timer, timer_func, 0);
        strcpy(process_name_string, pname);
        start_jiffies = jiffies;
        error_code = mod_timer(&timer, jiffies+msecs_to_jiffies(1));
        
        if(error_code)
        {
                printk(KERN_INFO "ERROR SETTING UP MOD_TIMER: %d\n", error_code);
                return error_code;
        }



        /* struct task_struct *task; */
        /* task = current; */
        /* printk(KERN_NOTICE "assignment: current process: %s, PID: %d\n", task->comm, task->pid); */

        /* for_each_process(task) */
        /* { */
        /*         printk("current process: %s PID: %d\n", task->comm, task->pid); */
        /* } */
                
	return 0;
}

void device_exit(void)
{
	printk(KERN_INFO "REMOVING CHAR DEVICE\n");
	device_destroy(class_object, dev_maj_min);
	class_destroy(class_object);
        cdev_del(&char_dev);
	unregister_chrdev_region(dev_maj_min, 1);
        del_timer(&timer);
	printk(KERN_INFO "DEVICE_EXIT: SUCCESS\n");
}

static ssize_t device_read(struct file *filep, char *buffer, size_t length, loff_t *offset)
{
	printk(KERN_INFO "DEVICE_READ\n");
        return 0;
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t length, loff_t *offset)
{
	printk(KERN_INFO "DEVICE_WRITE\n");
        return 0;
}

static int device_open(struct inode *inodep, struct file *filep)
{
	//since we only have one device we will just return 0 for success on open?
	return 0;
}

static int device_release(struct inode *inodep, struct file *filep)
{
	return 0;
}

//we can later use this to interact with a program from user space
static long device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
        int error_code = 0;

        switch(cmd)
        {
        case TASKXT_GET_PROCESSNAME:
                process.status = 0;
                if(task_active(process.processName))
                {
                        sampling = 1;
                        process.status = 1;
                }

                if(copy_to_user((taskxt_arg_t *)arg, &process, sizeof(taskxt_arg_t)))
                {
                        return -EACCES;
                }
                break;
        case TASKXT_SMP_PROCESSNAME:
                if(sampling == 1)
                {
                        error_code = extract_features(process.processName);
                        if(error_code)
                        {
                                return error_code;
                        }
                }
                break;
        case TASKXT_SET_PROCESSNAME:
                if(copy_from_user(&process, (taskxt_arg_t *)arg, sizeof(taskxt_arg_t)))
                {
                        return -EACCES;
                }

                strcpy(process_name_string, process.processName);
                sampling = 1;
                break;
        default:
                return -EINVAL;
        }
        
        return 0;
}

static void timer_func(struct timer_list *ptimer)
{
        int length = 0;
        int error_code = 0;
        int msecs = 0;
        
        if(first_loop == 1)
        {         
                end_jiffies = jiffies; //to see when the process actaully started
                delta_jiffies = end_jiffies - start_jiffies;
                msecs = jiffies_to_msecs(delta_jiffies);                
                printk(KERN_INFO "start_jiffies: %d, end_jiffies: %d, delta_jiffies: %d msecs: %d\n", start_jiffies, end_jiffies, delta_jiffies, msecs);

                //open file to write to
                length = strlen(ppath);
                strcpy(fullFileName, ppath);

                if(fullFileName[length-1] != '/')
                {
                        strcat(fullFileName, "/");
                }
                printk(KERN_INFO "FULLFILENAME: %s\n", fullFileName);                        
                        
                strcat(fullFileName, process_name_string);
                strcat(fullFileName, ".dat");
                printk(KERN_INFO "OUTPUT FILE: %s\n", fullFileName);
                        
                filepath = fullFileName;
        }

        
//      printk(KERN_INFO "inside of timer_func start_jiffes: %d, end_jiffies: %d, delta_jiffies: %d\n", start_jiffies, end_jiffies, delta_jiffies);
//      printk(KERN_INFO "task_active: %d\n", task_active(process.processName));
//      printk(KERN_INFO "task_active: %s\n", process.processName);

        //check for the process we are looking for
        if(task_active(process_name_string))
        {
                sampling = 1;
                process.status = 1;

                if(sample_amount < 1)
                {
                        extract_features(process_name_string);

                        sample_amount++;
                        printk(KERN_INFO "SAMPLE_AMOUNT: %d\n", sample_amount);

                        sampling = 0;
                }
 
        }
        
        
        mod_timer(ptimer, jiffies+msecs_to_jiffies(1));//I want it to wake up 1 miliseconds
        first_loop = 0;
}

static int task_active(char *pname)
{
        int flag = 0;
        struct task_struct *task;

        for_each_process(task)
        {
                if(!strcmp(task->comm, pname))
                {
                        flag = 1;
                        //printk(KERN_INFO "taskxt: match: %s and %s\n", task->comm, pname);
                        break;
                }
        }

        return flag;
}

static int extract_features(void *n)
{
        //extract_features declaration
        int found = 0;
        struct task_struct *task;
        sample process_info;
        int i = 0;
        int length = 0;
        int error_code = 0;
        int loop = duration / sample_rate;

        tf.pid[0] = 0;
        tf.cnt = 0;

        
        if(sampling)
        {
                for_each_process(task)
                {
                        if(!strcmp(task->comm, process_name_string))
                        {
                                found = 1;
                                //printk(KERN_INFO "FOUND PROCESS: %s\n", process_name_string);
                                break;
                        }
                }


                if(found)
                {
                        tf.pid[0]=0;
                        tf.cnt = 0;                
                        memset(&process_info, 0, sizeof(process_info));

                        for(i = 0; i < loop; i++)
                        {                
                        
                                for_each_process(task)
                                {                         
                                        if(!strcmp(task->comm, process_name_string))
                                        {
                                                if((task->active_mm))
                                                {
                                                        process_info.map_count = (*task->active_mm).map_count;
                                                        process_info.page_table_lock = (*task->active_mm).page_table_lock.rlock.raw_lock.val.counter;
                                                        process_info.hiwater_rss = (*task->active_mm).hiwater_rss;
                                                        process_info.hiwater_vm =(*task->active_mm).hiwater_vm;
                                                        process_info.total_vm = (*task->active_mm).total_vm;
                                                        process_info.shared_vm = (*task->active_mm).data_vm;
                                                        process_info.exec_vm = (*task->active_mm).exec_vm;
                                                        process_info.nr_ptes = (*task->active_mm).pgtables_bytes.counter;
                                                }
                                
                                                process_info.utime = task->utime;
                                                process_info.stime = task->stime;
                                                process_info.nvcsw = task->nvcsw;
                                                process_info.nivcsw = task->nivcsw;
                                                process_info.min_flt = task->min_flt;
                                                process_info.slock = task->alloc_lock.rlock.raw_lock.locked;
                                        
                                                if(task->fs)
                                                {
                                                        process_info.fscount = (*task->fs).users;
                                                }
                                        }
                                }
                                memcpy(&tf.pid[tf.cnt], &process_info, sizeof(process_info));
                                tf.cnt += sizeof(process_info);
                                memset(&process_info, 0, sizeof(process_info));
        
                        }

                        if((error_code = setup()))
                        {
                                cleanup();
                        }
                        
                        if(!error_code)
                        {
                                error_code = write_format_output();
                        }

                        cleanup();
                }
        }
        
        

        sampling = 0;
        //do_exit(0);
        
        return 0;
        
}

static int write_format_output(void)
{
        int error_code = 0;
        int i;
        int count;
        int loop;
        sample process_info;
        char buffer[100];
        char outputBuffer[500];

        loop = tf.cnt / sizeof(process_info);

        printk(KERN_INFO "IN WRITE FORMAT OUTPUT\n");
        for(i = 0; i < loop; i++)
        {
                count = i * sizeof(process_info);
                memcpy(&process_info, &tf.pid[count], sizeof(process_info));

                // Memory related features
                // map_count -> number of memory regions of a process
                sprintf(buffer, "%d", process_info.map_count);
                strcpy(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // page_table_lock -> used to mange the page table entries
                sprintf(buffer, "%lu", process_info.page_table_lock);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // hiwater_rss -> Max number of page frames ever owned by the process
                sprintf(buffer, "%lu", process_info.hiwater_rss);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // hiwater_vm -> Max number of pages appeared in memory region of process
                sprintf(buffer, "%lu", process_info.hiwater_vm);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // total_vm -> Size of process's address space in terms of number of pages
                sprintf(buffer, "%lu", process_info.total_vm);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // shared_vm -> Number of pages in shared file memory mappings of process
                sprintf(buffer, "%lu", process_info.shared_vm);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // exec_vm -> number of pages in executable memory mappings of process
                sprintf(buffer, "%lu", process_info.exec_vm);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // nr_ptes -> number of pages tables of a process
                sprintf(buffer, "%lu", process_info.nr_ptes);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // utime -> Tick count of a process that is executing in user mode
                sprintf(buffer, "%ld", process_info.utime);
                strcat(outputBuffer, buffer); strcat(outputBuffer, ",");

                // stime -> Tick count of a process in the kernel mode
                sprintf(buffer, "%ld", process_info.stime);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // nvcsw -> number of volunter context switches
                sprintf(buffer, "%lu", process_info.nvcsw);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // nivcsw -> number of in-volunter context switches
                sprintf(buffer, "%lu", process_info.nivcsw);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // min_flt -> Contains the minor page faults
                sprintf(buffer, "%lu", process_info.min_flt);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // alloc_lock.raw_lock.slock -> used to locl memory manager, files and file system etc.
                sprintf(buffer, "%lu", process_info.slock);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, ",");

                // fs.count - > number of file usage (was count, now field called users)
                sprintf(buffer, "%d", process_info.fscount);
                strcat(outputBuffer, buffer);
                strcat(outputBuffer, "\n");
       
                error_code = write_buffer(outputBuffer);
                if (error_code)
                {
                        return error_code;
                }
        }

        return error_code;
}

static int write_buffer(char *buffer)
{
        ssize_t s;
        int length;

        length = strlen(buffer);
        s = write_vaddr(buffer, length);

        if(s != length)
        {
                printk(KERN_INFO "ERROR IN WRITE_BUFFER");
                return (int) s;
        }

        return 0;
}

static ssize_t write_vaddr(void * v, size_t is)
{
        return (write_vaddr_disk(v, is));
}

static int setup(void)
{
        return (setup_disk());
}

static void cleanup(void)
{
        return (cleanup_disk());
}
