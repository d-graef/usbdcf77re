/*

License : GPL-2.0

https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

(c) 2024 Detlef Graef
(c) 2024 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik

usbdcf77-ntp-shm.h

*/


/* ----------------------------------------------------------------------
		connect to she shared memory segment of chrony, ntpd
---------------------------------------------------------------------- */

int init_ntp_shm(shmTime **p_shmtime_dcf77_i, int shmid)
{

	int shmflg = 0;
	void *addr;

	addr = shmat(shmid, NULL, shmflg);
	if (addr == (void *) -1) {
		return -1;
	}
	*p_shmtime_dcf77_i = (shmTime *)addr;	// assign pointer to SHM

	return 0;
}


/* ----------------------------------------------------------------------
		write current time to shared memory segment
---------------------------------------------------------------------- */


int write_time_ntp_shm(shmTime *p_shmtime_dcf77_u, t_dcf77_time_data *dcf77_time_s, t_status_bits *dcf77_status_bits_s, int dcf77_status,
					FILE *logfile_handle, int tz_offset_shm, t_usbdcf77re_stats *usbdcf77re_stats_s)
{ 

	struct tm t;
	struct tm *t2;
	time_t t_of_day;
	int clock_diff = 0;
	unsigned int clock_diff_abs = 0;
	unsigned int stats_counter = 0;
	struct timespec ts;
	char buffer[80];
	time_t log_timestamp = 0;
	struct timespec timestamp;

	if (dcf77_status_bits_s->valid == 1) {
		t.tm_year = dcf77_time_s->year - 1900;				// Year - 1900
		t.tm_mon = dcf77_time_s->month - 1;					// Month, where 0 = jan
		t.tm_mday = dcf77_time_s->day;						// Day of the month
		t.tm_hour = dcf77_time_s->hour + tz_offset_shm;		// input is local time e. g. CET, CEST, CET, CEST, WET, WEST. DCF77 = CET or CEST
		t.tm_min = dcf77_time_s->minute;
		t.tm_sec = dcf77_time_s->second;
		t.tm_isdst = -1;							// Is DST on? 1 = yes, 0 = no, -1 = unknown
		t_of_day = mktime(&t);						// convert DCF77 values to UNIX time
		clock_gettime(CLOCK_REALTIME, &timestamp);	// get current system time
		clock_diff = t_of_day - timestamp.tv_sec;	// diff of seconds
		clock_diff_abs = abs(clock_diff);

//		write values to SHM

			if (dcf77_status == 3) {
				p_shmtime_dcf77_u->clockTimeStampUSec = 56000;		// when source DCF 56 ms
				p_shmtime_dcf77_u->precision = -8;					// 2^(-8) = 0.0039 seconds
			} else {
				p_shmtime_dcf77_u->clockTimeStampUSec = 160000;		// when source RTC 60 ms + 100 ms loop delay meantime
				p_shmtime_dcf77_u->precision = -3;					// 2^(-3) = 0.125 seconds
			}

		p_shmtime_dcf77_u->mode = 0;
		p_shmtime_dcf77_u->count = 0;
		p_shmtime_dcf77_u->clockTimeStampSec = t_of_day;						// UTC
		p_shmtime_dcf77_u->receiveTimeStampSec = timestamp.tv_sec;				// from clock_gettime() UTC
		p_shmtime_dcf77_u->receiveTimeStampUSec = (timestamp.tv_nsec / 1000);	// from clock_gettime() UTC (nsec -> usec)
		p_shmtime_dcf77_u->leap = 0;
		p_shmtime_dcf77_u->clockTimeStampNSec = 0;
		p_shmtime_dcf77_u->receiveTimeStampNSec = 0;
		p_shmtime_dcf77_u->dummy[0] = 0;
		p_shmtime_dcf77_u->valid = 1;					// set this last;

//		convert UNIX time to human readable

		clock_gettime(CLOCK_REALTIME, &ts);			// returns current time in UNIX time
		log_timestamp = ts.tv_sec;
		t2 = localtime(&log_timestamp);

		strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S %Y", t2);

//		update stats
			if (dcf77_status == 3) {
				usbdcf77re_stats_s->dcf77_count = usbdcf77re_stats_s->dcf77_count + 1;
			} else {
			usbdcf77re_stats_s->rtc_count = usbdcf77re_stats_s->rtc_count + 1;
			}

//		write stats to logfile
		stats_counter = usbdcf77re_stats_s->dcf77_count + (usbdcf77re_stats_s->rtc_count * 4);

//		after about every 15 minutes write stats to logfile
			if (stats_counter >= 50) {
				fprintf(logfile_handle, "%s stats: (written to SHM) dcf77-count: %i rtc-count: %i\n", buffer, usbdcf77re_stats_s->dcf77_count,
				usbdcf77re_stats_s->rtc_count);
				usbdcf77re_stats_s->dcf77_count = 0;
				usbdcf77re_stats_s->rtc_count = 0;
			}

//		write logfile entry if time diff > 1 sec.
			if (clock_diff_abs > 0) {
				fprintf(logfile_handle, "%s diff sys-time (%li) / device (%li) %i second(s) (check receiving state)\n", buffer, timestamp.tv_sec, t_of_day, clock_diff);
			}

			if (log_flag ==1) {
				fprintf(logfile_handle, "%s dcf77: %02d:%02d:%02d  dcf-status: %i t-source: %i valid: %i summertime: %i dcf77-detail: %i \
 offset us: %li clock-diff: %i\n",
				buffer, dcf77_time_s->hour, dcf77_time_s->minute, dcf77_time_s->second, dcf77_status_bits_s->DCFStatus,
				dcf77_status_bits_s->timesource, dcf77_status_bits_s->valid, dcf77_status_bits_s->summertime, dcf77_status, timestamp.tv_nsec,
				clock_diff);
			}

		fflush(logfile_handle);
		

	} else {	// if valid != 1
		
		fprintf(logfile_handle, "valid bit != 1 valid-bit: %i\n", dcf77_status_bits_s->valid);
		fflush(logfile_handle);
	}

	return 0;

}



