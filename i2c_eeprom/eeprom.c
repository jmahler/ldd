/*
 * Reads the eeprom on the i2c bus.
 *
 * This example was derived from the example given on Pg. 271
 * of Essential Linux Device Drivers.  A working example was
 * not given in the book.  And the snippets provided are missing
 * large chunks and have many typos.
 *
 * This code currently does not work.
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/slab.h>

#define DEVICE_NAME "eep"
#define NUM_BANKS 2
#define BANK_SIZE 2048

static unsigned short normal_i2c[] = {
	SLAVE_ADDR1, SLAVE_ADDR2, I2C_CLIENT_END
};

int eep_attach(struct i2c_adapter *adapter, int address, int kind)
{
	static struct i2c_client *eep_client;

	eep_client = kmalloc(sizeof(*eep_client), GFP_KERNEL);

	eep_client->driver = &eep_driver;
	eep_client->addr	= address;
	eep_client->adapter	= adapter;
	eep_client->flags	= 0;
	strlcpy(eep_client->name, DEVICE_NAME, I2C_NAME_SIZE);

	i2c_attach_client(eep_client);
}

static int eep_probe(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, eep_attach);
}

static dev_t dev_number;
static struct class *eep_class;
static struct i2c_driver eep_driver =
{
	.driver = {
		.name = DEVICE_NAME,
	},
	.attach_adapter	= eep_probe,
	.detach_client	= eep_detach,
};

struct eep_bank {
	struct cdev cdev;
	struct i2c_client *client;
	unsigned int addr;
	unsigned short current_pointer;
	int bank_number;
};

struct eep_bank *eep_bank_list;

static int eep_open(struct inode *inode, struct file *file)
{
	unsigned int n;

	n = MINOR(file->f_dentry->d_inode->i_rdev);

	file->private_data = &eep_bank_list[n];

	return 0;
}

static int eep_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t eep_read(struct file *file, char *buf,
							size_t count, loff_t *ppos)
{
	int i;
	int transferred;
	int ret;
	int my_buf[BANK_SIZE];
	struct ee_bank *my_bank = (struct ee_bank *) file->private_data;

	if (i2c_check_functionality(my_bank->client,
						I2C_FUNC_SMBUS_READ_WORD_DATA)) {

		while (transferred < count) {
			ret = i2c_smbus_read_word_data(my_bank->client,
											my_bank->current_pointer+i);
			my_buf[i++] = (u8) (ret & 0xFF);
			my_buf[i++] = (u8) (ret >> 8);
			transferred += 2;
		}

		copy_to_user(buffer, (void *) my_buf, transferred);
		my_bank->current_pointer += transferred;
	}

	return transferred;
}

static struct file_operations eep_fops = {
	.owner 		= THIS_MODULE,
	.open		= eep_open,
	.release	= eep_release,
//	.read 		= eep_read,
//	.ioctl		= eep_ioctl,
//	.llseek 	= eep_llseek,
//	.write		= eep_write,
};

static int __init eep_init(void)
{
	int err, i;

	eep_bank_list = kmalloc(sizeof(struct eep_bank)*NUM_BANKS, GFP_KERNEL);
	memset(eep_bank_list, 0, sizeof(struct eep_bank)*NUM_BANKS);

	if (alloc_chrdev_region(&dev_number, 0, NUM_BANKS, DEVICE_NAME) < 0) {
		printk(KERN_DEBUG "Can't register device\n");
		return -1;
	}

	eep_class = class_create(THIS_MODULE, DEVICE_NAME);
	for (i = 0; i < NUM_BANKS; i++) {
		cdev_init(&eep_bank_list[i].cdev, &eep_fops);

		if (cdev_add(&eep_bank_list[i].cdev, (dev_number + i), i)) {
			printk("Bank kmalloc\n");
			return 1;
		}
		device_create(eep_class, NULL, MKDEV(MAJOR(dev_number), i),
						NULL, "eeprom%d", i);
	}

	err = i2c_add_driver(&eep_driver);
	if (err) {
		printk("Registering I2C driver failed, errno is %d\n", err);
		return err;
	}

	printk("EEPROM Driver Initialized.\n");

	return 0;
}

static void __exit eep_exit(void)
{
}

module_init(eep_init);
module_exit(eep_exit);
