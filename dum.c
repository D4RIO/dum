#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*STAT*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define CMD_MAX        512
#define TAM_LINE       4096
#define EOL_CR         0x0D
#define EOL_LF         0x0A
#define FMT_LINEA_UNIX "%s\x0A"
#define FMT_LINEA_MAC  "%s\x0D"
#define FMT_LINEA_DOS  "%s\x0D\x0A"
#define COLORFUL_BLUE  dumconfig->colorful?"\033[34m":""
#define COLORFUL_RED   dumconfig->colorful?"\033[31m":""
#define COLORFUL_DEF   dumconfig->colorful?"\033[0m":""
#define COLORFUL_BOLD  dumconfig->colorful?"\033[1m":""
#define NOVBREAK       (dumconfig->verbose&&(sum>0L))?"":"\n"

#include "dstring.h"
#include "messages.h"
#include "version.h"

/* Change it if you add some commandline syntax
 */
static const char usage_str[]=\
"dum [-vhct-] [command] [<files, ...>]\n"
"\n"
"The most commonly used dum commands are:\n\n"
"  config commands (parsed first):\n"
"     help             shows the help text\n"
"     version          shows the version string for current 'dum'\n"
"     verbose          sets verbose mode on -same as 'v' flag-\n"
"     showtime         sets date/time tracing on log strings (not all output)\n"
"     colorful         sets colorful mode on -same as 'c' flag and pretty-\n\n"
"  'knife' commands (at the end):\n"
"     to unix          rewrite input files using UNIX LF byte\n"
"     to dos           rewrite input files using DOS/WINDOWS CR+LF bytes\n"
"     show             for each input file, shows line ending style or 'BIN'\n\n"
"each command can also define a 'help' function,\n"
"so you can shoot your 'dum show help' to know wth\n"
"the 'show' command is. simple.\n"
//"    find\n"
//"        unix\n"
//"        dos\n"   // PROJECT
//"        mac\n"
"\n"
"Copyright (C) 2010, Dario A. Rodriguez";

typedef struct nd {
	unsigned long int cr;
	unsigned long int lf;
	unsigned long int crlf;
} numeric_data;

void delete_lineend(char * buffer);
int  analyze_file (FILE *file_to_read,numeric_data *totals);
int  is_regfile (const char *fname);


/* structure to save global configuration data, and functions --------------------------------------
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

options_st *dumconfig = NULL;

void options_build(options_st *ref) {
	ref->verbose      = 0;
	ref->colorful     = 0;
	ref->parsing_args = 1;
	ref->log_time     = 0;
	ref->unix_show    = 1;
	ref->mac_show     = 1;
	ref->dos_show     = 1;
	ref->binary_show   = 1;
	ref->dummy_show   = 1;
	ref->log_file     = NULL;
}

void options_reset(options_st *ref) {
	ref->verbose      = 0;
	ref->colorful     = 0;
	ref->parsing_args = 1;
	ref->log_time     = 0;
	ref->unix_show    = 1;
	ref->mac_show     = 1;
	ref->dos_show     = 1;
	ref->binary_show   = 1;
	ref->dummy_show   = 1;
	ref->log_file     = NULL;
}
/* ---------------------------------------------------------------------------------------------- */

typedef struct cmdst {
	char command[CMD_MAX+1];
	void (*execute)(int *, char ***);
} cmdst;






/* The whole flow is based on Commands, the very basic
 * builtin functions are the following. Other functions will
 * be added as modules
 */
void cmdversion( int *a, char ***b ) {

	puts( "dum version "VERSION_NO );
	exit(0);
}

void cmdhelp( int *a, char ***b ) {
	if ( !a && b && **b )
		die("invalid command '%s'... see 'help', ok?", **b);
	else
		printf("usage:\n\n    %s\n", usage_str);
	exit(0);
}

void cmdverbose( int *a, char ***b ) {

	(*a)--;(*b)++;
	dumconfig->verbose = 1;

}

void cmdcolorful( int *a, char ***b ) {

	(*a)--;(*b)++;
	dumconfig->colorful = 1;

}

void cmdshowtime( int *a, char ***b ) {

	(*a)--;(*b)++;
	dumconfig->log_time = 1;

}

