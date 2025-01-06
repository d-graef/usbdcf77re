/*

License : GPL-2.0

https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

(c) 2024 Detlef Graef
(c) 2024 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik


file: usbdcf77re.h

*/


int bcd_decimal(uint8_t hex);
int extract_dcf77_status_bits (uint8_t status_bits, t_status_bits *dcf77_status_bits_l);
int detect_sec_change_loop (const int delay, libusb_device_handle *pdcf77_l, int*m);
int get_rct_time_change();
int detect_rtc_time_change_loop(const int delay, libusb_device_handle *pdcf77_rtc, FILE *logfile_handle);

/* ----------------------------------------------------------------------
		show usage of usbdcf77re
---------------------------------------------------------------------- */

void usage(const char *usbdcf77re_version)
{

	printf("\tusbdcf77re version: %s", usbdcf77re_version);
	printf("\t (c) 2024 by Detlef Graef & Reusch Elektronik\n");
	printf("\t-h help\n");
	printf("\t-v print version\n");
	printf("\t-d <shmid> run as daemon; write data to SHM for ntpd, ntpsec, chrony\n");
	printf("\t-i info; get some info from te device\n");
	printf("\t-t show time from device\n");
	printf("\t-l some additional logging\n");

}


/* ----------------------------------------------------------------------
		get EEPROM serial number from device
---------------------------------------------------------------------- */

int get_eeprom_sn(libusb_device_handle *pdcf77)
{

	int status = -1;
	int exitCode = 0;
	unsigned char iodata[9];
	iodata[8] = 0;		// setting null terminated string at position 8 (0..7 serial number)

//		try to get serial number from EEPROM

	printf("Trying to get EEPROM serial number...\n");

	status = libusb_control_transfer(pdcf77, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
									USB_GET_SERIALNUMBER, 0, 0, iodata, SNLEN, USB_TIMEOUT);

	if (status >= 0) { 
			printf("Returned EEPROM serial number: %s\n", iodata);
	} else {
		const char* error_name = libusb_error_name(status);
		printf("Error: failed with error code: %s\n", error_name);
		printf("Error: failed with error code: %i\n", status);
		exitCode = -1;
	}
	return exitCode;

}

/* ----------------------------------------------------------------------
		get USB serial number from device
---------------------------------------------------------------------- */

int get_usb_sn(libusb_device_handle *pdcf77) {

	int status = -1;
	int exitCode = 0;
	unsigned char buf[256];
	buf[0]=0;

//		try to get serial number string

	status = libusb_get_string_descriptor_ascii(pdcf77, 3, buf, sizeof buf);

	if (status >= 0) { 
			printf("Returned string (serial number): %s\n", buf);
	} else {
		printf("Error: failed with error code: %i\n", status);
		exitCode = -1;
	}
	return exitCode;

}

/* ----------------------------------------------------------------------
		get manufacturer string from device
---------------------------------------------------------------------- */

int get_usb_manufacturer(libusb_device_handle *pdcf77) {

	int status = -1;
	int exitCode = 0;
	unsigned char buf[256];
	buf[0]=0;


//		try to get manufacturer
	status = -1;
	printf("Trying to get manufacurer...\n");
	buf[0]=0;

	status = libusb_get_string_descriptor_ascii(pdcf77, 1, buf, sizeof buf);		
	if (status >= 0) { 
		printf("Returned string (manufacturer): %s\n", buf);
	} else {
		printf("Error: failed with error code: %i\n", status);
		exitCode = -1;
	}
		return exitCode;

}

/* ----------------------------------------------------------------------
		get product string from device
---------------------------------------------------------------------- */

int get_usb_product(libusb_device_handle *pdcf77) {

	int status = -1;
	int exitCode = 0;
	unsigned char buf[256];
	buf[0]=0;

//		try to get product string
	status = -1;
	printf("Trying to get product string...\n");
	buf[0]=0;

	status = libusb_get_string_descriptor_ascii(pdcf77, 2, buf, sizeof buf);
	if (status >= 0) { 
			printf("Returned string (product): %s\n", buf);
	} else {
			printf("Error: failed with error code: %i\n", status);
			exitCode = -1;
	}
	return exitCode;

}

/* ----------------------------------------------------------------------
		get DCF77 status from device
---------------------------------------------------------------------- */

