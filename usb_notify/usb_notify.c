
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>

static int usb_notify_dev_subscriber(struct notifier_block *self,
				unsigned long action, void *_dev)
{
	struct usb_device *udev = _dev;

	switch (action) {
	case USB_DEVICE_ADD:
		dev_info(&udev->dev, "USB_DEVICE_ADD\n");
		break;
	case USB_DEVICE_REMOVE:
		dev_info(&udev->dev, "USB_DEVICE_REMOVE\n");
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block usb_notify_dev_nb = {
	.notifier_call = usb_notify_dev_subscriber,
};

static int usb_notify_bus_subscriber(struct notifier_block *self,
				unsigned long action, void *_dev)
{
	struct usb_bus *ubus = _dev;

	switch (action) {
	case USB_BUS_ADD:
		dev_info(ubus->controller, "USB_BUS_ADD, %d\n", ubus->busnum);
		break;
	case USB_BUS_REMOVE:
		dev_info(ubus->controller, "USB_BUS_REMOVE, %d\n", ubus->busnum);
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block usb_notify_bus_nb = {
	.notifier_call = usb_notify_bus_subscriber,
};

static int __init usb_notify_init(void)
{
	pr_info("Starting USB Notify Subscriber\n");

	usb_register_notify(&usb_notify_dev_nb);
	usb_register_notify(&usb_notify_bus_nb);

	return 0;
}

static void __exit usb_notify_exit(void)
{
	pr_debug("Stopping USB Notify Subscriber\n");

	usb_unregister_notify(&usb_notify_dev_nb);
	usb_unregister_notify(&usb_notify_bus_nb);
}

module_init(usb_notify_init);
module_exit(usb_notify_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
