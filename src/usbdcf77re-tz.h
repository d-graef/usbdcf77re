/*

License : GPL-2.0

https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

(c) 2024 Detlef Graef
(c) 2024 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik


*/


/* ----------------------------------------------------------------------
		get the timezone string, like CET, CEST, WEE, EET, EEST
---------------------------------------------------------------------- */

int get_tz_string(char *tz_string_m)
{

	struct timespec timestamp;
	struct tm *tm_time_tz = NULL;
	time_t unix_time = 0;

	clock_gettime(CLOCK_REALTIME, &timestamp);
	unix_time = timestamp.tv_sec;

	tm_time_tz = localtime(&unix_time);
	strcpy(tz_string_m, tm_time_tz->tm_zone);

	return 0;

}



/* ----------------------------------------------------------------------
		calculate the offset from the timezone string
---------------------------------------------------------------------- */

int calc_tz_offset(char *tz_string_c) {


	if (!strcmp(tz_string_c, "EET")) {
		return 3600;
	} else if (!strcmp(tz_string_c, "EEST")) {
	return 3600;
	} else if (!strcmp(tz_string_c, "WET")) {
	return -3600;
	} else if (!strcmp(tz_string_c, "WEST")) {
		return -3600;
	} else if (!strcmp(tz_string_c, "GMT")) {
		return -3600;
	} else if (!strcmp(tz_string_c, "BST")) {
	return -3600;
	}
	return 0;				// default 0; CET, CEST

}

