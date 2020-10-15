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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*STAT*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* SELF */
#include "dum.h"

#define CMD_MAX        512
#define LINE_SIZE      4096
#define EOL_CR         0x0D
#define EOL_LF         0x0A
#define FMT_LINE_UNIX "%s\x0A"
#define FMT_LINE_MAC  "%s\x0D"
#define FMT_LINE_DOS  "%s\x0D\x0A"
#define COLORFUL_BLUE  (options & COLORFUL_BIT)?"\033[34m":""
#define COLORFUL_RED   (options & COLORFUL_BIT)?"\033[31m":""
#define COLORFUL_DEF   (options & COLORFUL_BIT)?"\033[0m":""
#define COLORFUL_BOLD  (options & COLORFUL_BIT)?"\033[1m":""
#define NOVBREAK       ((options & VERBOSE_BIT)&&(sum>0L))?"":"\n"

#include "messages.h"
#include "version.h"

/* Change it if you add some commandline syntax */
static const char usage_str[]=\
"dum [-vhct-] [command] [<files, ...>]\n"
"\n"
"where dum commands are:\n"
"\n"
"  config commands (first) :\n"
"     help             shows the help text\n"
"     version          shows the version string for current 'dum'\n"
"     verbose          sets verbose mode on -same as 'v' flag-\n"
"     showtime         sets date/time tracing on log strings (not all output)\n"
"     colorful         sets colorful mode on -same as 'c' flag and pretty-\n"
"     pretty           see colorful\n"
"     show             selects what line ending(s) should be seen (dos/unix/mac,\n"
"                      also 'bin' when a file has several types of line ends,\n"
"                      'dummy' if no line endings at all, and 'all' to set \n"
"                      everything on, also 'all' is the default)\n"
"\n"
"                      example: find . -print0 | xargs -0 dum show dos analyze\n"
"\n"
"  action commands (at the end) :\n"
"     analyze          analyzes the file and shows line ending style\n"
"                      (this is affected by 'show')\n"
"     to unix          rewrites input files using UNIX LF byte\n"
"     to dos           rewrites input files using DOS/WINDOWS CR+LF bytes\n"
"\n"
"  flags are easy to understand:\n"
"     -v               same as 'verbose'...\n"
"     -h               same as 'help'\n"
"     -c               same as 'colorful'\n"
"     -t               same as 'showtime'\n"
"     --               stop processing flags, you can also just say:\n"
"                      'dum -vct-' and it's the same as 'dum -vct --'\n"
"\n"
"Examples of massive usage:\n"
"\n"
"   find /some/src -type f -a -name \"Makefile\" | xargs dum to unix\n"
"   find / -type f | xargs dum colorful analyze\n"
"\n"
"   dum written by Rodriguez Dario A\n"
"\n"
"NOTE: If you don't use action commands, dum will do nothing.\n"
"\n"
"Please send bugs and suggestions to <rodriguez.dario.a@gmail.com>"
"\n"
"Copyright (C) 2010,2015,2016,2020 Dario A. Rodriguez";


typedef struct nd {
	unsigned long int cr;
	unsigned long int lf;
	unsigned long int crlf;
} numeric_data;


void delete_lineend(char * buffer);
int  analyze_file(FILE *file_to_read, numeric_data *totals);
int  is_regfile(const char *fname);


/* byte to save global configuration data and functions */
unsigned char options = '\0';


typedef struct cmdst {
	char command[CMD_MAX+1];
	void (*execute)(int *, char ***);
} cmdst;


/**
 * The whole flow is based on Commands, the very basic
 * builtin functions are the following. Other functions (if any)
 * will be added as modules
 */
void
cmdversion(int *a, char ***b)
{
	puts( "dum version "VERSION_NO );
	exit(0);
}


void
cmdhelp(int *a, char ***b)
{
	if ( !a && b && **b )
		die("invalid command '%s'... see 'help'", **b);
	else
		printf("usage:\n\n    %s\n", usage_str);
	exit(0);
}


