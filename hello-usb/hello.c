
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

module_usb_driver(hello_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