void cmdanalyze( int *a, char ***b ) {

	register int i;
	FILE *fstruct;
	numeric_data totals;
	(*a)--;(*b)++;

	if (!a||!*b||!**b)
		die("invalid analyze usage!");

	for (i=0;i<*a;i++) {

		if ( ! is_regfile((*b)[i]) )
			continue;

		fstruct = fopen ((*b)[i],"r");

		if (!fstruct)
			die("I cannot open %s!\n",(*b)[i]);

		/* get statistics */
		analyze_file (fstruct,&totals);

		/* print the report line*/
		if ((totals.cr   >0L) &&
		    (totals.lf  ==0L) &&
		    (totals.crlf==0L) &&
		     dumconfig->mac_show)
			printf("%sMAC        %s %s\n",
			       COLORFUL_BOLD,
			       COLORFUL_DEF,
			       (*b)[i]);
		else if ((totals.cr  ==0L) &&
		         (totals.lf   >0L) &&
		         (totals.crlf==0L) &&
		          dumconfig->unix_show)
			printf("%sUNIX       %s %s\n",
			       COLORFUL_BOLD,
			       COLORFUL_DEF,
			       (*b)[i]);
		else if ((totals.cr  ==0L) &&
		         (totals.lf  ==0L) &&
		         (totals.crlf >0L) &&
		          dumconfig->dos_show)
			printf("%sDOS/WINDOWS%s %s\n",
			       COLORFUL_BOLD,
			       COLORFUL_DEF,
			       (*b)[i]);
		else if ((totals.cr  ==0L) &&
		         (totals.lf  ==0L) &&
		         (totals.crlf==0L) &&
		          dumconfig->dummy_show)
			printf("%sNO-ENDS    %s %s\n",
			       COLORFUL_BOLD,
			       COLORFUL_DEF,
			       (*b)[i]);
		else if (dumconfig->binary_show)
			printf("%sBINARY     %s %s\n",
			       COLORFUL_BOLD,
			       COLORFUL_DEF,
			       (*b)[i]);
		/* end report */

		fclose (fstruct);

	}
	exit(0);

}

void cmdshow( int *a, char ***b ) {

	register int i;
	(*a)--;(*b)++;

	if (!a||!*b||!**b)
		die("invalid show usage!");

	dumconfig->unix_show   = 0;
	dumconfig->dos_show    = 0;
	dumconfig->mac_show    = 0;
	dumconfig->binary_show = 0;
	dumconfig->dummy_show  = 0;

	for (i=0;i<*a;i++) {
		if (!strcmp((*b)[i],"unix"))
			dumconfig->unix_show = 1;
		else if (!strcmp((*b)[i],"dos") || !strcmp((*b)[i],"windows"))
			dumconfig->dos_show = 1;
		else if (!strcmp((*b)[i],"mac"))
			dumconfig->mac_show = 1;
		else if (!strcmp((*b)[i],"bin"))
			dumconfig->binary_show = 1;
		else if (!strcmp((*b)[i],"dummy"))
			dumconfig->dummy_show = 1;
		else if (!strcmp((*b)[i],"all")) {
			dumconfig->unix_show   = 1;
			dumconfig->dos_show    = 1;
			dumconfig->mac_show    = 1;
			dumconfig->binary_show = 1;
			dumconfig->dummy_show  = 1;
		} else if (!strcmp((*b)[i],"endshow")) {
			(*a)--;(*b)++;
			return;
		}
		else return;
		(*a)--;(*b)++;
	}

}


