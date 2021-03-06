#ifndef _N_TOUCH_H
#define _N_TOUCH_H

/* ****************************************/
/* n-way touch header-only library        */
/* - Just #include "ntouch.h" to use      */
/* - See ntouch.c for usage examples      */
/* - Operations are not atomic when async */
/* - Operations are not thread safe       */
/*                                        */
/* By: prenex                             */
/* See: http://github.com/prenex/ntouch   */
/*                                        */
/* Licence: unlicence                     */
/* ************************************** */

#define _SVID_SOURCE /* Feature set macro for systemv imp of scanddir(..) */
#include "stdio.h"
#include "string.h"
#include "dirent.h"
#include "stdlib.h"
#include "unistd.h"
#include "libgen.h"

/* **************** */
/* Config variables */
/* **************** */

/* #define DEBUG_LOG 1 */
#define MAX_OFILE_LEN 1023

/* *************************** */
/* GLOBAL VARS - nonthreadsafe */
/* *************************** */

/** Used for my_filter and my_sorter */
static int my_filter_int;       /* nonthreadsafe */
/** Used for my_filter and my_sorter */
static const char *my_filter_pattern; /* nonthreadsafe */

/* ****************** */
/* Scanf parse helper */
/* ****************** */

/* BOOLE */
#ifndef BOOLE
#define BOOLE int
#endif /* BOOLE */
#ifndef TRUE
#define TRUE 1
#endif /* TRUE */
#ifndef FALSE
#define FALSE 0
#endif /* FALSE */
/* For human readable states */
#define STATE_SHIFT 1024
/* States: 
 * * -1: is the state when '%' is found
 * * 0: when only char comparisons are needed
 * * 'u': after %u (waiting for digits)
 * * 'd': after %d (waiting for digits or minus)
 * * 'd'+STATE_SHIFT: in %d (waiting for digits)
 * * 's': after %s (consuming until whitespace)
 */
/* TODO: As you see, only %u, %d, %s is implemented as of now! */
/**
 * Function to see if a string fully corresponds to scanf patterns.
 * Scanf can do its job halfway even if the string would not parse fully!
 */
BOOLE correspond_to_pattern(const char *str, const char *pattern) {
	const char *str_ptr, *pat_ptr;
	char strc, patc;
	int statevar = 0; /* 0: charcmp */

	str_ptr = str;
	pat_ptr = pattern;

	/* Simple state engine here */
	while((pat_ptr != NULL) && (str_ptr != NULL) && (
	          (((statevar == 0) || (statevar == -1)) && ((strc = *str_ptr) != 0) && ((patc = *pat_ptr) != 0))
	       || ((statevar > 0) && (((strc = *str_ptr) != 0) || ((patc = *pat_ptr) != 0)))
	     )) {
		if(statevar == 0) {
			/* 0: CHARCMP */
			if(patc == '%') {
				statevar = -1; /* '%' starts*/
				++pat_ptr;
			} else {
				/* charcmp fail? */
				if(patc != strc) return FALSE;
				++pat_ptr;
				++str_ptr;
			}
		} else if(statevar == -1) {
			/* -1: After '%' in pattern */
			if(patc == '%') {
				/* %% to escape '%' char in pattern */
				statevar = 0;
			} else if(patc == 'u') {
				statevar = 'u';
				++pat_ptr;
			} else if(patc == 'd') {
				statevar = 'd' + STATE_SHIFT;
				++pat_ptr;
			} else if(patc == 's') {
				statevar = 's';
				++pat_ptr;
			}
		} else {
			/* Pattern-states - ONLY MOVE THE STR POINTER AND STATE!!! */
			switch(statevar) {
				case 'u':
					if(isdigit(strc)) ++str_ptr;
					else statevar = 0;
					break;
				case 'd':
					if(isdigit(strc) || (strc == '-') || (strc == '+')){
					   	++str_ptr;
						statevar = 'd'+STATE_SHIFT;
					} else statevar = 0;
					break;
				case 'd'+STATE_SHIFT:
					if(isdigit(strc)) ++str_ptr;
					else statevar = 0;
					break;
				case 's':
					/* TODO: maybe whitespaces are handled nonstandard by me? */
					if(isspace(strc)) statevar = 0;
					else ++str_ptr;
					break;
			}
		}
	}

	/* Handle the "prefix of the other" - kind of cases both ways */
	if(pat_ptr != NULL) {
		if(str_ptr == NULL){
			/* pattern is null, but string is not */
			return FALSE;
		} else {
			/* One is prefix non-empty of other? */
			/* BEWARE: patc and strc are bad here as assignments are in conditions and would fail in edge cases !!! */
			if(*pat_ptr != *str_ptr) {
				/* Only possible when one is the zero terminator here */
				return FALSE;
			}
		}
	} else if(str_ptr != NULL) {
		/* pattern is null, but string is not */
		return FALSE;
	}

	return TRUE;
}

