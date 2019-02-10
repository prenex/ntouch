#ifndef _N_TOUCH_H
#define _N_TOUCH_H

/* ****************************************/
/* n-way touch header-only library        */
/* - Just #include "ntouch.h" to use      */
/* - See ntouch.c for usage examples      */
/* - Operations are not atomic when async */
/*                                        */
/* By: prenex                             */
/* See: http://github.com/prenex/ntouch   */
/*                                        */
/* Licence: unlicence                     */
/* ************************************** */

#include "stdio.h"
#include "string.h"
#include "dirent.h"
#include "stdlib.h"
#include "unistd.h"

/* **************** */
/* Config variables */
/* **************** */

#define MAX_OFILE_LEN 1023

/* ********* */
/* Functions */
/* ********* */

/** Real ANSI-C might not have strdup (maybe I am overly paranoid) - might return NULL, you might need to free */
char* my_strdup(char *src) {
	char *ret;
	int len;

	if(src == NULL) {
		return NULL;
	}

	len = strlen(src);
	if(len) {
		ret = malloc((size_t)len * sizeof(char));
		strcpy(ret, src);

		return ret;
	} else {
		return NULL;
	}
}

/** 
 * Opens a numbered file according to parameters. Numbering happens before extension if there is any.
 * This variant returns the output file name for the opened file, which you need to free() yourself!
 *
 * @param path_filename A complete /path/with/basename.ext (relative to '.' if only filename).
 * @param modulus Numbering turns around using this modulus (for log rotation and stuff).
 * @param insertno Numbering starts at this point - when there is files wth bigger numbers they are shifted like a list. -1 = "push_back".
 * @param ofname_ptr When not NULL, the full filename (might contain path when provided) of the opened file is returned - you must free() this!
 *
 * @returns File handle - or NULL in case of errors.
 */
FILE* ntouch_at_with_filename(char *path_filename, unsigned int modulus, int insertno, char **ofname_ptr) {
	/* Output file string handling vars */
	char outfilename[MAX_OFILE_LEN+1];
	FILE *outf;

	/* Current directory listing */
	DIR *d;
	struct dirent *entry;

	/* The %d part of the outfile_pattern for the current dir entry */
	int entrys_num;
	/* Final file number is being generated here */
	int filenum;

	/* Used for filling the pointer with the outfile name if needed */
	char *outfilename_copy;

	/* TODO: extract path properly */
	char *path = ".";
	char *filename = path_filename;

	/* TODO: create outfile_pattern */
	char *outfile_pattern = "test%d.txt";

	/* Calculate file num */
	filenum = 0;
	if(insertno == -1) {
		/* Create numbered output filename: name<filenum>[.ext] */
		d = opendir(path);
		/* See the biggest file entrys filenum corresponding to the pattern */
		if(d) {
			while((entry = readdir(d)) != NULL) {
				entrys_num = 0;
				sscanf(entry->d_name, outfile_pattern, &entrys_num);
				if(entrys_num >= filenum) {
					filenum = entrys_num + 1;
				}
			}
			closedir(d);
		}
	} else {
		filenum = insertno;
		/* TODO: handle shifting when using insertno - similar loop like above */
	}
	if(modulus > 0) filenum = filenum % modulus;

	/* Create the final filename */
	snprintf(outfilename, MAX_OFILE_LEN, outfile_pattern, filenum);

	/* TODO: Open file */
	outf = NULL; /* NULL indicates error */

	/* Return of the filename if used */
	if(ofname_ptr != NULL) {
		/* Copy is needed if we want to return the string as stack will go out scope */
		/* User free() is needed because of this */
		outfilename_copy = my_strdup((char*)outfilename);
		/* Fill their pointer to point for the duplicated string */
		*ofname_ptr = outfilename_copy;
	}

	/* Return opened file handle */
	return outf;
}

/** 
 * Opens a numbered file according to parameters. Numbering happens before extension if there is any.
 *
 * @param path_filename A complete /path/with/basename.ext (relative to '.' if only filename).
 * @param modulus Numbering turns around using this modulus (for log rotation and stuff).
 * @param insertno Numbering starts at this point - when there is files wth bigger numbers they are shifted like a list. -1 = "push_back".
 *
 * @returns File handle - or NULL in case of errors.
 */
FILE* ntouch_at(char *path_filename, unsigned int modulus, int insertno) {
	return ntouch_at_with_filename(path_filename, modulus, insertno, NULL);
}

/** 
 * Opens a numbered (unique) file. Numbering happens before extension if there is any.
 *
 * @param path_filename A complete /path/with/basename.ext (relative to '.' if only filename).
 *
 * @returns File handle - or NULL in case of errors.
 */
FILE* ntouch(char *path_filename) {
	return ntouch_at(path_filename, 0, 0);
}

/* vim: set ts=4 sw=4 noet : */
#endif /* _N_TOUCH_H */