void
cmdverbose(int *a, char ***b)
{
	(*a)--;(*b)++;
	options |= VERBOSE_BIT;
}


void
cmdcolorful(int *a, char ***b)
{
	(*a)--;(*b)++;
	options |= COLORFUL_BIT;
}


void
cmdshowtime(int *a, char ***b)
{
	(*a)--;(*b)++;
	options |= LOG_TIME_BIT;
}


void
cmdanalyze(int *a, char ***b)
{
  register int i;
  FILE *fstruct;
  numeric_data totals;

  (*a)--; (*b)++;

  trace_verbose ("Starting analyze");

  if (!a || !*b || !**b)
	die("invalid analyze usage!");

  for (i=0; i<*a; i++)
	{

	  if ( ! is_regfile((*b)[i]) )
		continue;

	  fstruct = fopen((*b)[i],"r");

	  if (!fstruct)
		die("I cannot open %s!\n",(*b)[i]);


	  /* get statistics */
	  analyze_file (fstruct,&totals);


	  /* print the report line*/
	  if ((totals.cr   >0L) &&
	      (totals.lf  ==0L) &&
	      (totals.crlf==0L) &&
	      (options & MAC_SHOW_BIT))
		printf("%sMAC         %s %s\n",
			   COLORFUL_BOLD,
			   COLORFUL_DEF,
			   (*b)[i]);
	  else if ((totals.cr  ==0L) &&
		   (totals.lf   >0L) &&
		   (totals.crlf==0L) &&
		   (options & UNIX_SHOW_BIT))
		printf("%sUNIX        %s %s\n",
			   COLORFUL_BOLD,
			   COLORFUL_DEF,
			   (*b)[i]);
	  else if ((totals.cr  ==0L) &&
		   (totals.lf  ==0L) &&
		   (totals.crlf >0L) &&
		   (options & DOS_SHOW_BIT))
		printf("%sDOS/WINDOWS %s %s\n",
			   COLORFUL_BOLD,
			   COLORFUL_DEF,
			   (*b)[i]);
	  else if ((totals.cr  ==0L) &&
		   (totals.lf  ==0L) &&
		   (totals.crlf==0L) &&
		   (options & DUMMY_SHOW_BIT))
		printf("%sUNKNOWN     %s %s\n",
			   COLORFUL_BOLD,
			   COLORFUL_DEF,
			   (*b)[i]);
	  else if (options & BINARY_SHOW_BIT)
		printf("%sBINARY      %s %s\n",
			   COLORFUL_BOLD,
			   COLORFUL_DEF,
			   (*b)[i]);
	  /* end report */

	  fclose (fstruct);

	} /* end for */
  trace_verbose ("Ending normally");
  exit(0);
}


void
cmdshow( int *a, char ***b )
{
	register int i;

	(*a)--;(*b)++;

	if (!a||!*b||!**b)
		die("invalid show usage!");

	unsigned char negative_mask = 0xFF;

	/* set holes in the negative mask */
	negative_mask ^= UNIX_SHOW_BIT;
	negative_mask ^= DOS_SHOW_BIT;
	negative_mask ^= MAC_SHOW_BIT;
	negative_mask ^= BINARY_SHOW_BIT;
	negative_mask ^= DUMMY_SHOW_BIT;

	/* force bits to zero */
	options &= negative_mask;

	for (i=0; i < *a; i++) {
		if (!strcmp((*b)[i], "unix"))
		  options |= UNIX_SHOW_BIT;
		else if (!strcmp((*b)[i], "dos") || !strcmp((*b)[i],"windows"))
		  options |= DOS_SHOW_BIT;
		else if (!strcmp((*b)[i], "mac"))
		  options |= MAC_SHOW_BIT;
		else if (!strcmp((*b)[i], "bin"))
		  options |= BINARY_SHOW_BIT;
		else if (!strcmp((*b)[i], "dummy"))
		  options |= DUMMY_SHOW_BIT;
		else if (!strcmp((*b)[i], "all")) {
		  options |= UNIX_SHOW_BIT;
		  options |= DOS_SHOW_BIT;
		  options |= MAC_SHOW_BIT;
		  options |= BINARY_SHOW_BIT;
		  options |= DUMMY_SHOW_BIT;
		} else if (!strcmp((*b)[i], "endshow")) {
			(*a)--; (*b)++;
			return;
		}
		else return;
		(*a)--; (*b)++;
	}

}


