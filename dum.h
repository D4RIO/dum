/* Be careful, no serious, because every thing you put here is redistributed
 * to almost every C source in the hierarchy.
 * First, you find a good reason to NOT write here, then, if you really can't find
 * an alternate way, you write here, with good excuses.
 *
 * Copyright (C) 2010, Dario A. Rodriguez
 */

#ifndef _DUM_MAIN_HEADER_
#define _DUM_MAIN_HEADER_

#include "dstring.h"

/* This is the global options memory space
 */

typedef struct options_st {

	unsigned char  verbose;
	unsigned char  colorful;
	unsigned char  parsing_args;
	unsigned char  log_time;
	unsigned char  unix_show;
	unsigned char  mac_show;
	unsigned char  dos_show;
	unsigned char  binary_show;
	unsigned char  dummy_show;
	FILE          *log_file;

} options_st;

extern options_st *dumconfig;

#endif
