#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	//register_chrdev_region, fops ... char support
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
MODULE_LICENSE("Dual BSD/GPL");


//fops functions
int storage_open(struct inode *pinode, struct file *pfile);
int storage_close(struct inode *pinode, struct file *pfile);
ssize_t storage_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
static ssize_t storage_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);
//utility functions
static unsigned long strToInt(const char* pStr, int len, int base);
static char chToUpper(char ch);
static int intToStr(int val, char* pBuf, int bufLen, int base);
//global variables
int endRead = 0;
dev_t dev_id;
struct cdev *my_cdev;
int niz[10];
int pos = 0;

//fops
struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = storage_open,
	.read = storage_read,
	.write = storage_write,
	.release = storage_close,
};

//************** FILE OPERATIONS *********************

int storage_open(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully opened file\n");
	return 0;
}

int storage_close(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully closed file\n");
	return 0;
}

ssize_t storage_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{

	int ret;
	char temp_array[100];
	int len;
	if (endRead) {
		endRead = 0;
		pos = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	len = intToStr(niz[pos], temp_array, 100, 10);
	temp_array[len] = ',';
	len++;
	ret=copy_to_user(buffer, temp_array, len);
	pos ++;
	if (pos == 10) {
		endRead = 1;
	}
	return len;
}

static ssize_t storage_write(struct file *pfile,const  char __user *buffer, size_t length, loff_t *offset) 
{

	char *tmp;
	char buff [length];

	int buff_len;
	int number, place;
	int ret;
	ret = copy_from_user(buff, buffer, length);
	buff[length-1] = '\0';
	tmp = strchr(buff, ','); 
	*tmp = '\0'; 
	tmp++;
	buff_len = strlen(buff);
	number = strToInt(buff, buff_len, 10); 

	place = strToInt(tmp, strlen(tmp), 10); 
	niz[place] = number; 

	printk(KERN_INFO "Succesfully wrote into file\n"); 
	return length;
}

//************ UTILITY FUNCTIONS ***********************
static char chToUpper(char ch)
{
	if((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
	{
		return ch;
	}
	else
	{
		return ch - ('a'-'A');
	}
}

static unsigned long strToInt(const char* pStr, int len, int base)
{
	//                      0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,?,@,A ,B ,C ,D ,E ,F
	static const int v[] = {0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15};
	int i   = 0;
	unsigned long val = 0;
	int dec = 1;
	int idx = 0;

	for(i = len; i > 0; i--)
	{
		idx = chToUpper(pStr[i-1]) - '0';

		if(idx > sizeof(v)/sizeof(int))
		{
			printk("strToInt: illegal character %c\n", pStr[i-1]);
			continue;
		}

		val += (v[idx]) * dec;
		dec *= base;
	}

	return val;
}

static int intToStr(int val, char* pBuf, int bufLen, int base)
{
	static const char* pConv = "0123456789ABCDEF";
	int num = val;
	int len = 0;
	int pos = 0;

	while(num > 0)
	{
		len++;
		num /= base;
	}

	if(val == 0)
	{
		len = 1;
	}

	pos = len-1;
	num = val;

	if(pos > bufLen-1)
	{
		pos = bufLen-1;
	}

	for(; pos >= 0; pos--)
	{
		pBuf[pos] = pConv[num % base];
		num /= base;
	}

	return len;
}

//************ INIT AND EXIT ***********************
static int __init storage_init(void)
{
	int ret = 0;
	int i;
	ret = alloc_chrdev_region(&dev_id,0,1,"Storage");
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
	for (i=0; i<10; i++) {
		niz[i] = 0;
	}

	return 0;
}

static void __exit storage_exit(void)
{
	unregister_chrdev_region(dev_id,1);
	cdev_del(my_cdev);

	printk(KERN_INFO "Goodbye, cruel world\n");
}

module_init(storage_init);
module_exit(storage_exit);
