

usbdcf77re is a Linux daemon which query the current DCF77 time from the following USB device:

https://www.reusch-elektronik.de/re/en/products/usbdcf77/index.htm

and writes the current time to a shared memory (SHM) for ntpd or chrony.

For installation instructions see install.txt


For SHM driver support for ntpd see:

https://www.ntp.org/documentation/drivers/driver28/

SHM driver support from ntpsec:

https://docs.ntpsec.org/latest/driver_shm.html

SHM driver for chrony see "SHM" section:

https://chrony-project.org/doc/4.6.1/chrony.conf.html


The usbdcf77re daemon was successfully tested on Fedora Linux with chrony.

Sample output:


$ chronyc -n sources
MS Name/IP address         Stratum Poll Reach LastRx Last sample               
===============================================================================
#* NMEA                          0   4   377    20   -212us[ -295us] +/-  133us
#? DCF                           0   6   252   128  +3113us[+2971us] +/- 2793us
^- 194.x.y.z                     1  10   377   209  +3674us[+3366us] +/-   14ms
^- 2001:axa:b::ccc               2  10   377   583  +8014us[+2062us] +/- 8048us
^- 2001:axxa:0:bbb:0:ccc:1:1     1  10   377   712    +17ms[  +11ms] +/-   20ms


usbdcf77re is using the following USB library:

https://libusb.info/

This library is available for several Operating Systems.


Restrictions:
This works only with one device connected. If multiple are present, the first
found is used.


The device has an internal RTC, it is regulary updated from the DCF77 time source.
If the device receives no DCF77 signal over a longer period of time (several hours)
the internal RTC time may be out of sync.





