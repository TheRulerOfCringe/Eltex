#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define MAX_LEN 100

// module attributes
MODULE_LICENSE("GPL"); // this avoids kernel taint warning
MODULE_DESCRIPTION("Device Driver");
MODULE_AUTHOR("Ilya");

static char msg[MAX_LEN]={0};
static short readPos=0;
static int times = 0;

/* prototypes,else the structure initialization that follows fail */
static int dev_open(struct inode *, struct file *);
static int dev_rls(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);  // исправлено: было esize_t
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);  // исправлено: было esize_t

// structure containing callbacks
static struct file_operations fops = {  // добавлена фигурная скобка
    .read = dev_read, // address of dev_read
    .open = dev_open, // address of dev_open
    .write = dev_write, // address of dev_write
    .release = dev_rls, // address of dev_rls
};  // добавлена точка с запятой

// called when module is loaded, similar to main()
int init_module(void)
{
    int t = register_chrdev(0,"chardev",&fops); //register driver with major:89
    if (t<0) printk(KERN_ALERT "Device registration failed...\n");  // исправлено: printk
    else printk(KERN_ALERT "Device registered...\n");  // исправлено: printk

    return t;
}

// called when module is unloaded, similar to destructor in OOP
void cleanup_module(void)  // исправлено: было cleanup_model(void)
{
    unregister_chrdev(0,"chardev");
}

// called when 'open' system call is done on the device file
static int dev_open(struct inode *inod,struct file *fil)
{
    times++;
    printk(KERN_ALERT"Device opened %d times\n",times);  // исправлено: printk
    return 0;
}

// called when 'read' system call is done on the device file
static ssize_t dev_read(struct file *filp,char *buff,size_t len,loff_t *off)
{
    short count = 0;
    while (len && (msg[readPos]!=0))  // исправлено: было !en
    {
        put_user(msg[readPos],buff++); //copy byte from kernel space to user space
        count++;
        len--;
        readPos++;
    }
    return count;
}

static ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    // Ограничиваем длину
    if (len > MAX_LEN - 1)
        len = MAX_LEN - 1;
    
    // Сбрасываем позицию чтения
    readPos = 0;
    
    // Копируем всю строку целиком
    if (copy_from_user(msg, buff, len))
        return -EFAULT;
    
    // Добавляем нуль-терминатор
    msg[len] = '\0';
    
    return len;
}

// called when 'close' system call is done on the device file
static int dev_rls(struct inode *inod,struct file *fil)
{
    printk(KERN_ALERT"Device close\n");  // исправлено: printk
    return 0;
}
