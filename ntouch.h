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
#include "libgen.h"

/* **************** */
/* Config variables */
/* **************** */

#define MAX_OFILE_LEN 1023

/* ***************** */
/* Private functions */
/* ***************** */

/** Adds a %d at the proper position: before last dot or at the end. This uses a malloc copy, so you must use free() */
static char* gen_filename_pattern(const char *fn) {
	char *filename_pattern;
	int name_until;
	int newlen;
	int strend;
	int j;
	/* -1 means there was no extension (*) */
	int ext_from = -1;
	int	i = 0;

	/* Find the name-extension boundary */
	/* i becomes: len */
	/* name_until becomes: index of '.' or EOS */
	/* ext_from becomes: index where extension starts or -1 */
	while(fn[i] != 0) {
		if((fn[i] == '.') && (fn[i+1] != 0)) {
			ext_from = i+1;
			name_until = i;
		}
		++i;
	}
	strend = i;
	if(ext_from == -1) {
		name_until = strend;
	}

	/* alma.txt or alma */
	/* Calculate new length for the returned string */
	if(ext_from < 0) {
		newlen = name_until + 2 + 1;
		/*           name   "%d"  0 */
	} else {
		newlen = name_until + 2 + 1 + (strend - ext_from) + 1;
		/*           name   "%d" '.'       ext              0 */
	}

	/* Allocate memory for the final pattern */
	filename_pattern = (char*)malloc(newlen * sizeof(char));

	/* Construct pattern */
	/* Name */
	for(i = 0; i < name_until; ++i) {
		filename_pattern[i] = fn[i];
	}

	/* %d */
	filename_pattern[i++] = '%';
	filename_pattern[i++] = 'd';

	/* Extension */
	if(ext_from != -1) {
		filename_pattern[i++] = '.';
		for(j = ext_from; j < strend; ++j) {
			filename_pattern[i++] = fn[j];
		}
	}

	/* Zero-terminate */
	filename_pattern[i] = 0;

	/* RETURN */
	return filename_pattern;
}

/** Real ANSI-C might not have strdup (maybe I am overly paranoid) - might return NULL and you might need to free */
static char* my_strdup(char *src) {
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

/* **************** */
/* Public functions */
/* **************** */

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
	/* We need to separate the path from filename to get directory listing and name generation */
	char *path, *filename;
	/* Needed for dirname and basename: as they modify their arguments */
	char *patc, *basc;

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

	char *outfile_pattern;
	/* Used for filling the pointer with the outfile name if needed */
	char *outfilename_copy;

	/* extract path properly */
	patc = my_strdup(path_filename);
	basc = my_strdup(path_filename);
	path = dirname(patc);
	filename = basename(basc);

	/* Create outfile_pattern */
	outfile_pattern = gen_filename_pattern(filename);

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

	/* TODO: add path to outfile_pattern */
	/* Create the final filename */
	snprintf(outfilename, MAX_OFILE_LEN, outfile_pattern, filenum);

	/* Open file */
	outf = fopen(outfilename, "w+");

	/* Return of the filename if used */
	if(ofname_ptr != NULL) {
		/* Copy is needed if we want to return the string as stack will go out scope */
		/* User free() is needed because of this */
		outfilename_copy = my_strdup((char*)outfilename);
		/* Fill their pointer to point for the duplicated string */
		*ofname_ptr = outfilename_copy;
	}

	/* Cleanup */
	free(outfile_pattern);

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
