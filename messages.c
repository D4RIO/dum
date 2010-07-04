/*
* Copyright (C) 2010, Dario A. Rodriguez
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "dum.h"


static void freport (const char *prefix, const char *err, va_list params) {

	char msg[8192];
	FILE *fptr;

	struct tm *timeinfo;
	time_t esttime;
	time (&esttime);
	timeinfo = localtime (&esttime);
	vsnprintf (msg, sizeof(msg), err, params);

	if (dumconfig->log_file)
		fptr = dumconfig->log_file;
	else
		fptr = stderr;


	if (dumconfig->log_time)
		fprintf (fptr, "%02d/%02d/%02d %02d:%02d:%02d - %s%s\n",
					timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900,
					timeinfo->tm_hour, timeinfo->tm_min,  timeinfo->tm_sec,
					prefix, msg);
	else
		fprintf (fptr, "%s%s\n", prefix, msg);

}



static void die_func (const char *err, va_list params) {

	freport ("FATAL : ", err, params);
	exit (128);

}


void trace (const char *trc, ...) {

	va_list params;
	va_start (params, trc);
	freport ("TRACE : ",trc, params);
	va_end (params);

}



void trace_verbose (const char *trc, ...) {

	if (dumconfig->verbose) {
		va_list params;
		va_start (params, trc);
		freport ("TRVRB : ",trc, params);
		va_end (params);
	}

}



void die (const char *err, ...) {

	va_list params;
	va_start (params, err);
	die_func (err, params);
	va_end (params);

}

