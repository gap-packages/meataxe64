/*
      zis.c     meataxe-64 invariant subspace
      =====     J. G. Thackray   04.10.2017
                updated 28.1.19 RAP allow perms as generators
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "mfuns.h"
#include "io.h"
#include "slab.h"
#include "funs.h"
#include "bitstring.h"
#include "util.h"

/*
 * temporary files
 * We need the following
 * A pair of files for the echelised vectors that need no further multiplies
 * Initially these will be NULL. They have suffices .{bs,rem}
 * A pair of files per generator representing the cleaned
 * and echelised result from the last applicatoin of that generator
 * A further file per generator giving the NREF expanded form
 * of these two files
 * The image of each of these files under the current generator being applied
 *
 * These sets are held in a circular list, along with information
 * saying how many vectors is in each. Initially the numbers will be 0
 */

/* The multiplication results, concatenated */
struct mult_result {
  char *bs; /* The bitstring */
  char *rem; /* The remainder */
  char *plain; /* The NREF unrolled form for multiplying */
  uint64_t size; /* How many vectors */
};

typedef struct mult_result mult_result;

/* A generator */
struct gen {
  const char *file;
  mult_result next_tbd;
  struct gen *next;
};

typedef struct gen gen;

static const char prog_name[] = "zis";

static char *fun_tmp;

/* Statics for clean operation */
static char **clean_vars;

/*
 * The clean operation
 * Takes a bitstring with t bits and rem with m columns
 * and a matrix rows with t+m columns
 * and cleans to a matrix with m columns
 */
static void clean(const char *bs, const char *rem, const char *rows, const char *out)
{
  unsigned int i;
  /* Extract pivots from rows using bs giving tmp1 */
  /* Also produce tmp3 = nonsel(bs, rows) */
  fColumnExtract(bs, 1, rows, 1, clean_vars[0], 1, clean_vars[2], 1);
  /* Produce tmp2 = tmp1 * rem */
  fMultiply(fun_tmp, clean_vars[0], 1, rem, 1, clean_vars[1], 1);
  /* Produce out = tmp3 + tmp2 */
  fAdd(clean_vars[2], 1, clean_vars[1], 1, out, 1);
  /* And delete the temporaries */
  for (i = 0; i < 3; i++) {
    remove(clean_vars[i]);
  }
}

#define FUN_TMP "_funs"

