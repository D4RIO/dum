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

/* NOTE: You find a good reason NOT to write inside the header */

#ifndef _DUM_MAIN_HEADER_
#define _DUM_MAIN_HEADER_

/* This is the global options memory space */
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

/* and the symbol exported by dum.o */
extern options_st *dumconfig;

#endif
