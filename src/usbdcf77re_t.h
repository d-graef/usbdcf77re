/*

License : GPL-2.0

https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

(c) 2024 Detlef Graef
(c) 2024 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik

*/


// time informationen
typedef struct {
	int year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day_of_week;
	uint8_t status;
} t_dcf77_time_data;


// decoded status bits
typedef struct {
	uint8_t DCFStatus;
	uint8_t summertime;
	uint8_t leap_second;
	uint8_t timesource;
	uint8_t valid;
} t_status_bits;

// Status; Bit0/1: DCFStatus; Bit2: Summertime; Bit3: leap second; Bit4: spare antenna; Bit5: 0; Bit6: time source (0=RTC, 1=DCF); Bit7: valid


// statistics
typedef struct {
	unsigned int dcf77_count;
	unsigned int rtc_count;
} t_usbdcf77re_stats;


// ------------------------------------------------------------------
// from: https://docs.ntpsec.org/latest/driver_shm.html

// gpsd communicates with ntpd using shared memory segments (SHMs). The SHMs are numbered SHM(0) to SHM(7). and have a name (key) of
//       NTP0 to NTP7. The first local time source on the command line may use NTP0 and NTP1. The second may use NTP2 and NTP3, etc.
//
//       You can see the output of gpsd to ntpd in real time with the ntpshmmon command.


typedef struct {
	int mode;	/* 0 - if valid is set:
				*  use values,
				*  clear valid
				*  1 - if valid is set:
				*  if count before and after read of data is equal:
				*  use values
				*  clear valid
				*/
	volatile int	count;
	time_t			clockTimeStampSec;
	int				clockTimeStampUSec;
	time_t			receiveTimeStampSec;
	int				receiveTimeStampUSec;
	int				leap;
	int				precision;
	int				nsamples;
	volatile int	valid;
	unsigned		clockTimeStampNSec;		/* Unsigned ns timestamps */
	unsigned		receiveTimeStampNSec;	/* Unsigned ns timestamps */
	int				dummy[8];
} shmTime;