void
cmdto( int *a, char ***b )
{
  const int TMP_FILENAME_PATH_SIZE = 4096;

  char tmp_file_name[TMP_FILENAME_PATH_SIZE];
  char buffer[LINE_SIZE];
  int  app_me;
  register int i;
  struct stat stbuf;
  FILE *tmp_file;
  FILE *input_file;
  char output_type=0;

  enum LineEnd { UNIX='u', MAC='m', DOS='d' };

  (*a)--; (*b)++;

  trace_verbose ("Starting to");

  if (!a||!*b||!**b)
	die("invalid 'to' usage!");

  /* sets conversion type */
  if (!strcmp(**b,"unix"))
	output_type = UNIX;

  else if (!strcmp(**b,"mac"))
	output_type = MAC;

  else if (!strcmp(**b,"dos") || !strcmp(**b,"windows"))
	output_type = DOS;

  else
	die("invalid 'to' usage!");

  (*a)--;(*b)++; // type



  /* for each file... */
  for (i = 0; i < (*a); i++)
	{

	  /* defines tempname */
	  memset(tmp_file_name, 0, TMP_FILENAME_PATH_SIZE);
	  sprintf(tmp_file_name,"/tmp/dumtemp%d.txt",app_me);

	  for (app_me = 0; stat(tmp_file_name,&stbuf) == 0 && app_me >= 0; app_me++)
		{
		  memset(tmp_file_name, 0, TMP_FILENAME_PATH_SIZE);
		  snprintf(tmp_file_name, TMP_FILENAME_PATH_SIZE, "/tmp/dumtemp%d.txt", app_me);
		}

	  if (app_me < 0)
		die("OMG! How many temp files do you have? ... see /tmp/dumtemp*.txt");


	  /* open tmp_file and input */
	  tmp_file = fopen(tmp_file_name, "w");

	  if (!tmp_file)
		die( "I cannot open %s!\n",tmp_file_name);


	  input_file = fopen((*b)[i],"r");


	  if (!input_file)
		die( "I cannot open %s!\n",(*b)[i]);


	  /* Trace if verbose */
	  if (options & VERBOSE_BIT) {
		switch( output_type ) {
		case UNIX:
		  trace( "(TO UNIX) %s", (*b)[i] );
		  break;
		case DOS:
		  trace( "(TO DOS)  %s", (*b)[i] );
		  break;
		default:
		  die("not a valid conversion\n");
		}
	  }

	  /* read... */
	  memset(buffer,0,LINE_SIZE);
	  while (fgets(buffer,LINE_SIZE,input_file))
		{

		  delete_lineend(buffer);

		  /* writes the line to the output file */
		  switch(output_type) {

		  case UNIX:
			fprintf(tmp_file, FMT_LINE_UNIX, buffer);
			break;

		  case DOS:
			fprintf(tmp_file, FMT_LINE_DOS, buffer);
			break;

		  }

		  memset(buffer,0,LINE_SIZE);
		}

	  /* close files */
	  fclose(input_file);
	  fclose(tmp_file);

	  /* buffer gets recycled */
	  snprintf( buffer, LINE_SIZE,"mv %s %s", tmp_file_name, (*b)[i] );
	  if(system(buffer)) {
		die("Cannot move temporary file to the original one!\n");
	  }
	}

  trace_verbose ("Ending normally");
  exit(0);
}







