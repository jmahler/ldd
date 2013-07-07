NAME
----

hello - hellow world driver

DESCRIPTION
-----------

This is the simplest "Hello, World" Linux driver.
It just prints a message to the logs on load (insmod)
and unload (rmmod).

This example was taken from the [Ubuntu Wiki][ubudkms].

 [ubudkms]: https://wiki.kubuntu.org/Kernel/Dev/DKMSPackaging

As described in the wiki, [dkms][dkms] is used and a [Debian][deb]
package (deb) is built.

 [dkms]: http://linux.dell.com/dkms/
 [deb]: http://www.debian.org

Using DKMS
----------

It is assumed that the kernel source has been installed and is
under /usr/src/.

First copy this project in to a directory under /usr/src/
It is helpful if the directory includes the version number.

The following uses dkms to build and install the module.
Then it is loaded in to the kernel (insmod) and then removed (rmmod).
And finally dkms is used to remove it from the system.

    root# cp -a hello /usr/src/hello-0.1

    root# cd /usr/src/hello-0.1/

    root# dkms add -m hello -v 0.1

    root# dkms build -m hello -v 0.1

    root# dkms install -m hello -v 0.1

    root# modprobe hello

    root# dmesg | tail -1
    [447492.530229] Hello world.

    root# rmmod hello

    root# dmesg | tail -1
    [447552.738466] Goodbye world.

    root# dkms remove -m hello -v 0.1 --all


Debian Package
--------------

Once we know that the module builds and installs using dkms,
we can build a Debian package (deb) to make it even easier to
install on a system.

To build the deb run:

    root# cd /usr/src/hello-0.1/

    root# dkms mkdeb -m hello -v 0.1

A message will indicate where the output was placed.
Typically this is in:

    /var/lib/dkms/hello/0.1/deb/

