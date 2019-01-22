//#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	//register_chrdev_region, fops ... char support
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");

dev_t dev_id;
struct cdev *my_cdev;

int hello_open(struct inode *pinode, struct file *pfile);
int hello_close(struct inode *pinode, struct file *pfile);
ssize_t hello_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t hello_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
	.release = hello_close,
};


int hello_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int hello_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t hello_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
}

ssize_t hello_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
		printk(KERN_INFO "Succesfully wrote into file\n");
		return length;
}

static int __init hello_init(void)
{
	int ret = 0;
	dev_id = MKDEV(240,0);
	ret = register_chrdev_region(dev_id,1,"Hello");
	if(ret)
	{
		printk(KERN_ERR "failed to register char device\n");
		return ret;
	}

	my_cdev = cdev_alloc();
	my_cdev -> owner = THIS_MODULE;
	my_cdev -> ops = &my_fops;
	ret = cdev_add(my_cdev,dev_id,1);
	if(ret)
	{
		unregister_chrdev_region(dev_id,1);
		printk(KERN_ERR "failed to add char device\n");
		return ret;
	}

	printk(KERN_INFO "Hello, world\n");
	return 0;
}

static void __exit hello_exit(void)
{
	unregister_chrdev_region(dev_id,1);
	cdev_del(my_cdev);

	printk(KERN_INFO "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
