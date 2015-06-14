
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hid.h>

static const struct usb_device_id hello_table[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
			USB_INTERFACE_SUBCLASS_BOOT,
			USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ }
};
MODULE_DEVICE_TABLE(usb, hello_table);

static int hello_probe(struct usb_interface *intf,
						const struct usb_device_id *id)
{
	pr_debug("Hello, World\n");

	return 0;  /* yes, that's our device */
}

static void hello_disconnect(struct usb_interface *intf)
{
	pr_debug("Goodbye, cruel world\n");
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
