#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/io.h>

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Timer driver");
#define DEVICE_NAME "xilaxitimer"
#define DRIVER_NAME "xilaxitimer"
//#define IRQ_NUM		164

#define XIL_AXI_TIMER_BASEADDR 0x42800000
#define XIL_AXI_TIMER_HIGHADDR 0x4280FFFF

#define XIL_AXI_TIMER_TCSR_OFFSET		0x0
#define XIL_AXI_TIMER_TLR_OFFSET		0x4
#define XIL_AXI_TIMER_TCR_OFFSET		0x8
#define XIL_AXI_TIMER_CSR_INT_OCCURED_MASK	0x00000100

#define XIL_AXI_TIMER_CSR_CASC_MASK		0x00000800
#define XIL_AXI_TIMER_CSR_ENABLE_ALL_MASK	0x00000400
#define XIL_AXI_TIMER_CSR_ENABLE_PWM_MASK	0x00000200
#define XIL_AXI_TIMER_CSR_INT_OCCURED_MASK	0x00000100
#define XIL_AXI_TIMER_CSR_ENABLE_TMR_MASK	0x00000080
#define XIL_AXI_TIMER_CSR_ENABLE_INT_MASK	0x00000040
#define XIL_AXI_TIMER_CSR_LOAD_MASK		0x00000020
#define XIL_AXI_TIMER_CSR_AUTO_RELOAD_MASK	0x00000010
#define XIL_AXI_TIMER_CSR_EXT_CAPTURE_MASK	0x00000008
#define XIL_AXI_TIMER_CSR_EXT_GENERATE_MASK	0x00000004
#define XIL_AXI_TIMER_CSR_DOWN_COUNT_MASK	0x00000002
#define XIL_AXI_TIMER_CSR_CAPTURE_MODE_MASK	0x00000001

#define TIMER_CNT	0xF8000000
//*************************************************************************
static int timer_probe(struct platform_device *pdev);
static int timer_open(struct inode *i, struct file *f);
static int timer_close(struct inode *i, struct file *f);
static ssize_t timer_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t timer_write(struct file *f, const char __user *buf, size_t count,
                         loff_t *off);
static int __init timer_init(void);
static void __exit timer_exit(void);
static int timer_remove(struct platform_device *pdev);


static char chToUpper(char ch);
static unsigned long strToInt(const char* pStr, int len, int base);
static void setup_and_start_timer(unsigned int milliseconds);

//*********************GLOBAL VARIABLES*************************************
static struct file_operations timer_fops =
  {
    .owner = THIS_MODULE,
    .open = timer_open,
    .release = timer_close,
    .read = timer_read,
    .write = timer_write
  };
static struct of_device_id timer_of_match[] = {
  { .compatible = "xlnx,xps-timer-1.00.a", },
  { /* end of list */ },
};
static struct platform_driver timer_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= timer_of_match,
  },
  .probe		= timer_probe,
  .remove	= timer_remove,
};

struct timer_info {
  unsigned long mem_start;
  unsigned long mem_end;
  void __iomem *base_addr;
  int irq_num;
};

static struct timer_info *tp = NULL;

MODULE_DEVICE_TABLE(of, timer_of_match);


static struct cdev c_dev;
static dev_t first;
static struct class *cl;
static int int_cnt;

//***************************************************
// INTERRUPT SERVICE ROUTINE (HANDLER)

