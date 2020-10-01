#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#include <asm/io.h>

#include "define.h"

#define DEVICE_NAME  "gpio_test"
#define NR_DEVICES   1



static int major = 0;
module_param(major, int, 0);

static int minor = 0;
module_param(minor, int, 0);

static unsigned long gpio_base =  GPIO_BASE;
static int nr_devices = NR_DEVICES;
static struct gpio_test_dev *device;




static ssize_t gpio_read(struct file *filp,
	                             char __user *buf, size_t count, loff_t *f_pos)
{
int retval;
struct gpio_test_dev *dev = filp->private_data;
void __iomem *io_addr;
unsigned long addr = dev->base + *f_pos;
unsigned char *kbuf, *ptr;


	mutex_lock(&dev->mutex);
	if (addr + count > GPIO_BASE + MEM_SIZE)
		count = (GPIO_BASE + MEM_SIZE) - addr;

	if (count < 0)
		return 0;

	kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	ptr = kbuf;
	retval = count;

	io_addr = (void __iomem *)(dev->io_base + (addr - dev->base));

	while (count) {
		*(u32 *)ptr = ioread32(io_addr);
		io_addr += 4;
		count   -= 4;
		ptr     += 4;
	}

	if ((retval > 0) && copy_to_user(buf, kbuf, retval))
		retval = -EFAULT;

	kfree(kbuf);

	*f_pos += retval;
	mutex_unlock(&dev->mutex);

	return retval;
}


static int my_atoi(char *str)
{
int num = 0;

    while(*str != '\0'){
        if ( *str < 48 || *str > 57 ) {
            break;
        }
        num += *str - 48;
        num *= 10;
        str++;
    }

    num /= 10;
    return num;
}


enum gpio_modes {GPIO_IN=0, GPIO_OUT};
enum gpio_pin   {OFF=0, ON};

static ssize_t gpio_write(struct file *filp,
	                       const char __user *buf, size_t count, loff_t *f_pos)
{
struct gpio_test_dev *dev = filp->private_data;
unsigned char *kbuf;
void __iomem *io_addr;
int mode, outval, pin;
int retval;


	mutex_lock(&dev->mutex);

	kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;
	if (copy_from_user(kbuf, buf, count))
		return -EFAULT;

	//gpio_pin_mode
	switch (kbuf[0]) {
	case '0' :
		mode = GPIO_OUT;
		break;

	case '1' :
		mode = GPIO_IN;
		break;

	default :
		retval = -EINVAL;
		goto fail;
	}

	//gpio_pin_status
	switch (kbuf[1]) {
	case '0' :
		outval = OFF;
		break;

	case '1' :
		outval = ON;
		break;

	default  :
		retval = -EINVAL;
		goto fail;
	}

	//pin number
	pin = my_atoi(&kbuf[2]);
	if (pin < 0 || pin > 57) {
		retval = -EINVAL;
		goto fail;
	}


	switch (pin) {
	case  0 ... 9:
		io_addr = (void __iomem *)(dev->io_base + GPFSEL0);
		iowrite32(1 << (3 * pin), io_addr);
		break;

	case 10 ... 19 :
		io_addr = (void __iomem *)(dev->io_base + GPFSEL1);
		iowrite32(1 << (3 * (pin - 10)), io_addr);
		break;

	case 20 ... 29 :
		io_addr = (void __iomem *)(dev->io_base + GPFSEL2);
		iowrite32(1 << (3 * (pin - 20)), io_addr);
		break;

	case 30 ... 39 :
		io_addr = (void __iomem *)(dev->io_base + GPFSEL3);
		iowrite32(1 << (3 * (pin - 30)), io_addr);
		break;

	case 40 ... 49 :
		io_addr = (void __iomem *)(dev->io_base + GPFSEL4);
		iowrite32(1 << (3 * (pin - 40)), io_addr);
		break;

	case 50 ... 57 :
		io_addr = (void __iomem *)(dev->io_base + GPFSEL5);
		iowrite32(1 << (3 * (pin - 50)), io_addr);
		break;
	}


	switch (pin) {
	case 0 ... 31:
		if (outval == ON) {
			io_addr = (void __iomem *)(dev->io_base + GPFSET0);
			iowrite32(1 << pin, io_addr);
		}
		else if (outval == OFF) {
			io_addr = (void __iomem *)(dev->io_base + GPCLR0);
			iowrite32(1 << pin, io_addr);
		}
		break;

	case 32 ... 57:
		if (outval == ON) {
			io_addr = (void __iomem *)(dev->io_base + GPFSET1);
			iowrite32(1 << (pin - 31), io_addr);
		}
		else if (outval == OFF) {
			io_addr = (void __iomem *)(dev->io_base + GPCLR1);
			iowrite32(1 << (pin - 31), io_addr);
		}
		break;
	}

	retval = count;


  fail:
	kfree(kbuf);
	mutex_unlock(&dev->mutex);
	return retval;
}



static int gpio_open(struct inode *inode, struct file *filp)
{
struct gpio_test_dev *dev;

	dev = container_of(inode->i_cdev, struct gpio_test_dev, cdev);

	mutex_lock(&dev->mutex);
	dev->io_base = (unsigned long)ioremap_nocache(gpio_base, MEM_SIZE);
	if (!dev->io_base)
		return -ENOMEM;

	filp->private_data = dev;
	mutex_unlock(&dev->mutex);

	return 0;
}



static int gpio_release(struct inode *inode, struct file *filp)
{
struct gpio_test_dev *dev;

	dev = filp->private_data;

	mutex_lock(&dev->mutex);
	iounmap((void *)dev->io_base);
	dev->io_base = 0;

	mutex_unlock(&dev->mutex);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
struct file_operations gpio_fops = {
	.owner   = THIS_MODULE,
	.open    = gpio_open,
	.release = gpio_release,
	.read    = gpio_read,
	.write   = gpio_write,
};
////////////////////////////////////////////////////////////////////////////////


static void gpio_cleanup(void)
{
dev_t devno = MKDEV(major, minor);

	if (device) {
		cdev_del(&device->cdev);
		kfree(device);
	}

	unregister_chrdev_region(devno, nr_devices);
}


static void setup_cdev(struct gpio_test_dev *dev)
{
int err, devno = MKDEV(major, minor);

	cdev_init(&dev->cdev, &gpio_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &gpio_fops;
	err = cdev_add(&dev->cdev, devno, 1);

	if (err)
		printk(KERN_INFO "gpio_test : error add cdev %d\n", err);
}


static int __init gpio_init(void)
{

int     result;
dev_t   devno = 0;


	if (major) {
		devno = MKDEV(major, minor);
		result = register_chrdev_region(devno, nr_devices, DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&devno, minor, nr_devices, DEVICE_NAME);
		major = MAJOR(devno);
	}
	if (result < 0) {
		printk(KERN_INFO "cannot get major num\n");
		return result;
	}


	device = kmalloc(sizeof(struct gpio_test_dev), GFP_KERNEL);
	if (!device) {
		result = -ENOMEM;
		goto fail;
	}
	memset(device, 0, sizeof(struct gpio_test_dev));


	setup_cdev(device);
	mutex_init(&device->mutex);
	device->base = (unsigned long)gpio_base;
	device->io_base = 0;

    return 0;

  fail:
    gpio_cleanup();
	return result;
}


module_init(gpio_init);
module_exit(gpio_cleanup);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("kohei-wada");
