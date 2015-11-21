
NAME
----

hello-usb-keyboard - Hello world kernel module for a usb keyboard

DESCRIPTION
-----------

The `hello` module simply prints a message when a usb keyboard is
plugged in or disconnected.

    $ make
     (compiles to produce hello.ko)
    # insmod hello.ko
    (plug in usb device)
     "Hello, World" in log
    (disconnect usb device)
     another "Goodbye, cruel world" in log
    # rmmod hello

MODULE LOADING
--------------

This module can be loaded in several different ways.  The first is the
manual method.  The module is compiled out of tree and can be loaded
with `insmod`.

The second method allows `modprobe` to be used.  The module is still
compiled out of tree but it is then placed in the tree.  A place such as
extra is a good choice.

    # cp hello.ko /lib/modules/3.15.0-rc7-00058-gf2159d1/kernel/drivers/extra/

Then `depmod` is run to have it update its modules.dep files for the
current kernel.

    # depmod

Now `modprobe` can be used.

    # modprobe hello

The third method supports all the previous methods as well making it
possible for the kernel to auto load the module.  In this case the module
is placed in the tree and then the whole kernel is re-compiled.  The
location `drivers/usb/misc` is a good place.

    # cp hello.c linux/drivers/usb/misc/

An entry will have to be added to the makefile for this new source file.
Also, the -DDEBUG option must be added in order for `pr_debug()` to work.

    diff --git a/drivers/usb/misc/Makefile b/drivers/usb/misc/Makefile
    index e748fd5..4c81ce7 100644
    --- a/drivers/usb/misc/Makefile
    +++ b/drivers/usb/misc/Makefile
    @@ -2,6 +2,9 @@
     # Makefile for the rest of the USB drivers
     # (the ones that don't fit into any other categories)
     #
    +ccflags-y += -DDEBUG
    +
    +obj-m                                                  += hello.o
     obj-$(CONFIG_USB_ADUTUX)               += adutux.o
     obj-$(CONFIG_USB_APPLEDISPLAY)         += appledisplay.o
     obj-$(CONFIG_USB_CYPRESS_CY7C63)       += cypress_cy7c63.o
    jeri@hudson:~/linux/drivers$

After the kernel is rebuilt and rebooted the kernel will autoload the
module any time a matching usb device is plugged in.

MODULE PROBE
------------

This module will print "Hello, World" when a usb keyboard is connected and
the probe function is called.  However, if probe is called for another
driver, and it asserts that it is the correct device, the probe function
for the hello module will not be called.  To ensure that the hello probe
function is called, other drivers may need to be blacklisted.

    # /etc/modprobe.d/hello-blacklist.conf
    blacklist hid_generic
    blacklist usbhid
    blacklist hid

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>