int get_dcf77_status(libusb_device_handle *pdcf77) {

	int status = -1;
	int exitCode = -1;
	unsigned char iodata_dcf77_status[8];
	int dcf77status = -1;

//		try to get DCF77 status

	status = libusb_control_transfer(pdcf77, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
									USB_GET_DCFSTATUS, 0, 0, iodata_dcf77_status, 1, USB_TIMEOUT);

	if (status >= 0) {
			dcf77status = iodata_dcf77_status[0];
//			returns:0=no signal, 1=waiting for minute start, 2=receiving data, 3=DCF time available
	} else {
			const char* error_name = libusb_error_name(status);
			printf("Error: failed with error code: %s\n", error_name);
			printf("Error: failed with error code: %i\n", status);
			exitCode = -1;
			return exitCode;
	}
		return dcf77status;
}

/* ----------------------------------------------------------------------
		get DCF77 quality from device
---------------------------------------------------------------------- */

int get_dcf77_quality(libusb_device_handle *pdcf77) {

	int status = -1;
	int exitCode = 0;
	status = -1;
	unsigned char iodata_dcf77_quality[8];
	int dcf77quality = 0;

//		try to get DCF77 signal quality

	status = libusb_control_transfer(pdcf77, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
									USB_GET_DCFQUALITY, 0, 0, iodata_dcf77_quality, 4, USB_TIMEOUT);

	if (status >= 0) { 
			memcpy(&dcf77quality, &iodata_dcf77_quality, sizeof(dcf77quality));
			printf("DCF77 quality: %i\n", dcf77quality);
		} else {
			const char* error_name = libusb_error_name(status);
			printf("Error: failed with error code: %s\n", error_name);
			printf("Error: failed with error code: %i\n", status);
			exitCode = -1;
		}
		return exitCode;
}

/* ----------------------------------------------------------------------
		get device info
---------------------------------------------------------------------- */

int get_dcf77_dev_info(libusb_device_handle *pdcf77) {

	int status = -1;
	int exitCode = 0;

//		try to get device info

	printf("Trying to get device information...\n");
	status = -1;
	unsigned char iodata_devinfo[5];

	status = libusb_control_transfer(pdcf77, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
									USB_GET_DEVICEINFO, 0, 0, iodata_devinfo, 5, USB_TIMEOUT);

	if (status >= 0) { 
			printf("Returned devinfo version : %i\n", iodata_devinfo[0]);
			printf("Returned devinfo built : %i\n", iodata_devinfo[2]);
			printf("Returned devinfo type : %i\n", iodata_devinfo[3]);
			printf("Returned devinfo mode : %i\n", iodata_devinfo[4]);						
		} else {
			const char* error_name = libusb_error_name(status);
			printf("Error: failed with error code: %s\n", error_name);
			printf("Error: failed with error code: %i\n", status);
			exitCode = -1;
		}
	return exitCode;
	
}

/* ----------------------------------------------------------------------
		get current time from device
---------------------------------------------------------------------- */

int get_dcf77_current_time(libusb_device_handle *pdcf77, t_dcf77_time_data *dcf77_time, t_status_bits *dcf77_status_bits) {

	int status = -1;
	int exitCode = 0;

//		try to get time from device

	status = -1;
	unsigned char iodata_time[8];

	status = libusb_control_transfer(pdcf77, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
									USB_GET_TIMEDATA, 0, 0, iodata_time, 8, USB_TIMEOUT);

	if (status >= 0) { 
			dcf77_time->year = 0;
			dcf77_time->month = 0;
			dcf77_time->day = 0;
			dcf77_time->hour = 0;
			dcf77_time->minute = 0;
			dcf77_time->second = 0;
			dcf77_time->day_of_week = 0;
			dcf77_time->status = 0;

			dcf77_time->year = bcd_decimal(iodata_time[0]) + 2000;
			dcf77_time->month = bcd_decimal(iodata_time[1]);
			dcf77_time->day = bcd_decimal(iodata_time[2]);
			dcf77_time->hour = bcd_decimal(iodata_time[3]);
			dcf77_time->minute = bcd_decimal(iodata_time[4]);
			dcf77_time->second = bcd_decimal(iodata_time[5]);
			dcf77_time->day_of_week = bcd_decimal(iodata_time[6]);
			dcf77_time->status = iodata_time[7];
			extract_dcf77_status_bits(dcf77_time->status, dcf77_status_bits);	// extract the DCF77 status bits
		} else {
			const char* error_name = libusb_error_name(status);
			printf("Error: failed with error code: %s\n", error_name);
			printf("Error: failed with error code: %i\n", status);
			exitCode = -1;
		}

	return exitCode;
}


