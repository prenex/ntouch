/* ****************************************/
/* n-way touch application                */
/* - also example for ntouch.h            */
/*                                        */
/* By: prenex                             */
/* See: http://github.com/prenex/ntouch   */
/*                                        */
/* Licence: unlicence                     */
/* ************************************** */

#include "ntouch.h"

#include "stdio.h"
#include "string.h"
#include "ctype.h"

/* **************** */
/* Config variables */
/* **************** */

const char *help_str = "--help";
const char *lr_str = "-lr";

/* ***************** */
/* Dynamic variables */
/* ***************** */

/* **** */
/* Code */
/* **** */

/* Prints "--help" */
void helptext() {
	printf("Usage:\n");
	printf("------\n");
	printf("\n");
	printf("ntouch --help          # Shows help message\n");
	printf("ntouch                 # Shows help message\n");
	printf("ntouch alma.txt        # Creates alma0.txt - if exists creates alma1.txt instead etc.\n");
	printf("ntouch alma.txt 4      # Inserts alma4.txt as empty - shift earlier as alma5.txt etc.\n");
	printf("ntouch -lr 3 asdf.log  # Logrotate: asdf0.log .. asdf2.log ... asdf0.log ...\n");
	printf("ntouch -lr 3 a.log 2   # Logrotate with insert: mix of -lr and the insert num.\n");
	printf("\n");
	printf("Remarks:\n");
	printf("--------\n");
	printf("\n");
	printf("* We return 0 for success and return 1 in case of errors!\n");
	printf("* On success the operation prints the filename to stdout so you know what got created!\n");
	printf("* All parameters should be non-negative and -lr param cannot be zero!\n");
	printf("* The order of parameters do count! It cannot be anything just the above ones!\n");
	printf("* The numbering always happen BEFORE the last dot in the filename (bf extension) \n");
	printf("  or at the end of the file if there is no 'file extension'!\n");
	printf("\n");
}

/* When -1 or 0 is returned, you should exit(0), otherwise there was an error! */
int handleparams(int argc, char **argv, int *logrno, int *insertno, char **targetfilename) {
	/* Default operation parameters */
	/* 0 means the same as if there was no -lr */
	*logrno = 0;
	/* -1 means "push_back" insertion */
	*insertno = -1;

	if(argc > 1) {
		/* simplest case */
		if(strcmp(help_str, argv[1]) == 0) {
			/* --help */
			helptext();
			/* -1 means we will return 0 from the main prog */
			return -1;
		} else if(strcmp(lr_str, argv[1]) == 0){
			/* --lr */
			if((argc <= 3) || !isdigit(argv[2][0])) {
				/* ERR */
				helptext();
				return 1;
			} else {
				/* ! logrno */
				sscanf(argv[2], "%d", logrno);
				if(*logrno < 1) {
					/* ERR */
					helptext();
					return 1;
				}
				if(argc == 5) {
					/* ! insertno */
					sscanf(argv[4], "%d", insertno);
				} else if(argc != 4){
					/* ERR */
					helptext();
					return 1;
				}
				/* ! targetfilename */
				*targetfilename = argv[3];
			}
		} else if((argc == 3) && isdigit(argv[2][0])){
			/* No --lr, but 3 parameters and last is number */
			/* ! insertno */
			sscanf(argv[2], "%d", insertno);
			/* ! targetfilename */
			*targetfilename = argv[1];
		} else {
			/* ! targetfilename */
			*targetfilename = argv[1];
		}
	} else {
		/* ERR */
		helptext();
		return 1;
	}

	/* OK */
	return 0;
}

int main(int argc, char **argv) {
	/* Target filename */
	char *targetfilename;
	/* Output filename */
	char *outfilename;
	/* Output file handle */
	FILE *outf;

	/* Logrotate num */
	int logrno;

	/* Insertion point num */
	int insertno;

	/* Return values */
	int ret;

	/* Handle parameters */
	ret = handleparams(argc, argv, &logrno, &insertno, &targetfilename);
	printf("ntouch_at_with_filename(%d %d, ..)\n", logrno, insertno);
	if(ret != 0) {
		if(ret == -1) {
			/* EXIT */
			return 0;
		} else {
			/* EXIT */
			return ret;
		}
	}

	/* Call into ntouch.h depending on parameters */
	/* TODO: Remove debug code here */
	printf("ntouch_at_with_filename(%s %d %d, ..)\n", targetfilename, logrno, insertno);
	outf = ntouch_at_with_filename(targetfilename, logrno, insertno, &outfilename);
	ret = (outf != NULL) ? 0 : 2;

	/* Tell the user (code) what file to open */
	if(ret == 0) {
		printf("%s\n", outfilename);
	}

	/* Cleanup - and write so that scripts can use the file for writing */
	if(outf != NULL) {
		fclose(outf);
	}
	/* If ret was -1 this might(?) fail, so the cleanup order counts here! */
	if(outfilename != NULL) free(outfilename);

	/* EXIT - showing there was problem if we got here */
	return ret;
}

/* vim: set ts=4 sw=4 noet : */
