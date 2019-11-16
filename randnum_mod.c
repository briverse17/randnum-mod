#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/uaccess.h>

#define MAXRANGE 1000000

static dev_t dev_num; // Global variable for device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class
int range = MAXRANGE; //Range to generate random numbers: (-range, range)

//get number of digits
static size_t number_len(int num)
{
	size_t len;
	int n;
	n = abs(num);
	len = 0;

	if (num == 0)
		return 1;

	while (n != 0)
	{
		len++;
		n /= 10;
	}

	if (num < 0)
		return len + 1;
	return len;
}

//simple power function
static int mypow(int base, int exp)
{
	int i, res;
	
	if (exp == 0)
		return 1;

	res = 1;
	for (i = exp; i > 0; i--)
		res *= base;

	return res;
}

//convert from int to char* - equivalent to string::itoa()
static void number_to_str(int num, char* str)
{	
	int rem, len, i, ch, end;
	int n;

	len = number_len(num);
	
	if (len == 1)
	{
		ch = num + 48;
		str[0] = (char)ch;
		str[len] = '\0';
		return;
	}

	n = abs(num);

	end = len;	
	if (num < 0)
	{
		str[0] = '-';
		end = len - 1;
	}

	for (i = 0; i < end; i++)
	{
		rem = n % 10;
		n /= 10;
		ch = rem + 48;
		str[len - (i + 1)] = (char)ch;
	}

	str[len] = '\0';
}

//convert from char* to int - equivalent to string::atoi()
static void str_to_number(int *num, char* str)
{
	int len, start, end, i;

	len = strlen(str);
	end = len - 1;
	start = 0;
	if (str[0] == '-')
		start = 1;

	for (i = start; i < len; i++)
	{
		*num += ((int)str[i] - 48) * mypow(10, end - i);
	}

	if (str[0] == '-')
		*num = 0 - *num;
}

//will be assigned to syscall "open"
static int randnum_open(struct inode *i, struct file *f)
{
	printk("Driver: open()\n");
	return 0;
}

//will be assigned to syscall "release"
static int randnum_close(struct inode *i, struct file *f)
{
	printk("Driver: close()\n");
	return 0;
}

//will be assigned to syscall "read"
static ssize_t randnum_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	char* data;
	int num;
	
	printk("Driver: read()\n");

	get_random_bytes(&num, sizeof(int));
	num %= range;
	len = number_len(num);
	data = kmalloc(len * sizeof(char), GFP_KERNEL);

	if (data == NULL)
	{
		printk("Cannot generate random number!\n");
		return -EFAULT;
	}
	else
	{
		number_to_str(num, data);
		printk("random num = %s\n", data);
	}

	if (copy_to_user(buf, data, len))
		return -EFAULT;

	buf[len] = '\0';

	kfree(data);


	return len;
}

//will be assigned to syscall "write"
//in this program, its specific task is to change the range of generation
static ssize_t randnum_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	char* data;
	int* num;

	printk("Driver: write()\n");
	
	data = kmalloc(len * sizeof(char) + 1, GFP_KERNEL);
	num = kmalloc(sizeof(int), GFP_KERNEL);
	*num = 0;

	if (data == NULL)
	{
		printk("Cannot reset range! (max = 1000000)\n");
		return -EFAULT;
	}
	else
	{
		if (copy_from_user(data, buf, len))
			return -EFAULT;
		str_to_number(num, data);
		if (*num < 0 || *num > MAXRANGE)
		{
			printk("Cannot reset range! (max = 1000000)\n");
			return -EFAULT;
		}
		range = *num;
		printk("new range = %d\n", range);
	}

	kfree(num);
	kfree(data);

	return len;
}

//assign the above entry points to linux's syscalls
static struct file_operations pugs_fops =
{
	.owner = THIS_MODULE,
	.open = randnum_open,
	.release = randnum_close,
	.read = randnum_read,
	.write = randnum_write
};

//initialize the module
static int __init randnum_init(void)
{
	printk(KERN_INFO "Randnum device: Registered\n");
	if (alloc_chrdev_region(&dev_num, 0, 1, "randnum_mod") < 0)
	{
		return -1;
	}
	if ((cl = class_create(THIS_MODULE, "randnum_drv")) == NULL)
	{
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}
	if (device_create(cl, NULL, dev_num, NULL, "randnum_dev") == NULL)
	{
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
	return -1;
	}
	cdev_init(&c_dev, &pugs_fops);
	if (cdev_add(&c_dev, dev_num, 1) == -1)
	{
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}
	return 0;
}

//exit the module
static void __exit randnum_exit(void)
{
	printk(KERN_INFO "Randnum device: Unregistered\n");
	cdev_del(&c_dev);
	device_destroy(cl, dev_num);
	class_destroy(cl);
	unregister_chrdev_region(dev_num, 1);
}

module_init(randnum_init);
module_exit(randnum_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Minh Vu <vumnguyenn@gmail.com>");
MODULE_DESCRIPTION("Random Number Character Driver");
