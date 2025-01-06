
usbdcf77re is a daemon which reads the current time from the following device:

https://www.reusch-elektronik.de/re/en/products/usbdcf77/index.htm

and writes the time to a shared memory segment for chrony, ntpd or ntpsec.

Currently it was tested on Fedora Linux 41 and chrony.

The libusb https://libusb.info/ library is used to query the device.

License: GPL-2.0

