/*
 * Utilities for meataxe64
 *
 */

#ifndef included__util
#define included__util
#include <stddef.h>

/* Produces a 2 dimensional array of temporary names */
extern char ***mk_tmps(const char *tmp, unsigned int rchops, unsigned int cchops);

/* Frees a 2 dimensional array of temporary names */
extern void free_tmpnames(char ***names, unsigned int rchops, unsigned int cchops);

/* Create a temporary name root unique to this process */
extern const char *tmp_name(void);

/* Temporary chop computer. We need something much better in the fullness of time */
extern unsigned int chop_size(const char *file);

/* Function to make a new temporary name */
extern char *mk_tmp(const char *name, const char *tmp_root, size_t tmp_len);

#endif