/* ***************** */
/* Private functions */
/* ***************** */

/** 
 * Helper function for get_shift_untilno.
 * Uses global vars in non-threadsafe way.
 */
static int my_filter(const struct dirent *entry) {
	int ret, entrys_num = -1;
	ret = sscanf(entry->d_name, my_filter_pattern, &entrys_num);
	if(!correspond_to_pattern(entry->d_name, my_filter_pattern)) return FALSE;
	ret = (entrys_num >= my_filter_int);
	return ret;
}

/** 
 * Helper function for get_shift_untilno.
 * Uses global vars in non-threadsafe way.
 *
 * Rem.: This function only works if my_filter was used before sorting.
 */
static int my_sort(const struct dirent **a, const struct dirent **b) {
	int numa, numb;
	sscanf((*a)->d_name, my_filter_pattern, &numa);
	sscanf((*b)->d_name, my_filter_pattern, &numb);
	if(numa == numb) {
		return 0;
	} else if(numa < numb) {
		return 1;
	} else {
		return -1;
	}
}

/**
 * Helper function for shift_files_like.
 *
 * @returns -1 if no shift-mv is necessary, otherwise the index of the first bigger-than filenum numbered index with no successor!
 */
static int get_shift_untilno(const char *path, const char *filename_pattern, const int filenum) {
	struct dirent **namelist;
	int n, statevar, untilno, next_untilno;

	/* Set up sort and filter function vars */
	/* These are here to simulte a lambda for scandir below */
	my_filter_int = filenum;
	my_filter_pattern = filename_pattern;

	n = scandir(path, &namelist, &my_filter, &my_sort);
	if (n < 0) {
		perror("scandir");
	} else {
		/* Small state engine */
		statevar = 0; /* 0: Unsure if filenum is used or not */
		while (n--) {
			sscanf(namelist[n]->d_name, filename_pattern, &next_untilno);
			if(statevar == 0) {
				if(next_untilno != filenum) {
					statevar = 1; /* 1: The filenum is unused */
				} else {
					statevar = 2; /* 2: The filenum is used, untilno is unsure */
					untilno = next_untilno;
				}
			} else if(statevar == 2) {
				if(next_untilno != untilno + 1) {
					/* Found a hole or end of numbers */
					statevar = 3; /* 3: The filenum is used, untilno surely found */
				} else {
					/* Still haven't found the hole or end */
					untilno = next_untilno;
				}
			}
			free(namelist[n]);
		}
		free(namelist);
	}

	/* 2+ means that target filename was used and shift is necessary! */
	if(statevar < 2) {
		return -1;
	} else {
		return untilno;
	}
}

