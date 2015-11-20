
NAME
----

usb_notify - Subscribe to the USB notifier chain and display events.

DESCRIPTION
-----------

The `usb_notify` module shows how to use the Linux notifier chain that
is part of the USB system.  It subscribes to add/remove events for both
devices and busses and it will print a message to the logs when a
notification occurs.

    ~/ldd/usb_notify$ make
    (module should build without any errors)
    ~/ldd/usb_notify$ sudo su
    ~/ldd/usb_notify# insmod usb_notify.ko
      ...
      Nov 20 11:28:52 newt kernel: Starting USB Notify Subscriber
      ...
    (plug in some usb device)
      ...
      Nov 20 11:29:36 newt kernel: USB_DEVICE_ADD
      ...
    (remove the usb device)
      ...
      Nov 20 11:30:13 newt kernel: USB_DEVICE_REMOVE
      ...
    (remove a bus by unbinding it)
    ~/ldd/usb_notify# echo "0000:00:1d.0" > /sys/bus/pci/drivers/ehci-pci/unbind 
      ...
      Nov 20 11:32:39 newt kernel: ehci-pci 0000:00:1d.0: remove, state 4
      Nov 20 11:32:39 newt kernel: usb usb1: USB disconnect, device number 1
      Nov 20 11:32:39 newt kernel: usb 1-1: USB disconnect, device number 2
      Nov 20 11:32:39 newt kernel: USB_DEVICE_REMOVE
      Nov 20 11:32:39 newt kernel: USB_DEVICE_REMOVE
      Nov 20 11:32:39 newt kernel: ehci-pci 0000:00:1d.0: USB bus 1 deregistered
      Nov 20 11:32:39 newt kernel: USB_BUS_REMOVE
      ...
    (re-add the bus by binding it)
    ~/ldd/usb_notify# echo "0000:00:1d.0" > /sys/bus/pci/drivers/ehci-pci/bind 
      ...
      Nov 20 11:38:56 newt kernel: USB_BUSS_ADD
      Nov 20 11:38:56 newt kernel: ehci-pci 0000:00:1d.0: new USB bus
      registered, assigned bus number 1
      Nov 20 11:38:56 newt kernel: ehci-pci 0000:00:1d.0: debug port 2
      Nov 20 11:38:56 newt kernel: ehci-pci 0000:00:1d.0: cache line size of
      64 is not supported
      ...
    ~/ldd/usb_notify# rmmod usb_notify.ko

REFERENCES
----------

This example is based upon the example given on Page 65 of the book
"Bootstrap Yourself with Linux-USB" by Rajaram Regupathy (2012).

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>

COPYRIGHT
---------

Copyright &copy; 2015, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

