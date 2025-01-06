/*****************************************************************************/
/*                                                                           */
/*   USB-DCF77-Clock                                                         */
/*   USB requests                                                            */
/*                                                                           */
/*   (c) 2009 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik               */
/*                                                                           */
/*   WinAVR/GCC 20090313                                                     */
/*   Created: 2009-07-18                                                     */
/*   Changed: 2009-08-05                                                     */
/*                                                                           */
/*****************************************************************************/

//-----------------------------------------------------------------------------
// Supported USB Requests
//-----------------------------------------------------------------------------
#define USB_GET_STATUS				0x01	// returns device state (STATUS_BUSY, if busy)
#define USB_GET_DEVICEINFO		0x02	// returns device informationen
#define	USB_GET_SERIALNUMBER	0x03	// returns USB serial number (from EEPROM)
#define USB_GET_DCFSTATUS			0x04	// returns device state
#define USB_GET_TIMEDATA			0x05	// request time informationen
#define USB_GET_DCFQUALITY		0x06	// request DCF signal quality
#define USB_GET_CFG						0x07	// returns configuration byte from EEPROM
#define	USB_GET_BATVOLTAGE		0x08	// returns RTC battery voltage
#define USB_GET_TIMECHANGE		0x09	// returns 1, if time has changed since last request

#define	USB_SET_SERIALNUMBER	0x83	// write USB serial number (into EEPROM) (requires restart)
#define USB_SET_CFG						0x87	// write configuration byte into EEPROM (requires restart)
#define	USB_SET_MEASUREBAT		0x88	// initiate battery voltage measure now
#define USB_NONE							0xFF	// nothing (internal used)


//-----------------------------------------------------------------------------
// device status
//-----------------------------------------------------------------------------
#define DCFSTATUS_NOSIGNAL 	0				// no DCF signal
#define DCFSTATUS_WAITSTART	1				// DCF signal present, waiting for minute start
#define DCFSTATUS_GETDATA	2				// DCF data will be logged, DCF time not available
#define DCFSTATUS_TIMEAVAIL	3				// DCF time available

//-----------------------------------------------------------------------------
// device status
//-----------------------------------------------------------------------------
#define STATUS_BUSY 	0
#define STATUS_READY	1


