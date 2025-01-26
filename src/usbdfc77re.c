/*

License : GPL-2.0

https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

(c) 2024 Detlef Graef
(c) 2024 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik

*/

/* ----------------------------------------------------------------------
		global vars
---------------------------------------------------------------------- */


int exit_flag = 0;		// for access from signal handler
short log_flag = 0;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#if __FreeBSD__
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif
#include <assert.h>
#include <time.h>
#include <sys/shm.h>
#include <signal.h>

#define VID 0x16C0		// USB vendor ID
#define PID 0x27EA		// USB product ID
#define SNLEN 8			// Lenght of USB serial number
#define CONTROLBYTES 8
#define USB_TIMEOUT 500		//in ms

#include "commands.h"
#include "usbdcf77re_t.h"
#include "usbdcf77re.h"
#include "usbdcf77-ntp-shm.h"
#include "usbdcf77re-tz.h"
#include "usbdcf77re_sig.h"


/* ----------------------------------------------------------------------
		main()
---------------------------------------------------------------------- */


int main(int argc, char **argv) {

	t_dcf77_time_data dcf77_time;			// struct with time from device
	t_status_bits dcf77_status_bits;		// struct with status from the time query from the device
	short detailed_dcf77_status = 0;		// 0=no signal, 1=waiting for minute start, 2=receiving data, 3=DCF time available
	shmTime *p_shmtime_dcf77 = NULL;
	pid_t usbdcf77re_pid = 0;
	char tz_string[] = "nnnn";				// timezone string max. 4 char
	int tz_offset = 0;						// in seconds
	const char usbcdf77re_version[] = "0.1.1";

	t_usbdcf77re_stats usbdcf77re_stats;

	int shm_id = 99;
	int exitCode = 0;	

	short show_info = 0;
	short show_time = 0;
	char *shm_id_s = NULL;
	int c = 0;
	int w_count = 0;
	FILE *dcf77_logfile_handle = NULL;
	FILE *dcf77_pid_file_handle = NULL;

	usbdcf77re_stats.dcf77_count = 0;
	usbdcf77re_stats.rtc_count = 0;

	if (argc == 1) {
		usage(usbcdf77re_version);
		return 0;
	} 

	while ((c = getopt (argc, argv, "vhitld:")) != -1)
		switch (c)
		{
		case 'v':
			printf("usbdcf77re version: %s\n", usbcdf77re_version);
			return 0;
			break;
		case 'h':
			usage(usbcdf77re_version);
			return 0;
			break;
		case 'i':
			show_info = 1;
			break;
		case 't':
			show_time = 1;
			break;
		case 'l':
			log_flag = 1;
			break;
		case 'd':
			shm_id_s = optarg;
			shm_id = atoi(shm_id_s);
			break;
		case '?':
			if (optopt == 'd') {
				printf("Option -%c requires an argument.\n", optopt);
			}
			else {
				printf ("Unknown option character `\\x%x'.\n", optopt);
				return 1;
			}
			break;
		default:
			usage(usbcdf77re_version);
			return 0;
		}	// while getopt


	if ((log_flag == 1) & (shm_id ==99)) {		// -d option not set
		usage(usbcdf77re_version);
		return 0;
	}

	if (show_info == 1) {
		show_dev_info();
		return 0;
	} else if (show_time ==1) {
		show_dev_time(&dcf77_time, &dcf77_status_bits);
		return 0;
	}	//	else continue in daemon mode

	signal(SIGINT, handle_sigint);				// register signal handler
	signal(SIGTERM, handle_sigterm);			// register signal handler

	dcf77_logfile_handle = fopen("/var/log/usbdcf77re.log", "a");	// open logfile
	fprintf(dcf77_logfile_handle, "=== starting ===\n");
	fflush(dcf77_logfile_handle);

	libusb_context *plc;
	if (libusb_init(&plc) != LIBUSB_SUCCESS) {
		fprintf(dcf77_logfile_handle, "Error, could not initialize libusb\n");
		return -1;
	}
	libusb_device_handle *pldh = libusb_open_device_with_vid_pid(plc, VID, PID);

	// check if PID file exists
	if (check_pid_file()) {
		printf("Please check if usbdcf77re is already running and check the PID file /var/run/usbdcf77re.pid\n");
		return 3;
	}

	daemon(0,0);						// run in background
	
	usbdcf77re_pid = getpid();
	dcf77_pid_file_handle = fopen("/var/run/usbdcf77re.pid", "w");		// write PID file
	fprintf(dcf77_pid_file_handle, "%i\n", usbdcf77re_pid);
	fflush(dcf77_pid_file_handle);
	
	get_tz_string(tz_string);
	fprintf(dcf77_logfile_handle, "timezone: %s\n", tz_string);		// write timezone to logfile
	tz_offset = calc_tz_offset(tz_string);

	if (pldh) {

		fprintf(dcf77_logfile_handle, "Successfully opened USB device USBDCF77RE\n");
		fflush(dcf77_logfile_handle);

		if ((init_ntp_shm(&p_shmtime_dcf77, shm_id)) == -1) {
			fprintf(dcf77_logfile_handle, "Error: attaching to SHM");		// write timezone to logfile;
			return -1;
		}
		
		detailed_dcf77_status = get_dcf77_status(pldh);			// return: 0, 1, 2, 3. red, yellow, green.
		usleep(80000);

		while (exit_flag == 0) {					// ========= main loop =========

			get_time_change(pldh, dcf77_logfile_handle, detailed_dcf77_status);		// always query via "USB_GET_TIMEDATA" USB command

			if (detailed_dcf77_status != 3) {

				for (w_count = 1; w_count <= 31; w_count++) {
					usleep(2000000);			// check exit flag every 2 sec. wait 62 seconds (31 * 2 sec.)
					if (exit_flag != 0) {
						shutdown(pldh, dcf77_logfile_handle, dcf77_pid_file_handle);
						return 0;
					}
				}
			}	// if

		// query time from device right after switch of second
		get_dcf77_current_time(pldh, &dcf77_time, &dcf77_status_bits);

		detailed_dcf77_status = get_dcf77_status(pldh);			// return: 0, 1, 2, 3. red yellow, green.

		// write time immediately to shared memory
		write_time_ntp_shm(p_shmtime_dcf77, &dcf77_time, &dcf77_status_bits, detailed_dcf77_status, dcf77_logfile_handle, tz_offset,
						&usbdcf77re_stats);
		
		for (w_count = 1; w_count <= 7; w_count++) {
			usleep(2000000);			// check exit flag every 2 sec. wait 14 seconds (7 * 2 sec.)
			if (exit_flag != 0) {
				shutdown(pldh, dcf77_logfile_handle, dcf77_pid_file_handle);
				shmdt(p_shmtime_dcf77);				// detach from SHM
				return 0;
			}
		}
		detailed_dcf77_status = get_dcf77_status(pldh);				// get current status short before next loop

		}		// main while loop


	} else {
		fprintf(dcf77_logfile_handle, "Error: No device connected, or not enough permissions.\n");
		exitCode = -1;
	}

	libusb_exit(plc);
	return exitCode;

}		// end main()


