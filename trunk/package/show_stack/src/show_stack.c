#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("SanLongCai");

static int show_pid = 1;
module_param(show_pid, int, 0);

static int myatoi(const char *buf, unsigned int count)
{
    int i;
    int res;
    
	if(! buf || count == 0)
		return 0;
    
	res = 0;
    i = 0;
    
	while(i < count && buf[i]) {
		if(buf[i] < '0' || buf[i] > '9') {
			printk(KERN_INFO "the buf:%s conn't convert to int", buf);
			return 0;
		}
		
		res = res * 10 + buf[i] - '0';
		i++;
	}
	
	return res;
}

extern void show_stack(struct task_struct *task, unsigned long *sp);
extern void show_user_raw_stack_by_task(struct task_struct *task);

static int sstack_show()
{
	struct task_struct *task;
	
	if((task = find_task_by_vpid(show_pid)) == NULL)
    {
		printk("Process %d not found!!", show_pid);
		return 1;
	}
   
	printk("\n-----Start dump kernel stack-----\n");
	show_stack(task, NULL);
	printk("\n-----End dump kernel stack && Start dump user stack-----\n");
	show_user_raw_stack_by_task(task);
	printk("\n-----End dump user stack-----\n");
	
	return 0;
}

static int sstack_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	int len = 0;
	len += sprintf(page, "%d\n", show_pid);
}

static int sstack_write_proc(struct file *file, const char *buffer, unsigned long
count, void *data)
{
    char buf[16];
    int new_pid;
    
	if(count > 16) {
		printk(KERN_INFO "the count is too large");
		return -EINVAL;
    }
    
	if(copy_from_user(buf, buffer, count)) {
		return -EFAULT;
    }
    
	if(buf[count - 1] < '0' || buf[count - 1] > '9')
		buf[count -1] = '\0';
    
	if(new_pid = myatoi(buf, 16))
    {
		int ret;
		show_pid = new_pid;
		printk(KERN_INFO "new_pid=%d", new_pid);
		if(ret = sstack_show())
			return ret;
		return count;
    }
    else
		return -EINVAL;
}

static void sstack_create_proc(void)
{
    struct proc_dir_entry *res = create_proc_entry("show_stack", S_IRUGO | S_IWUGO, NULL);
	
	if(res) {
		res->read_proc = sstack_read_proc;
		res->write_proc = sstack_write_proc;
	}
	else
		printk(KERN_INFO "create the proc failure");
}

static int __init sstack_init(void)
{
    printk(KERN_INFO "sstack_init\n");
    sstack_create_proc();
    return 0;
}

static void __exit sstack_exit(void)
{
    printk(KERN_INFO "sstack_exit\n");
    remove_proc_entry("show_stack", NULL);
}

module_init(sstack_init);
module_exit(sstack_exit);
