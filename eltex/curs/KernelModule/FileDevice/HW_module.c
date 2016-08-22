#include "HW_module.h"

static int __init InitModule(void)
{
        major = register_chrdev(0, DEVICE_NAME, &fops);

        if(major < 0) {    
                printk(KERN_ALERT DEVICE_NAME": Registering the character device failed with %d\n", major);
                
                return major;
        }
        printk(KERN_ALERT DEVICE_NAME": MJ %d MN 1\n", major);
        
        return 0;
}

static void __exit ExitModule(void)
{
        unregister_chrdev(major, DEVICE_NAME);    
}

module_init(InitModule);
module_exit(ExitModule);

static int device_open(struct inode *inode, struct file *file)
{
        try_module_get(THIS_MODULE);

        return SUCCESS;
}
static int device_release(struct inode *inode, struct file *file)
{
        module_put(THIS_MODULE);

        return SUCCESS;
}
static ssize_t device_read(struct file *filep, char *buffer, size_t lenght, loff_t *offset)
{
        size_t writed = 0;
        char msg[BUF_LEN] = "HelloWorld";
        
        while(msg[writed] != '\0' && writed < lenght){
                put_user(*(msg + writed), buffer++);
                writed++;
        }

        return writed;
}
static ssize_t device_write(struct file *filep, const char *buffer, size_t lenght, loff_t *offset)
{
        char msg[BUF_LEN] = "";
        size_t readed = 0;
        
        while(readed < BUF_LEN && readed < lenght){
                get_user(*(msg + readed), buffer++);
                readed++;
        }
        if(readed >= BUF_LEN - 1){
                msg[BUF_LEN - 1] = '\0';
        } else {
                msg[readed + 1] = '\0';
        }
        
        printk(DEVICE_NAME": %s\n", msg);

        return readed;
}
