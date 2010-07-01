#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CMD_MAX        512
#define TAM_LINEA      4096
#define EOL_CR         0x0D
#define EOL_LF         0x0A
#define FMT_LINEA_UNIX "%s\x0A"
#define FMT_LINEA_MAC  "%s\x0D"
#define FMT_LINEA_DOS  "%s\x0D\x0A"

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
"     set verbose      sets verbose mode on -same as 'v' flag-\n"
"     set full-output  sets date/time tracing on log strings\n"
"     set colorful     sets colorful mode on -same as 'c' flag-\n\n"
"  'knife' commands (at the end):\n"
"     to unix          rewrite input files using UNIX LF byte\n"
"     to dos           rewrite input files using DOS/WINDOWS CR+LF bytes\n"
"     to mac           rewrite input files using MAC's CR byte\n"
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

void cambiar_fin_linea (char * buffer_linea);
int analyze_file (FILE* archivo_lect);



/* structure to save global configuration data, and functions --------------------------------------
 */
typedef struct options_st {

	unsigned char  verbose;
	unsigned char  colorful;
	unsigned char  parsing_args;
	unsigned char  log_time;
	FILE          *log_file;

} options_st;

options_st *dumconfig = NULL;

void options_build(options_st *ref) {
	ref->verbose      = 0;
	ref->colorful     = 0;
	ref->parsing_args = 1;
	ref->log_time     = 0;
	ref->log_file     = NULL;
}

void options_reset(options_st *ref) {
	ref->verbose      = 0;
	ref->colorful     = 0;
	ref->parsing_args = 1;
	ref->log_time     = 0;
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

void cmdset( int *a, char ***b ) {

	(*a)--;(*b)++;
	if (!a||!*b||!**b)
		die("invalid set usage!");

	if (!strcmp(**b,"verbose")) {
		dumconfig->verbose = 1;
		(*a)--;(*b)++;
	}

	else if (!strcmp(**b,"colorful")) {
		dumconfig->colorful = 1;
		(*a)--;(*b)++;
	}

	else if (!strcmp(**b,"full-output")) {
		dumconfig->log_time = 1;
		(*a)--;(*b)++;
	}

	else
		die("invalid set usage!");

}







/* This global, static array containing commands and functions -------------------------------------
 */
static const struct cmdst commands[] = {

	{ "help"               ,cmdhelp                },
	{ "version"            ,cmdversion             },
	{ "set"                ,cmdset                 },
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
			//argc-=i; argv+=i;
			execute_command( *argv,  &argc,  &argv );
		}
	}

	return 0;
}



void cambiar_fin_linea(char * buffer_linea) {

	register int i=0;
	while( buffer_linea[i]!=0 ){

		if( buffer_linea[i] == EOL_CR || buffer_linea[i] == EOL_LF ) buffer_linea[i]=0;
		i++;

	}

}


int analyze_file(FILE* archivo_lect) {

	char buffer_linea[TAM_LINEA];
	memset(buffer_linea, 0, TAM_LINEA);
	unsigned long count_crlf=0L;
	unsigned long count_cr  =0L;
	unsigned long count_lf  =0L;


	/* read file */
	while( fgets(buffer_linea,TAM_LINEA,archivo_lect) ){

		register int i=0;
		while( buffer_linea[i] ){

			if     ( buffer_linea[i]==EOL_CR && buffer_linea[i+1]==EOL_LF ) {
				count_crlf++;
				i++;
			}else if( buffer_linea[i]==EOL_CR ){
				count_cr++;
			}else if( buffer_linea[i]==EOL_LF ){
				count_lf++;
			}

			i++;
		}
		memset(buffer_linea,0,TAM_LINEA);
	}

	/* statistics */
	if      (count_cr==0L && count_lf==0L && count_crlf==0L)
		printf("NO-LINE-ENDS\n");
	else if (count_cr==0L && count_lf==0L && count_crlf>0L)
		printf("DOS/WINDOWS\n");
	else if (count_cr>0L  && count_lf==0L && count_crlf==0L)
		puts("MAC");
	else if (count_cr==0L && count_lf>0L  && count_crlf==0L)
		puts("UNIX");
	else {
		unsigned long sum=count_cr+count_lf+count_crlf;
		puts("BIN");
		if (dumconfig->verbose){
			printf("Has %ld%% UNIX, %ld%% DOS/WINDOWS, %ld%%MAC line ending sequences over %ld total\n",
		                             (count_lf*100)/sum,
		                             (count_crlf*100)/sum,
		                             (count_cr*100)/sum,
		                             sum);
		}
	}

	return 0;
}