int main(int argc, const char *argv[])
{
  /* Set up tmp stem tmp_pid*/
  const char *tmp_root = tmp_name();
  /* How long the temporary filename root is */
  size_t tmp_len = strlen(tmp_root);
  unsigned int i, j; /* Loop variables */
  const char *in_vecs;
  const char *out_stem;
  char *out_bs;
  char *out_rem;
  size_t out_stem_len;
  /* Echelised needing no more multiplies */
  char *mult_result_bs = mk_tmp(prog_name, tmp_root, tmp_len);
  char *mult_result_rem = mk_tmp(prog_name, tmp_root, tmp_len);
  /* The result of the initial vectors echelisation */
  /* This also initalises the leading zeros */
  char *zero_bs = mk_tmp(prog_name, tmp_root, tmp_len);
  char *in_vecs_rem = mk_tmp(prog_name, tmp_root, tmp_len);
  char *in_vecs_bs = mk_tmp(prog_name, tmp_root, tmp_len);
  char *mul_tmp = mk_tmp(prog_name, tmp_root, tmp_len);
  char *ech_tmp_bs = mk_tmp(prog_name, tmp_root, tmp_len);
  char *ech_tmp_rem = mk_tmp(prog_name, tmp_root, tmp_len);
  char *ech_tmp_bs1 = mk_tmp(prog_name, tmp_root, tmp_len);
  char *ech_tmp_rem1 = mk_tmp(prog_name, tmp_root, tmp_len);
  char *ech_tmp_bsc = mk_tmp(prog_name, tmp_root, tmp_len);
  char *ech_tmp_bsr = mk_tmp(prog_name, tmp_root, tmp_len);
  char *clean_tmp1 = mk_tmp(prog_name, tmp_root, tmp_len);
  char *clean_tmp2 = mk_tmp(prog_name, tmp_root, tmp_len);
  int ngens = argc - 3;
  uint64_t res;
  uint64_t nor = 0;
  uint64_t fdef = 0;
  uint64_t rank, mrank = 0;
  uint64_t hdr[5];
  gen *gens, *this_gen;
  CLogCmd(argc, argv);
  /* Check command line <vecs> <output stem> [<gen>*] */
  /* Must be at least 3 args */
  if (argc < 4) {
    LogString(80,"usage zis <seeds> <output stem> [<generator>*]");
    exit(21);
  }
  in_vecs = argv[1];
  out_stem = argv[2];
  out_stem_len = strlen(out_stem);
  /* Temporary root for functions */
  fun_tmp = malloc(tmp_len + sizeof(FUN_TMP) + 1);
  strcpy(fun_tmp, tmp_root);
  strcat(fun_tmp, FUN_TMP);
  /* Echelise initial vecs. Also sets up zero_bs */
  res = fProduceNREF(fun_tmp, in_vecs, 0, zero_bs, 1, in_vecs_rem, 1);
  /* fail if rank 0 */
  if (0 == res) {
    /* Given zero space to spin, give up */
    fprintf(stderr, "%s: %s has rank zero, not spinning\n", prog_name, in_vecs);
    remove(zero_bs);
    remove(in_vecs_rem);
    exit(10);
  }
  rank = res;
  /* Reread to get a second copy of zero_bs: Fudge */
  fProduceNREF(fun_tmp, in_vecs, 1, in_vecs_bs, 1, in_vecs_rem, 1);
  /* Allocate the temporaries for the clean operation */
  clean_vars = malloc(3 * sizeof(*clean_vars));
  for (i = 0; i < 3; i++) {
    clean_vars[i] = mk_tmp(prog_name, tmp_root, tmp_len);
  }
  EPeek(argv[1], hdr);    // get field from vectors
  fdef = hdr[1];
  /* Now check all the generators, all square, same size */
  for (i = 0; i < ngens; i++) {

    EPeek(argv[i + 3], hdr);
    if (0 == i) {
      nor = hdr[2];
      if (hdr[3] != nor) {
        fprintf(stderr, "%s: cannot spin with non square matrix %s\n", prog_name, argv[i + 3]);
        exit(20);
      }
    } else {
      if ( nor != hdr[2] || nor != hdr[3] ) {
        fprintf(stderr, "%s: cannot spin with incompatible matrix %s\n", prog_name, argv[i + 3]);
        exit(20);
      }
    }
  }
  /* Set up gen structures */
  gens = malloc(ngens * sizeof(*gens));
  for (i = 0; i < ngens; i++) {
    gens[i].file = argv[i + 3];
    gens[i].next_tbd.bs = mk_tmp(prog_name, tmp_root, tmp_len);
    gens[i].next_tbd.rem = mk_tmp(prog_name, tmp_root, tmp_len);
    gens[i].next_tbd.plain = mk_tmp(prog_name, tmp_root, tmp_len);
    gens[i].next_tbd.size = 0; /* Nothing assigned for multiply yet */
    gens[i].next = gens + ((i + 1) % ngens); /* A circular list */
  }
  /*
   * Put the echelised result into the last gen.
   * Once multiplied by that it can form the first part
   * of the "multiplied by everything" result
   */
  make_plain(NULL, in_vecs_bs, in_vecs_rem, gens[ngens - 1].next_tbd.plain, fdef);
  rename(in_vecs_rem, gens[ngens - 1].next_tbd.rem);
  rename(in_vecs_bs, gens[ngens - 1].next_tbd.bs);
  gens[ngens - 1].next_tbd.size = rank;
  this_gen = gens; /* The first one */
  while (mrank < rank && rank < nor) {
    /* Still some stuff to multiply */
    gen *mul_gen = this_gen;
    int first = 1; /* No results for this gen yet */
    uint64_t extra_rank = 0;
    /* Do the current gen for all results, starting with its own */
    for (i = 0; i < ngens; i++) {
      gen *clean_gen = this_gen;
      if (0 != mul_gen->next_tbd.size) {
        /* Something to do for the last result from this generator */
        fMultiply(fun_tmp, mul_gen->next_tbd.plain, 1, 
                  this_gen->file, 1, mul_tmp, 1);
        if (0 != mrank) {
          /* Clean this result with the overall multiplied stuff */
          clean(mult_result_bs, mult_result_rem, mul_tmp, clean_tmp1);
        } else {
          rename(mul_tmp, clean_tmp1);
        }
        /* Now clean with all the results awaiting multiply, starting with gen */
        for (j = 0; j < ngens; j++) {
          /* Clean with clean_gen->mult_result.{bs,rem}, if it exists */
          if (0 != clean_gen->next_tbd.size) {
            clean(clean_gen->next_tbd.bs, clean_gen->next_tbd.rem, clean_tmp1, clean_tmp2);
            /* Now move tmp2 to tmp1 */
            rename (clean_tmp2, clean_tmp1);
          }
          /* Next generator */
          clean_gen = clean_gen->next;
        }
        /* Now clean with previous results of this round of multiply */
        if (first) {
          /* Just echelise this */
          res = fProduceNREF(fun_tmp, clean_tmp1, 1, ech_tmp_bs, 1, ech_tmp_rem, 1);
          if (0 != res) {
            first = 0;
          } else {
            remove(ech_tmp_bs);
            remove(ech_tmp_rem);
          }
          extra_rank += res;
        } else {
          /* Clean with previous echelised */
          clean(ech_tmp_bs, ech_tmp_rem, clean_tmp1, clean_tmp2);
          /* Then echelise */
          res = fProduceNREF(fun_tmp, clean_tmp2, 1, ech_tmp_bs1, 1, ech_tmp_rem1, 1);
          if (0 != res) {
            /* Then back clean ech_tmp_rem with ech_tmp_{bs,rem}1*/
            clean(ech_tmp_bs1, ech_tmp_rem1, ech_tmp_rem, clean_tmp1);
            /* Then join as if echelised all together */
            /* First combine pivots and get a riffle */
            fPivotCombine(ech_tmp_bs, 1, ech_tmp_bs1, 1, ech_tmp_bsc, 1, ech_tmp_bsr, 1);
            /*
             * Now do a riffle controlled by ech_tmp_bsr
             * Selected rows is those resulting from the back clean
             * Non selected rows is those we back cleaned with
             */
            fRowRiffle(ech_tmp_bsr, 1, clean_tmp1, 1, ech_tmp_rem1, 1, ech_tmp_rem, 1);
            /* Now delete/rename */
            /* The combined pivots are the new ech result */
            rename(ech_tmp_bsc, ech_tmp_bs);
            /* The row riffle is no longer needed */
            remove(ech_tmp_bsr);
            /* These two were only ever temporary */
            remove(ech_tmp_rem1);
            remove(ech_tmp_bs1);
          }
          remove(ech_tmp_bs1);
          remove(ech_tmp_rem1);
          extra_rank += res;
        }
        remove(clean_tmp1);
        remove(clean_tmp2);
      }
      /* Move on to the next */
      mul_gen = mul_gen->next;
    }
    /* Add in rank */
    rank += extra_rank;
    /* TBD: is this all in the right order? */
    /*
     * Stuff to be manipulated
     * mult_result_bs: the pivot columns of the multiplied result
     * this_gen->next_tbd.bs the pivots of what this gen last produced
     * ech_tmp_bsc: the combination of the above two, replacing mult_result_bs
     * zero_bs: the zeros so far in what we've cleaned
     * ech_tmp_bs: the pivots of what we've just produced
     * the new zero_bs comes from the above two
     * The old zero_bs is placed with the new rows just produced
     */
    if (0 != extra_rank) {
      /* We can make the plain form for the next multiply */
      make_plain(zero_bs, ech_tmp_bs, ech_tmp_rem, this_gen->next_tbd.plain, fdef);
    }
    if (0 != this_gen->next_tbd.size) {
      /* Update the vectors multiplied if this generator has any */
      if (0 != mrank) {
        /* Back clean */
        clean(this_gen->next_tbd.bs, this_gen->next_tbd.rem, mult_result_rem, clean_tmp1);
        /* Combine pivots, getting new pivots and row riffle */
        fPivotCombine(mult_result_bs, 1, this_gen->next_tbd.bs, 1, ech_tmp_bsc, 1, ech_tmp_bsr, 1);
        /* Riffle rows */
        fRowRiffle(ech_tmp_bsr, 1, clean_tmp1, 1, this_gen->next_tbd.rem, 1, mult_result_rem, 1);
        /* Renames and deletes */
        rename(ech_tmp_bsc, mult_result_bs);
      } else {
        /* First fully multiplied vectors */
        rename(this_gen->next_tbd.bs, mult_result_bs);
        rename(this_gen->next_tbd.rem, mult_result_rem);
      }
    }
    if (0 != extra_rank) {
      /* Update this gen for results of multiply, and zero_bs */
      fPivotCombine(zero_bs, 1, ech_tmp_bs, 1, ech_tmp_bsc, 1, ech_tmp_bsr, 1);
      /* ech_tmp_bsc is the new zero_bs */
      rename(ech_tmp_bsc, zero_bs);
      /* Don't need the row riffle this time */
      remove(ech_tmp_bsr);
      /* Update this generator's tbd */
      rename(ech_tmp_bs, this_gen->next_tbd.bs);
      rename(ech_tmp_rem, this_gen->next_tbd.rem);
    }
    /* Update mrank with size we just multiplied */
    mrank += this_gen->next_tbd.size;
    /* Need to update size even if zero */
    this_gen->next_tbd.size = extra_rank;
    /* Move on to next generator */
    this_gen = this_gen->next;
  }
  /* Even if we get whole space, we'll still produce a bs and rem */
  out_bs = malloc(out_stem_len + 4);
  out_rem = malloc(out_stem_len + 5);
  strcpy(out_bs, out_stem);
  strcat(out_bs, ".bs");
  strcpy(out_rem, out_stem);
  strcat(out_rem, ".rem");
  if (rank < nor) {
    /*
     * Finally put the results where requested
     * assuming we have a proper subspace
     */
    rename(mult_result_bs, out_bs);
    rename(mult_result_rem, out_rem);
    free(out_bs);
    free(out_rem);
  } else {
    /*
     * Whole space case.
     * We produce a 0x0 rem, and a bs of the right width all 1
     */
    header my_hdr;
    EFIL *out;
    size_t rslen;
    uint64_t * bsrs;
    /* Write the remnant */
    my_hdr.named.rnd1 = 1;
    my_hdr.named.fdef = fdef;
    my_hdr.named.nor = 0;
    my_hdr.named.noc = 0;
    my_hdr.named.rnd2 = 0;
    out = EWHdr(out_rem, my_hdr.hdr);
    EWClose1(out, 0);
    /* Write the bitstring */
    my_hdr.named.rnd1 = 2;
    my_hdr.named.fdef = 1;
    my_hdr.named.nor = nor;
    my_hdr.named.noc = nor;
    my_hdr.named.rnd2 = 0;
    out = EWHdr(out_bs, my_hdr.hdr);
    /* Populate the bitstring */
    rslen = 16 + ((nor + 63) / 64) * 8;
    bsrs = malloc(rslen);
    bsrs[0] = nor;
    bsrs[1] = nor;
    memset(bsrs + 2, 0xff, rslen - 16);
    /* Write the bitstring */
    EWData(out, rslen, (uint8_t *)bsrs);
    EWClose1(out, 0);
  }
  /* Delete temps */
  remove(mult_result_bs);
  remove(mult_result_rem);
  remove(zero_bs);
  remove(mul_tmp);
  remove(ech_tmp_bs);
  remove(ech_tmp_rem);
  remove(ech_tmp_bs1);
  remove(ech_tmp_rem1);
  remove(ech_tmp_bsc);
  remove(ech_tmp_bsr);
  remove(clean_tmp1);
  remove(clean_tmp2);
  /* And the things in the gen structures */
  for (i = 0; i < ngens; i++) {
    remove(gens[i].next_tbd.bs);
    free(gens[i].next_tbd.bs);
    remove(gens[i].next_tbd.rem);
    free(gens[i].next_tbd.rem);
    remove(gens[i].next_tbd.plain);
    free(gens[i].next_tbd.plain);
  }
  free(gens);
  /* Free all temporaries */
  free(mult_result_bs);
  free(mult_result_rem);
  free(zero_bs);
  free(in_vecs_rem);
  free(in_vecs_bs);
  free(mul_tmp);
  free(ech_tmp_bs);
  free(ech_tmp_rem);
  free(ech_tmp_bs1);
  free(ech_tmp_rem1);
  free(ech_tmp_bsc);
  free(ech_tmp_bsr);
  free(clean_tmp1);
  free(clean_tmp2);
  free(fun_tmp);
  /* Return the rank */
  printf("%lu\n", rank);
  return 0;
}