/* ----------------------------------------------------------------------
		maybe for future use
---------------------------------------------------------------------- */

int get_dcf77_time_change(libusb_device_handle *pdcf77, FILE *logfile_handle)
{

	int exitCode = 0;
	const int poll_200ms = 200000;		// in us
	const int poll_50ms = 50000;		// in us
	const int poll_5ms = 5000;			// in us
	int m1 = 0;
	int m2 = 0;
	int m3 = 0;
	int loop_count = 0;				// passed to subroutine

	m1 = detect_sec_change_loop(poll_200ms, pdcf77, &loop_count);
	if (m1 == 23) {
		fprintf(logfile_handle, "dcf77 %i us loop count (fallback to RTC): %i\n", poll_200ms, loop_count);
		return 23;			// fallback to rtc
	}
	fprintf(logfile_handle, "dcf77 %i us loop count (success): %i ", poll_200ms, loop_count);
	usleep(700000);					// 1000 - 800 - 2 * 50 ms

	m2 = detect_sec_change_loop(poll_50ms, pdcf77, &loop_count);
	if (m2 == 23) {
		fprintf(logfile_handle, "dcf77 %i us loop count (fallback to RTC): %i ", poll_50ms, loop_count);
		return 23;			// fallback to RTC
	}
	fprintf(logfile_handle, "dcf77 %i us loop count (success): %i ", poll_50ms, loop_count);
	usleep(880000);					// 900 ms is sometimes to long

	m3 = detect_sec_change_loop(poll_5ms, pdcf77, &loop_count);
	if (m3 == 23) {
		fprintf(logfile_handle, "dcf77 %i us loop count (fallback to RTC): %i\n", poll_5ms, loop_count);
		return 23;			// fallback to rtc
	}
	fprintf(logfile_handle, "dcf77 %i us loop count (success): %i\n", poll_5ms, loop_count);

	return exitCode;
}



/* ----------------------------------------------------------------------
		maybe for future use
---------------------------------------------------------------------- */

int detect_sec_change_loop (const int delay, libusb_device_handle *pdcf77_l, int *m)
{

//		USB_GET_TIMECHANGE call:
//		returns 1 if time has changed since last request and time source is DCF77
//		returns 0 if time has not changed since last request and time source is DCF77
//		returns 23 if source is RTC

	int status = -1;
	unsigned char iodata_time_change[8];
	int last_poll = 1;
	int current_poll = 1;
	*m = 0;

//	try to get USB_GET_TIMECHANGE flag
	while (last_poll >= current_poll) {
		status = -1;
		last_poll = current_poll;

		status = libusb_control_transfer(pdcf77_l, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
		                                  USB_GET_TIMECHANGE, 0, 0, iodata_time_change, 1, USB_TIMEOUT);
//		current_poll = iodata_time_change[0];		// will be 1 on second change

		if (status >= 0) {
			if (iodata_time_change[0] == 1) {
				current_poll = 1;
			} else if (iodata_time_change[0] == 0) {
				current_poll = 0;
			} else {
				return 23;			// when source is RTC return value will be 23
			}
		} else {
			const char* error_name = libusb_error_name(status);
			printf("Error: failed with error code: %s\n", error_name);		// fprintf to logfile
			printf("Error: failed with error code: %i\n", status);			// fprintf to logfile
		}

		(*m)++;
		usleep(delay);				// wait after the previous request

	}	// while loop

	return 0;
}


/* ----------------------------------------------------------------------
		try to get change of second with few requests (avoid requests every 5 ms for one second)
---------------------------------------------------------------------- */