/* This global, static array containing commands and functions */
static const struct cmdst commands[] = {

  { "help"      , cmdhelp      },
  { "colorful"  , cmdcolorful  },
  { "pretty"    , cmdcolorful  },
  { "showtime"  , cmdshowtime  },
  { "version"   , cmdversion   },
  { "verbose"   , cmdverbose   },
  { "analyze"   , cmdanalyze   },
  { "show"      , cmdshow      },
  { "to"        , cmdto        },
  { ""          , NULL         }

};


static void
execute_command( char* command, int *argc, char*** argv)
{
  int executed = 0;
  char **cmdref = &command;

  if (!command)
	die("dum.c:%d  UNEXPECTED NULL POINTER", __LINE__);

  const struct cmdst *p=commands;
  while (p->command[0])
	{
	  if(!strcmp(p->command,command)) {
		p->execute(argc, argv);
		executed = 1;
	  }
	  p++;
	}

  if (!executed)
	cmdhelp(NULL,&cmdref);

}


int
main(int argc, char **argv)
{

  if (argc < 2)
	cmdhelp(0, NULL);

  int j;

  options = '\0';

  /* default to show all */
  options |= UNIX_SHOW_BIT;
  options |= DOS_SHOW_BIT;
  options |= MAC_SHOW_BIT;
  options |= BINARY_SHOW_BIT;
  options |= DUMMY_SHOW_BIT;

  argv++; /* lose first */
  argc--;

  for (;;)
	{

	  if(!argc || !argv[0])
		break;

	  unsigned short int parsing_args;
	  
	  if (parsing_args && argv[0][0] == '-') {

		for (j=1; j < strlen(argv[0]); j++)
		  switch (argv[0][j]) {
		  case '-':
			parsing_args = 0;
			break;
		  case 'v':
		  case 'V':
			options |= VERBOSE_BIT;
			break;
		  case 'h':
		  case 'H':
			cmdhelp(0,NULL);
			break;
		  case 'c':
		  case 'C':
			options |= COLORFUL_BIT;
			break;
		  case 't':
		  case 'T':
			options |= LOG_TIME_BIT;
			break;
		  default:
			die("Bad option: %c (on %s)",
				argv[0][j],argv[0]);
			break;
		  }

		argc--; argv++;

	  } else {
		/* command processing - use it just as an access to functionalities */
		if(strlen(argv[0]) > CMD_MAX)
		  die("Command specified is too long, "
			  "current limit is %d",CMD_MAX);

		execute_command(*argv,  &argc,  &argv);
	  }
	}

  trace_verbose ("Ending normally with no commands");
  return 0;
}


void
delete_lineend(char * buffer)
{
  register int i = 0;
  while( buffer[i] != 0 )
	{
	  if( buffer[i] == EOL_CR || buffer[i] == EOL_LF )
		buffer[i] = 0;

	  i++;
	}
}


int
analyze_file(FILE* file_to_read,numeric_data *totals)
{
  char buffer[LINE_SIZE];
  memset(buffer, 0, LINE_SIZE);
  totals->crlf = 0L;
  totals->cr = 0L;
  totals->lf = 0L;

  /* read file */
  while (fgets(buffer, LINE_SIZE, file_to_read))
	{

	  register int i = 0;
	  while (buffer[i])
		{

		  if (buffer[i] == EOL_CR && buffer[i+1] == EOL_LF) {
			totals->crlf++;
			i++;
		  }

		  else if (buffer[i] == EOL_CR)
			totals->cr++;

		  else if (buffer[i] == EOL_LF)
			totals->lf++;

		  /* finally */
		  i++;
		}
	  memset(buffer,0,LINE_SIZE);
	}

  return 0;
}


int
is_regfile (const char *fname)
{
	struct stat stbuff;

	/* does not exist, is not regular file then */
	if (stat(fname, &stbuff))
	  return 0;

	return S_ISREG(stbuff.st_mode);
}