/** Shift (mv around) numbered files with filenum greater than the parameter */
static void shift_files_like(const char *path, const char *filename_pattern, int filenum) {
	int fn_untilno;
	char srcfilename[MAX_OFILE_LEN+1];
	char dstfilename[MAX_OFILE_LEN+1];

	/* See if shifting is needed or not */
	fn_untilno = get_shift_untilno(path, filename_pattern, filenum);

	/* Do the shifting - this handles the -1 case too */
	while((fn_untilno >= 0) && (fn_untilno >= filenum)) {
		/* Generate src and dst filenames */
		snprintf(srcfilename, MAX_OFILE_LEN, filename_pattern, fn_untilno);
		snprintf(dstfilename, MAX_OFILE_LEN, filename_pattern, fn_untilno + 1);

		/* Do the mv operation: rename is usable as we are in the same directory and same fs */
		rename(srcfilename, dstfilename);
#ifdef DEBUG_LOG
		fprintf(stderr, "SHIFT: %s -> %s\n", srcfilename, dstfilename);
#endif /* DEBUG_LOG */

		/* Back to front: otherwise we would override files */
		--fn_untilno;
	}
}

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

	/* %u */
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
		ret = malloc((len+1) * sizeof(char));
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
 * BEWARE: When modulus is not zero and insertno is not -1, SHIFTING WILL NOT HAPPEN!
 *
 * @param path_filename A complete /path/with/basename.ext (relative to '.' if only filename).
 * @param modulus Numbering turns around using this modulus (for log rotation and stuff).
 * @param insertno Numbering starts at this point - when there is files wth bigger numbers they are shifted like a list. -1 = "push_back".
 * @param ofname_ptr When not NULL, the full filename (might contain path when provided) of the opened file is returned - you must free() this!
 *
 * @returns File handle - or NULL in case of errors.
 */
FILE* ntouch_at_with_filename(char *path_filename, unsigned int modulus, int insertno, char **ofname_ptr) {
	/* Generic temporal int */
	int i;
	/* For string len calculations */
	int patlen, fnlen;
	/* We need to separate the path from filename to get directory listing and name generation */
	char *path, *filename;
	/* Needed for dirname and basename: as they modify their arguments */
	char *patc, *basc;

	/* Output file string handling vars */
	char outfilename[MAX_OFILE_LEN+1];
	/* Output file handle */
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
	char *fullfilename_copy;

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
				i = sscanf(entry->d_name, outfile_pattern, &entrys_num);
				if(correspond_to_pattern(entry->d_name, outfile_pattern)){
					if(entrys_num >= filenum) {
						filenum = entrys_num + 1;
					}
				}
			}
			closedir(d);
		}
	} else {
		filenum = insertno;
		/* 0 means there is no logrotation mode, in which case overwriting is more logical! */
		if(modulus == 0) {
			shift_files_like(path, outfile_pattern, filenum);
		}
	}
	if(modulus > 0) filenum = filenum % modulus;

	/* Create the final filename */
	snprintf(outfilename, MAX_OFILE_LEN, outfile_pattern, filenum);

	/* Construct full path once again - now with numbers */
	/* Copy is needed if we want to return the string as stack will go out scope */
	/* User free() is needed because of this */
	/* Allocate*/
	patlen = strlen(path);
	fnlen = strlen(outfilename);
	fullfilename_copy = malloc((patlen + 1 + fnlen + 1) * sizeof(char));
	/* Fill in with data */
	strcpy(fullfilename_copy, path);
	fullfilename_copy[patlen] = '/';
	strcpy(fullfilename_copy+patlen+1, outfilename);

	/* "Return" the filename through the pointer if provided */
	if(ofname_ptr != NULL) {
		/* Fill their pointer to point for the duplicated string */
		*ofname_ptr = fullfilename_copy;
	}

	/* Open file */
	outf = fopen(fullfilename_copy, "w+");

	/* Cleanup */
	free(outfile_pattern);
	free(basc);
	free(patc);

	/* Return opened file handle */
	return outf;
}

/** 
 * Opens a numbered file according to parameters. Numbering happens before extension if there is any.
 *
 * BEWARE: When modulus is not zero and insertno is not -1, SHIFTING WILL NOT HAPPEN!
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
