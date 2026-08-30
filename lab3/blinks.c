/* 
 * kbleds.c – мигает светодиодами клавиатуры, пока не будет выгружен. 
 */ 
 
#include <linux/init.h> 
#include <linux/kd.h> /* Для KDSETLED. */ 
#include <linux/module.h> 
#include <linux/tty.h> /* Для tty_struct. */ 
#include <linux/vt.h> /* Для MAX_NR_CONSOLES. */ 
#include <linux/vt_kern.h> /* Для fg_console. */ 
#include <linux/console_struct.h> /* Для vc_cons. */ 
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/string.h>

#define MODE 0664
#define KOBJ "lab3"
 
MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs."); 
 
static struct timer_list my_timer; 
static struct tty_driver *my_driver; 
static unsigned long kbledstatus = 0;
static struct kobject *example_kobject;
static int test; 
static int leds[] = {0x001,0x002,0x004,0x002};
static int index_leds =0;
 
#define MODE 0664
#define KOBJ "lab3"
#define BLINK_DELAY HZ / 5
#define ALL_LEDS_ON 0x007 
#define RESTORE_LEDS 0xFF 

 
/* Функция my_timer_func периодически мигает светодиодами, 
 * вызывая для драйвера клавиатуры команду управления вводом-выводом  
 * KDSETLED. Дополнительную информацию по командам ввода-вывода 
 * смотрите в функции vt_ioctl() файла drivers/tty/vt/vt_ioctl.c. 
 * 
 * Аргумент KDSETLED попеременно устанавливается то на 7 (что приводит к 
 * активации режима LED_SHOW_IOCTL и загоранию всех светодиодов), то на 
 * 0xFF (любое значение выше 7 переключает режим обратно на 
 * LED_SHOW_FLAGS, в результате чего светодиоды отображают фактический 
 * статус клавиатуры). Подробности смотрите в функции setledstate() файла 
 * drivers/tty/vt/keyboard.c.
  */ 
static void my_timer_func(struct timer_list *unused) 
{ 
    struct tty_struct *t = vc_cons[fg_console].d->port.tty;
    
    if (test==0x000) {
        kbledstatus = leds[index_leds];
        index_leds++;
        if (index_leds==4) index_leds=0;
    }
    else {
        kbledstatus = RESTORE_LEDS;
        if (kbledstatus == test) 
        kbledstatus = RESTORE_LEDS; 
    else 
        kbledstatus = test;
    }
 
    
 
    (my_driver->ops->ioctl)(t, KDSETLED, kbledstatus); 
 
    my_timer.expires = jiffies + BLINK_DELAY; 
    add_timer(&my_timer); 
} 

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{
    return sprintf(buf, "%d\n", test);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    sscanf(buf, "%du", &test);
    return count;
}

static struct kobj_attribute foo_attribute = __ATTR(test, MODE, foo_show,
                                                    foo_store);
 
static int __init kbleds_init(void) 
{ 
    int i,error=0; 
 
    pr_info("kbleds: loading\n"); 
    pr_info("kbleds: fgconsole is %x\n", fg_console); 

    for (i = 0; i < MAX_NR_CONSOLES; i++) { 
        if (!vc_cons[i].d) 
            break; 
        pr_info("poet_atkm: console[%i/%i] #%i, tty %p\n", i, MAX_NR_CONSOLES, 
                vc_cons[i].d->vc_num, (void *)vc_cons[i].d->port.tty); 
    } 
    pr_info("kbleds: finished scanning consoles\n"); 

    example_kobject = kobject_create_and_add(KOBJ,
                                             kernel_kobj);
    if (!example_kobject)
        return -ENOMEM;

    error = sysfs_create_file(example_kobject, &foo_attribute.attr);
    if (error)
    {
        pr_debug("failed to create the foo file in /sys/kernel/%s \n",KOBJ);
    }
 
    my_driver = vc_cons[fg_console].d->port.tty->driver; 
    pr_info("kbleds: tty driver magic %d\n", my_driver->major); 
 
    /* Первая настройка таймера мигания светодиодов. */ 
    timer_setup(&my_timer, my_timer_func, 0); 
    my_timer.expires = jiffies + BLINK_DELAY; 
    add_timer(&my_timer); 

    
     return error; 
} 
 
static void __exit kbleds_cleanup(void) 
{ 
    pr_info("kbleds: unloading...\n"); 
    del_timer(&my_timer); 
    (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, 
                            RESTORE_LEDS); 
    kobject_put(example_kobject);
} 
 
module_init(kbleds_init); 
module_exit(kbleds_cleanup); 
 
MODULE_LICENSE("GPL");