int get_time_change(libusb_device_handle *pdcf77_rtc, FILE *logfile_handle, int detailed_dcf77_status_f)
{

	int exitCode = 0;
	const int poll_200ms = 200000;		// in us
	const int poll_50ms = 50000;		// in us
	const int poll_5ms = 5000;		// in us
	int m = 0;

	if (detailed_dcf77_status_f == 3) {
		m = detect_rtc_time_change_loop(poll_200ms, pdcf77_rtc, logfile_handle);
		if (log_flag ==1) {
			fprintf(logfile_handle, "clock %i us loop count: %i ", poll_200ms, m);
		}
		usleep(700000);					// 1000 - 800 - 2 * 50 ms

		m = 0;
		m = detect_rtc_time_change_loop(poll_50ms, pdcf77_rtc, logfile_handle);
		
		if (log_flag ==1) {
			fprintf(logfile_handle, "clock %i us loop count: %i ", poll_50ms, m);
		}
		usleep(880000);					// 900 ms is sometimes to long

		m = 0;
		m = detect_rtc_time_change_loop(poll_5ms, pdcf77_rtc, logfile_handle);
		
		if (log_flag ==1) {
			fprintf(logfile_handle, "clock %i us loop count: %i\n", poll_5ms, m);
		}

		}

//		continue (return) if dcf status != 3
		
		return exitCode;

}

/* ----------------------------------------------------------------------
		query current time from device until second has changed
---------------------------------------------------------------------- */

int detect_rtc_time_change_loop(const int delay, libusb_device_handle *pdcf77_rtc, FILE *logfile_handle)
{

	uint8_t last_second = 0;
	uint8_t current_second = 0;
	int status = -1;
	unsigned char iodata_time[8];
	int l = 0;

//		init
	status = libusb_control_transfer(pdcf77_rtc, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
	                                  USB_GET_TIMEDATA, 0, 0, iodata_time, 8, USB_TIMEOUT);

	if (status >= 0) { 
		current_second = bcd_decimal(iodata_time[5]);
		last_second = current_second;
	} else {
		const char* error_name = libusb_error_name(status);
		fprintf(logfile_handle, "Error: failed with error code: %s\n", error_name);
		fprintf(logfile_handle, "Error: failed with error code: %i\n", status);
		l = -1;
	}

	while (last_second == current_second) {

		last_second = current_second;
		usleep(delay);				// wait after the previous request

		status = libusb_control_transfer(pdcf77_rtc, LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
										USB_GET_TIMEDATA, 0, 0, iodata_time, 8, 5000);

		if (status >= 0) { 
			current_second = bcd_decimal(iodata_time[5]);
		} else {
			const char* error_name = libusb_error_name(status);
			fprintf(logfile_handle, "Error: failed with error code: %s\n", error_name);
			fprintf(logfile_handle, "Error: failed with error code: %i\n", status);
			l = -1;
		}

		l = l + 1;
		}		// while
		
		return l;

}


/* ----------------------------------------------------------------------
		extract status bits like summertime
---------------------------------------------------------------------- */

int extract_dcf77_status_bits(uint8_t status_bits, t_status_bits *dcf77_status_bits_l)
{


	dcf77_status_bits_l->DCFStatus = 0;
	dcf77_status_bits_l->summertime = 0;
	dcf77_status_bits_l->leap_second = 0;
	dcf77_status_bits_l->timesource = 0;
	dcf77_status_bits_l->valid = 0;

// Status; Bit0/1: DCFStatus; Bit2: Summertime; Bit3: leap second; Bit4: spare antenna; Bit5: 0; Bit6: time source (0=RTC, 1=DCF); Bit7: valid
		
	// Binary: 01100111

	// Extracting the 0th bit (0-based index)
	uint8_t bit_position = 0;
	uint8_t mask = 0;

	// Create a mask with only the x th bit set to 1
	mask = 1 << bit_position;			// bit position = 0

	// Extract the bit using AND and right shift
	dcf77_status_bits_l->DCFStatus = (status_bits & mask) >> bit_position;

	bit_position = 1;
	mask = 0;				// reset the mask
	mask = 1 << bit_position;
	dcf77_status_bits_l->summertime = (status_bits & mask) >> bit_position;

	bit_position = 2;
	mask = 0;
	mask = 1 << bit_position;
	dcf77_status_bits_l->leap_second = (status_bits & mask) >> bit_position;

	bit_position = 6;
	mask = 0;
	mask = 1 << bit_position;
	dcf77_status_bits_l->timesource = (status_bits & mask) >> bit_position;

	bit_position = 7;
	mask = 0;
	mask = 1 << bit_position;
	dcf77_status_bits_l->valid = (status_bits & mask) >> bit_position;

	return 0;
}


