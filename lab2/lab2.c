#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>
#define SIZE 10
#define PROC_NAME "lab2"
 
static int len = 0;
static int temp = 0;
static char *msg = NULL;
 
static ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp ) {
    if(count > temp) {
        count = temp;
    }
    temp = temp - count;
    int unread_byte = copy_to_user(buf, msg, count);
    if (unread_byte >0 ) return -EFAULT;
    if(count == 0)
        temp = len;
    return count;
}
 
static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp) {
    int unread_byte = copy_from_user(msg, buf, count);  
    if (unread_byte>0) return -EFAULT;   
    len = count;
    temp = len;
    return count;

}
 
static const struct proc_ops proc_fops = {
    proc_read: read_proc,
    proc_write: write_proc,
};
 
static void create_new_proc_entry(void) { //use of void for no arguments is compulsory now
    proc_create(PROC_NAME, 0, NULL, &proc_fops);
    msg = kmalloc(SIZE * sizeof(char), GFP_KERNEL);
}
 
static int __init proc_init (void) {
    printk(KERN_INFO "hello");
    create_new_proc_entry();
    return 0;
}
 
static void __exit proc_cleanup(void) {
    printk(KERN_INFO "bye bye");
    remove_proc_entry("PROC_NAME", NULL);
    kfree(msg);
}
 
MODULE_LICENSE("LALALA");
MODULE_AUTHOR("Bakuk Alekssandr");
module_init(proc_init);
module_exit(proc_cleanup);

