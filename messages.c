/**
 * ---------------------------------------------------------------------------
 * Copyright (c) 2010,2016
 *                       Rodriguez Dario Andres, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ---------------------------------------------------------------------------
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "dum.h"


static void
freport (const char *prefix, const char *err, va_list params)
{
  char msg[8192];
  FILE *fptr;

  struct tm *timeinfo;
  time_t esttime;
  time (&esttime);
  timeinfo = localtime (&esttime);
  vsnprintf(msg, sizeof(msg), err, params);


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



static void
die_func (const char *err, va_list params)
{
	freport ("FATAL : ", err, params);
	exit (128);
}


void
trace (const char *trc, ...)
{
	va_list params;
	va_start (params, trc);
	freport ("TRACE : ",trc, params);
	va_end (params);
}



void
trace_verbose (const char *trc, ...)
{
	if (dumconfig->verbose) {
		va_list params;
		va_start (params, trc);
		freport ("TRVRB : ",trc, params);
		va_end (params);
	}
}


void
die (const char *err, ...)
{
	va_list params;
	va_start (params, err);
	die_func (err, params);
	va_end (params);
}
