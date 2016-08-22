#ifndef _HW_MODULE_H_
#define _HW_MODULE_H_

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
/*
        struct file_operations
        struct inode
        struct file
        register_chrdev
*/
#include <linux/fs.h>
/*
        put_user
        get_user
*/
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya Zakrevsky");
MODULE_DESCRIPTION("A module for print 'Hello' to log file.");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "HW_module"
#define BUF_LEN 15

static int major;

static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
};

#endif
