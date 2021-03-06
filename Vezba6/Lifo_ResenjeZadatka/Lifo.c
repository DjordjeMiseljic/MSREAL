#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>	//register_chrdev_region, fops ... char support
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
MODULE_LICENSE("Dual BSD/GPL");


//function prototypes
int lifo_open(struct inode *pinode, struct file *pfile);
int lifo_close(struct inode *pinode, struct file *pfile);
ssize_t lifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
static ssize_t lifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

//***************************************************************
static unsigned long strToInt(const char* pStr, int len, int base);
static char chToUpper(char ch);
static int intToStr(int val, char* pBuf, int bufLen, int base);
//****************************************************************


//*****************GLOBAL VARIABLES**************************
int pos = 0;
int cat_iterations = 0;
int amount_to_print = 1;
int lifo_num = 0;
int endRead = 0;
dev_t dev_id;
int hex_or_dec = 10;
struct cdev *my_cdev;
int lifo[16];
struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = lifo_open,
	.read = lifo_read,
	.write = lifo_write,
	.release = lifo_close,
};
//***********************************************************

int lifo_open(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully opened file\n");
	return 0;
}

int lifo_close(struct inode *pinode, struct file *pfile) 
{
	printk(KERN_INFO "Succesfully closed file\n");
	return 0;
}

ssize_t lifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{

	char temp_array[100];
	int len;
	int i = 0;
	if (endRead) 
	{
		endRead = 0;
		cat_iterations = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	if (pos == 0)
	{
		printk("buffer is empty");
		return 0;
	} 
	len = intToStr(lifo[--pos], temp_array, 100, hex_or_dec);
	cat_iterations ++;
	if(cat_iterations != amount_to_print && pos != 0)
	{
		temp_array[len] = ',';
		len++;

	}
	else
	{
		temp_array[len] = '\n';
		len++;
		endRead = 1;

	}
	i = copy_to_user(buffer, temp_array, len);    
	return len;
}

static ssize_t lifo_write(struct file *pfile,const  char __user *buffer, size_t length, loff_t *offset) 
{

	char *right_ptr;
	char *left_ptr;
	char buff [length];
	int num_of_commas = 0;
	int buff_len;
	int i = 0;
	i = copy_from_user(buff, buffer, length);

	buff[length-1] = '\0';
	left_ptr = buff;
	if(strncmp("hex", buff, 3) == 0) {
		hex_or_dec = 16;
		printk("Cat will print in hexadecimal format\n");
		return length;
	}
	if(strncmp("dec", buff, 3) == 0) {
		hex_or_dec = 10;
		printk("Cat will print in decimal format\n");
		return length;
	}
	if(strncmp("num=", buff, 4) == 0) {
		left_ptr = strchr(buff, '=');
		left_ptr++;
		amount_to_print = strToInt(left_ptr, strlen(left_ptr), 10);
		printk("Amount to print is: %d", amount_to_print);
		return length;
	}

	for (i = 0; i < strlen(buff); i++) {
		if(buff[i] == ',')
			num_of_commas++;
	}
	for (i = 0; i < (num_of_commas + 1); i++) {
		right_ptr = strchr(left_ptr, ',');
		if (right_ptr != NULL) {
			*right_ptr = '\0';
			right_ptr++;
			buff_len = strlen(left_ptr);
			if (pos == 15)
			{
				printk("Lifo buffer is full\n");
				return length;
			}
			lifo[pos] = strToInt(left_ptr, buff_len, 16);
			//printk ("num %d is %d", pos, lifo[pos]);
			pos++;
			left_ptr = right_ptr;
		}
		else {
			buff_len = strlen(left_ptr);
			if (pos == 15)
			{
				printk("lifo buffer is full\n");
				return length;
			}
			lifo[pos] = strToInt(left_ptr, buff_len, 16);
			pos++;
			break;
		}


	}
	return length;
}

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

static int __init lifo_init(void)
{
	int ret = 0;
	int i;
	ret = alloc_chrdev_region(&dev_id,0,1,"Lifo");
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

	printk(KERN_INFO "Lifo, world\n");
	for (i=0; i<10; i++) {
		lifo[i] = 0;
	}

	return 0;
}

static void __exit lifo_exit(void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(dev_id,1);


	printk(KERN_INFO "Goodbye, cruel world\n");
}

module_init(lifo_init);
module_exit(lifo_exit);
