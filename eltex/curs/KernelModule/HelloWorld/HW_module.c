#include <linux/module.h>
/*
        printk()
        KERN_ALERT
*/
#include <linux/kernel.h>
/*
        __init
        __exit
*/
#include <linux/init.h>

static int __init InitModule(void)
{
        printk(KERN_ALERT "Hello Worl from Module\n");
        
        return 0;
}

static void __exit ExitModule(void)
{
        printk(KERN_ALERT "HW Module is down\n");;
}

module_init(InitModule);
module_exit(ExitModule);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya Zakrevsky");
MODULE_DESCRIPTION("A module for print 'Hello' to log file.");

