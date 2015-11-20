
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>

static int usb_notify_subscriber(struct notifier_block *self,
				unsigned long action, void *unused)
{
	pr_info("%s\n", __func__);

	switch (action) {
	case USB_DEVICE_ADD:
		pr_info("  USB_DEVICE_ADD\n");
		break;
	case USB_DEVICE_REMOVE:
		pr_info("  USB_DEVICE_REMOVE\n");
		break;
	case USB_BUS_ADD:
		pr_info("  USB_BUS_ADD\n");
		break;
	case USB_BUS_REMOVE:
		pr_info("  USB_BUS_REMOVE\n");
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block usb_notify_nb = {
	.notifier_call = usb_notify_subscriber,
};

static int __init usb_notify_init(void)
{
	pr_info("Starting USB Notify Subscriber\n");

	usb_register_notify(&usb_notify_nb);

	return 0;
}

static void __exit usb_notify_exit(void)
{
	pr_debug("Stopping USB Notify Subscriber\n");

	usb_unregister_notify(&usb_notify_nb);
}

module_init(usb_notify_init);
module_exit(usb_notify_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
