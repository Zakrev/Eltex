#ifndef _GROOT_H_
#define _GROOT_H_

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
        get_user
*/
#include <asm/uaccess.h>

/*
        struct task_struct
        for_each_process
*/
#include <linux/sched.h>

/*
        struct cred
        prepare_kernel_cred
        commit_cred
*/
#include <linux/cred.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya Zakrevsky");
MODULE_DESCRIPTION("A module for set process in root group. Kernel 3.16");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
void CommitCred(struct cred *, struct task_struct *);

#define DEVICE_NAME "GRoot"
#define BUF_LEN 10

static int major;

static struct file_operations fops = {
  .write = device_write,
  .open = device_open,
  .release = device_release
};

#endif