void cmdto( int *a, char ***b ) {

	char tmp_file_name[4096];
	char buffer[TAM_LINE];
	int  app_me;
	register int i;
	struct stat stbuf;
	FILE *tmp_file;
	FILE *input_file;
	char output_type=0;

	enum LineEnd { UNIX='u',MAC='m',DOS='d' };

	(*a)--;(*b)++; // to

	if (!a||!*b||!**b)
		die("invalid 'to' usage!");

	/* sets conversion type */
	if (!strcmp(**b,"unix"))
		output_type = 'u';
	else if (!strcmp(**b,"mac"))
		output_type = 'm';
	else if (!strcmp(**b,"dos") || !strcmp(**b,"windows"))
		output_type = 'd';
	else
		die("invalid 'to' usage!");

	(*a)--;(*b)++; // type



	/* for each file... */
	for (i=0;i<(*a);i++) {


		/* defines tempname
		*/
		memset(tmp_file_name,0,4096);
		sprintf(tmp_file_name,"/tmp/dumtemp%d.txt",app_me);
		for (app_me=0;stat(tmp_file_name,&stbuf)==0 && app_me>=0;app_me++) {

			memset(tmp_file_name,0,4096);
			sprintf(tmp_file_name,"/tmp/dumtemp%d.txt",app_me);

		}
		if (app_me<0)
			die("OMG! How many temp files do you have? ... see /tmp/dumtemp*.txt");



		/* open tmp_file and input */
		tmp_file=fopen( tmp_file_name, "w" );
		if (!tmp_file)
			die( "I cannot open %s!\n",tmp_file_name);
		input_file = fopen((*b)[i],"r");
		if (!input_file)
			die( "I cannot open %s!\n",(*b)[i]);

		/* Trace if verbose
		*/
		if (dumconfig->verbose) {
			switch( output_type ){
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
		memset(buffer,0,TAM_LINE);
		while( fgets(buffer,TAM_LINE,input_file) ){

			delete_lineend(buffer);

			/* escribimos la linea al archivo de salida */
			switch( output_type ){
				case UNIX:
					fprintf( tmp_file, FMT_LINEA_UNIX, buffer );
					break;
				case DOS:
					fprintf( tmp_file, FMT_LINEA_DOS, buffer );
					break;
			}
			memset(buffer,0,TAM_LINE);
		}

		/* close files */
		fclose(input_file);
		fclose(tmp_file);

		/* reutilizamos el buffer de la linea para el comando que pone el
		+  temporal en el archivo original
		*/
		sprintf( buffer, "mv %s %s", tmp_file_name, (*b)[i] );
		if(system(buffer)){ die("Cannot move temporary file to the original one!\n"); }
	}

	exit(0);

}







/* This global, static array containing commands and functions -------------------------------------
 */
static const struct cmdst commands[] = {

	{ "help"               ,cmdhelp                },
	{ "colorful"           ,cmdcolorful            },
	{ "pretty"             ,cmdcolorful            },
	{ "showtime"           ,cmdshowtime            },
	{ "version"            ,cmdversion             },
	{ "verbose"            ,cmdverbose             },
	{ "analyze"            ,cmdanalyze             },
	{ "show"               ,cmdshow                },
	{ "to"                 ,cmdto                  },
	{ ""                   ,NULL                   }

};
static void execute_command( char* command, int *argc, char*** argv ){

	int executed = 0;
	char **cmdref=&command;

	if (!command)
		die("dum.c:%d  NULL POINTER NOT EXPECTED",__LINE__);

	const struct cmdst *p=commands;
	while( p->command[0] ) {
		if(!strcmp(p->command,command)) {
			p->execute( argc, argv );
			executed = 1;
		}
		p++;
	}
	
	if (!executed)
		cmdhelp(NULL,&cmdref);

}
/* ---------------------------------------------------------------------------------------------- */




int main(int argc, char **argv) {

	if (argc<2)
		cmdhelp(0,NULL);

	int j;

	/* Sets options global data structure
	 * as main() stack memory.
	 * Could be allocated in heap, but I like this way
	 */
	options_st dumconfig_main;
	dumconfig = &dumconfig_main;
	options_build(&dumconfig_main);

	argv++; // lost first
	argc--;

	for (;;) {

		if(!argc||!argv[0])
			break;

		if (dumconfig->parsing_args && argv[0][0]=='-') {

			for (j=1;j<strlen(argv[0]);j++)
				switch (argv[0][j]) {
					case '-':
						dumconfig->parsing_args = 0;
						break;
					case 'v':
					case 'V':
						dumconfig->verbose = 1;
						break;
					case 'h':
					case 'H':
						cmdhelp(0,NULL);
						break;
					case 'c':
					case 'C':
						dumconfig->colorful = 1;
						break;
					case 't':
					case 'T':
						dumconfig->log_time = 1;
						break;
					default:
						die("Bad option: %c (on %s)",
							argv[0][j],argv[0]);
						break;
				}

			argc--;argv++;

		}else{
			/* command processing - use it just as an access to functionalities */
			if( strlen(  argv[0]) > CMD_MAX )
				die("Command specified is too long, "
				    "current limit is %d",CMD_MAX);

			execute_command( *argv,  &argc,  &argv );
		}
	}

	return 0;
}



void delete_lineend(char * buffer){

	register int i=0;
	while( buffer[i]!=0 ){

		if( buffer[i] == EOL_CR || buffer[i] == EOL_LF ) buffer[i]=0;
		i++;

	}

}


int analyze_file(FILE* file_to_read,numeric_data *totals) {

	char buffer[TAM_LINE];
	memset(buffer, 0, TAM_LINE);
	totals->crlf=0L;
	totals->cr  =0L;
	totals->lf  =0L;

	/* read file */
	while( fgets(buffer,TAM_LINE,file_to_read) ){

		register int i=0;
		while( buffer[i] ){

			if     ( buffer[i]==EOL_CR && buffer[i+1]==EOL_LF ) {
				totals->crlf++;
				i++;
			}else if( buffer[i]==EOL_CR ){
				totals->cr++;
			}else if( buffer[i]==EOL_LF ){
				totals->lf++;
			}

			i++;
		}
		memset(buffer,0,TAM_LINE);
	}

	return 0;
}

int is_regfile (const char *fname) {

	struct stat stbuff;
	if (stat(fname,&stbuff))
		return 0;
	return S_ISREG(stbuff.st_mode)?1:0;

}

