
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

static const struct usb_device_id hello_table[] = {
	/*{.driver_info = 42}, */ /* call for every usb device */
	{ USB_DEVICE(0x046e, 0x6000) },
	{ USB_DEVICE(0x04f3, 0x0801) },
	{ USB_DEVICE(0x0403, 0xffa8) },
	{ }
};
MODULE_DEVICE_TABLE(usb, hello_table);

static int hello_probe(struct usb_interface *intf,
						const struct usb_device_id *id)
{
	pr_debug("usb probe: Hello, World\n");

	return 0;  /* yes, that's our device */
}

static void hello_disconnect(struct usb_interface *intf)
{
	pr_debug("usb disconnect: Goodbye, cruel world\n");
}

static struct usb_driver hello_driver = {
	.name		= "hello",
	.id_table	= hello_table,
	.probe		= hello_probe,
	.disconnect	= hello_disconnect,
};

static int __init hello_init(void)
{
	int result;

	pr_debug("init: Hello, World\n");

	result = usb_register(&hello_driver);
	if (result)
		pr_err("usb_register failed. Error number %d", result);

	return result;
}

static void __exit hello_exit(void)
{
	pr_debug("exit: Goodbye, cruel world\n");
	usb_deregister(&hello_driver);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