static irqreturn_t xilaxitimer_isr(int irq,void*dev_id)		
{      
  unsigned int data;
  
  /* 
   * Check Timer Counter Value
   */
  data = ioread32(tp->base_addr + XIL_AXI_TIMER_TCR_OFFSET);
  printk("xilaxitimer_isr: Interrupt Occurred ! Timer Count = 0x%08X\n",data);
  
  /* 
   * Clear Interrupt
   */
  data = ioread32(tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  iowrite32(data | XIL_AXI_TIMER_CSR_INT_OCCURED_MASK,
	    tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  
  /* 
   * Disable Timer after 100 Interrupts
   */
  int_cnt++;
  
  if (int_cnt>=20)
    {
      printk("xilaxitimer_isr: all of the interrupts have occurred. Disabling timer");
      data = ioread32(tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
      iowrite32(data & ~(XIL_AXI_TIMER_CSR_ENABLE_TMR_MASK), tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
    }
  
  return IRQ_HANDLED;
}

//***************************************************
// PROBE AND REMOVE

static int timer_probe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;
  

  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!r_mem) {
    printk(KERN_ALERT "invalid address\n");
    return -ENODEV;
  }
  tp = (struct timer_info *) kmalloc(sizeof(struct timer_info), GFP_KERNEL);
  if (!tp) {
    printk(KERN_ALERT "Cound not allocate timer device\n");
    return -ENOMEM;
  }

  tp->mem_start = r_mem->start;
  tp->mem_end = r_mem->end;
  
  
  if (!request_mem_region(tp->mem_start,tp->mem_end - tp->mem_start + 1, DRIVER_NAME))
  {
    printk(KERN_ALERT "Couldn't lock memory region at %p\n",(void *)tp->mem_start);
    rc = -EBUSY;
    goto error1;
  }
  else {
    printk(KERN_INFO "xilaxitimer_init: Successfully allocated memory region for timer\n");
  }
  /* 
   * Map Physical address to Virtual address
   */
  
  tp->base_addr = ioremap(tp->mem_start, tp->mem_end - tp->mem_start + 1);
  if (!tp->base_addr) {
    printk(KERN_ALERT "led: Could not allocate iomem\n");
    rc = -EIO;
    goto error2;
  }
  // GET IRQ NUM  
  tp->irq_num = platform_get_irq(pdev, 0);
  //printk("irq number is: %d\n", tp->irq_num);
  
  if (request_irq(tp->irq_num, xilaxitimer_isr, 0, DEVICE_NAME, NULL)) {
    printk(KERN_ERR "xilaxitimer_init: Cannot register IRQ %d\n", tp->irq_num);
    return -EIO;
  }
  else {
    printk(KERN_INFO "xilaxitimer_init: Registered IRQ %d\n", tp->irq_num);
  }
  // starting timer, default tick is 1 ms
  setup_and_start_timer(1000);
  printk("probing done");
 error2:
  release_mem_region(tp->mem_start, tp->mem_end - tp->mem_start + 1);
 error1:
  return rc;
  
}

static int timer_remove(struct platform_device *pdev)
{
  unsigned int data;
    /* 
   * Exit Device Module
   */
  data = ioread32(tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  iowrite32(data & ~(XIL_AXI_TIMER_CSR_ENABLE_TMR_MASK),
            tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  iounmap(tp->base_addr);
  release_mem_region(tp->mem_start, tp->mem_end - tp->mem_start + 1);
  free_irq(tp->irq_num, NULL);
  return 0;
}

//***************************************************
// IMPLEMENTATION OF FILE OPERATION FUNCTIONS

static int timer_open(struct inode *i, struct file *f)
{
  printk("opening done");
  return 0;
}
static int timer_close(struct inode *i, struct file *f)
{
    printk("closing done");
    return 0;
}
static ssize_t timer_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk("reding entered");
    return 0;
}
static ssize_t timer_write(struct file *f, const char __user *buf, size_t count,
                           loff_t *off)
{
  char buffer[count];
  unsigned int timer_ms;
  int i = 0;
  printk("writing enetered");
  i = copy_from_user(buffer, buf, count);
  buffer[count - 1] = '\0';
  timer_ms = strToInt(buffer, count, 10);
  if (timer_ms > 40000)
  {
    printk("maximum period exceeded, enter something less than 40000 ");
    return count;
  }
  setup_and_start_timer(timer_ms);
  
  
  return count;
}

//***************************************************
// HELPER FUNCTIONS (STRING TO INTEGER)


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

//***************************************************
//HELPER FUNCTION THAT RESETS AND STARTS TIMER WITH PERIOD IN MILISECONDS


static void setup_and_start_timer(unsigned int milliseconds)
{
  /* 
   * disable Timer Counter
   */
  unsigned int timer_load;
  unsigned int zero = 0;
  unsigned int data;
  timer_load = zero - milliseconds*100000;
  data = ioread32(tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  iowrite32(data & ~(XIL_AXI_TIMER_CSR_ENABLE_TMR_MASK),
            tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
    /* 
   * Set Timer Counter
   */
  iowrite32(timer_load, tp->base_addr + XIL_AXI_TIMER_TLR_OFFSET);
  data = ioread32(tp->base_addr + XIL_AXI_TIMER_TLR_OFFSET);
  printk("xilaxitimer_init: Set timer count 0x%08X\n",data);
  
  /* 
   * Set Timer mode and enable interrupt
   */
  iowrite32(XIL_AXI_TIMER_CSR_LOAD_MASK,
	    tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  iowrite32(XIL_AXI_TIMER_CSR_ENABLE_INT_MASK | XIL_AXI_TIMER_CSR_AUTO_RELOAD_MASK,
	    tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);

  /* 
   * Start Timer
   */
  data = ioread32(tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);
  iowrite32(data | XIL_AXI_TIMER_CSR_ENABLE_TMR_MASK,
	    tp->base_addr + XIL_AXI_TIMER_TCSR_OFFSET);

}

//***************************************************
// INIT AND EXIT FUNCTIONS OF DRIVER

static int __init timer_init(void)
{
  
  int_cnt = 0;
  
  printk(KERN_INFO "xilaxitimer_init: Initialize Module \"%s\"\n", DEVICE_NAME);

  if (alloc_chrdev_region(&first, 0, 1, "Timer_region") < 0)
  {
    printk(KERN_ALERT "<1>Failed CHRDEV!.\n");
    return -1;
  }
  printk(KERN_INFO "Succ CHRDEV!.\n");

  if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
  {
    printk(KERN_ALERT "<1>Failed class create!.\n");
    goto fail_0;
  }
  printk(KERN_INFO "Succ class chardev1 create!.\n");
  if (device_create(cl, NULL, MKDEV(MAJOR(first),0), NULL, "timer") == NULL)
  {
    goto fail_1;
  }

  printk(KERN_INFO "Device created.\n");

  cdev_init(&c_dev, &timer_fops);
  if (cdev_add(&c_dev, first, 1) == -1)
  {
    goto fail_2;
  }

  printk(KERN_INFO "Device init.\n");

  return platform_driver_register(&timer_driver);

 fail_2:
  device_destroy(cl, MKDEV(MAJOR(first),0));
 fail_1:
  class_destroy(cl);
 fail_0:
  unregister_chrdev_region(first, 1);
  return -1;

} 

static void __exit timer_exit(void)  		
{

  platform_driver_unregister(&timer_driver);
  cdev_del(&c_dev);
  device_destroy(cl, MKDEV(MAJOR(first),0));
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk(KERN_ALERT "timer exit.\n");

  printk(KERN_INFO "xilaxitimer_exit: Exit Device Module \"%s\".\n", DEVICE_NAME);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_AUTHOR ("Xilinx");
MODULE_DESCRIPTION("Test Driver for Zynq PL AXI Timer.");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("custom:xilaxitimer");