/* ----------------------------------------------------------------------
		query the time from device and show it
---------------------------------------------------------------------- */

int show_dev_time (t_dcf77_time_data *dcf77_time_t, t_status_bits *dcf77_status_bits_t)
{

	int dcf77_detail = 0;

	libusb_context *plc;

	if (libusb_init(&plc) != LIBUSB_SUCCESS) {
		printf("Error, could not initialize libusb\n");
		return -1;
	}

	libusb_device_handle *pldh = libusb_open_device_with_vid_pid(plc, VID, PID);

	get_dcf77_quality(pldh);
	usleep(80000);
	dcf77_detail = get_dcf77_status(pldh);		// 0=no signal, 1=waiting for minute start, 2=receiving data, 3=DCF time available
	usleep(80000);

	get_dcf77_current_time(pldh, dcf77_time_t, dcf77_status_bits_t);

	printf("DCF77 receive status: %i\n", dcf77_detail);
	printf("year: %i\n", dcf77_time_t->year);
	printf("month: %i\n", dcf77_time_t->month);
	printf("day: %i\n", dcf77_time_t->day);
	printf("day of week: %i\n", dcf77_time_t->day_of_week);
	printf("hour: %i\n", dcf77_time_t->hour);
	printf("minute: %i\n", dcf77_time_t->minute);
	printf("second: %i\n", dcf77_time_t->second);
	printf("summertime: %i\n", dcf77_status_bits_t->summertime);
	printf("valid: %i\n",  dcf77_status_bits_t->valid);

	libusb_close(pldh);					// close USB device handle
	libusb_exit(plc);

	return 0;

}

/* ----------------------------------------------------------------------
		show some device info
---------------------------------------------------------------------- */

int show_dev_info()
{

	libusb_context *plc;

	if (libusb_init(&plc) != LIBUSB_SUCCESS) {
		printf("Error, could not initialize libusb\n");
		return -1;
	}

	libusb_device_handle *pldh = libusb_open_device_with_vid_pid(plc, VID, PID);

	get_usb_sn(pldh);
	usleep(80000);
	get_eeprom_sn(pldh);
	usleep(80000);
	get_usb_manufacturer(pldh);
	usleep(80000);
	get_usb_product(pldh);
	usleep(80000);
	get_dcf77_dev_info(pldh);

	libusb_close(pldh);					// close USB device handle
	libusb_exit(plc);

	return 0;
}


/* ----------------------------------------------------------------------
		convert BCD values (received from DCF77) to decimal
---------------------------------------------------------------------- */

int bcd_decimal(uint8_t hex)
{
	assert(((hex & 0xF0) >> 4) < 10);		// More significant nybble is valid
	assert((hex & 0x0F) < 10);				// Less significant nybble is valid
	int dec = ((hex & 0xF0) >> 4) * 10 + (hex & 0x0F);
	return dec;
}       

/* ----------------------------------------------------------------------
		check if pid file exists
---------------------------------------------------------------------- */

int check_pid_file()
{

	FILE *pid_file = NULL;

	if ((pid_file = fopen("/var/run/usbdcf77re.pid", "r"))) {
		fclose(pid_file);
		return 1;
	} else {
		return 0;
	}

}

/* ----------------------------------------------------------------------
		close logfile and handle to device
---------------------------------------------------------------------- */

int shutdown(libusb_device_handle *pldh, FILE* dcf77_logfile_handle, FILE* dcf77_pid_file_handle)
{

	fprintf(dcf77_logfile_handle, "Caught signal %d shutting down\n", exit_flag);
	fprintf(dcf77_logfile_handle, "Closing USB device USBDCF77RE\n");

	libusb_close(pldh);					// close USB device handle

	fprintf(dcf77_logfile_handle, "=== end ===\n");
	fflush(dcf77_logfile_handle);
	fclose(dcf77_logfile_handle);				// close file
	fclose(dcf77_pid_file_handle);
	remove("/var/run/usbdcf77re.pid");
	return 0;
}

