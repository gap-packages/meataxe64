/*
 * Utilities for meataxe64
 *
 */

#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include "field.h"
#include "io.h"

char ***mk_tmps(const char *tmp, unsigned int rchops, unsigned int cchops)
{
  char ***tmpnames = malloc(sizeof(*tmpnames) * rchops);
  size_t len;
  uint64_t i, j;
  unsigned int k;

  len = strlen(tmp);
  for (i = 0; i < rchops; i++) {
    tmpnames[i] = malloc(sizeof(tmpnames[i]) * cchops);
    for (j = 0; j < cchops;j++) {
      /* Temporary names of the form tmp.<row>.<col>*/
      len += 3; /* Account for dots and EOS */
      /* Compute space for digits. We could also just over estimate at one digit per 3 bits, allowing 11 for a 32 bit word */
      k = rchops;
      while (0 != k) {
        len++;
        k /= 10;
      }
      k = cchops;
      while (0 != k) {
        len++;
        k /= 10;
      }
      tmpnames[i][j] = malloc(len);
      sprintf(tmpnames[i][j], "%s.%u.%u", tmp, (unsigned int)i, (unsigned int)j);
    }
  }
  return tmpnames;
}

void free_tmpnames(char ***names, unsigned int rchops, unsigned int cchops)
{
  unsigned int i, j;
  for(i= 0; i< rchops; i++) {
    for (j = 0; j < cchops; j++) {
      free(names[i][j]);
    }
    free(names[i]);
  }
  free(names);
}

/* stdio makes this extern */
int remove(const char *file)
{
  assert(NULL != file);
  return unlink(file);
}

static char buf[256];

const char *tmp_name(void)
{
  pid_t pid = getpid();
  sprintf(buf, "tmp%u", (unsigned int)pid);
  return buf;
}

#define TMP_FORMAT "%s_%d"
#define PRINTED_INT_LEN (3 * sizeof(unsigned int))
static unsigned int idx = 0;
/*
 * Function to make a new temporary name
 * We allocate space for the temporary root,
 * an undescore, a printed integer and a terminator
 */
char *mk_tmp(const char *name, const char *tmp_root, size_t tmp_len)
{
  char *tmp = malloc(tmp_len + PRINTED_INT_LEN + 2);
  if (NULL == tmp) {
    fprintf(stderr, "%s: failed to allocate memory for temporary file name \n", name);
    exit(1000);
  }
  sprintf(tmp, TMP_FORMAT, tmp_root, idx++);
  return tmp;
}

#ifdef NEVER
/* Very temporary chop size algorithm*/
#define CHOP_SIZE 0x10
unsigned int chop_size(const char *file)
{
  struct stat buf;
  int e;
  off_t size;
  unsigned int chops;

  /* TBD: determine file size */
  e = stat(file, &buf);
  if (e) {
    /* Failed to stat */
    LogString(82, "Cannot open input");
    exit(22);
  }
  size = buf.st_size; /* File size, guide to how much chopping */
  chops = size / CHOP_SIZE;
  if (0 == chops) {
    chops += 1;
  }
  return chops;
}
#endif
