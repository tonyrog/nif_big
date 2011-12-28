#ifndef __ERL_NIF_BIG_H__
#define __ERL_NIF_BIG_H__

#include "erl_nif.h"

typedef ERL_NIF_TERM ErlNifBigDigit;

#define NUM_TMP_DIGITS 4
#define DIGIT_BITS (sizeof(ErlNifBigDigit)*8)

typedef struct 
{
    unsigned size;          // number of digits 
    unsigned sign;          // 1= negative, 0=none-negative
    ErlNifBigDigit* digits;  // least significant digit first D0 D1 .. Dsize-1
    ErlNifBigDigit  ds[NUM_TMP_DIGITS];
} ErlNifBignum;

extern int enif_is_big(ErlNifEnv* env, ERL_NIF_TERM big_term);
extern int enif_inspect_big(ErlNifEnv* env,ERL_NIF_TERM big_term,ErlNifBignum* big);
extern ERL_NIF_TERM enif_make_number(ErlNifEnv* env, ErlNifBignum* big);

extern int enif_get_number(ErlNifEnv* env, ERL_NIF_TERM t, ErlNifBignum* big);
extern int enif_copy_number(ErlNifEnv* env, ErlNifBignum* big, size_t min_size);
extern void enif_release_number(ErlNifEnv* env, ErlNifBignum* big);
extern int enif_get_copy_number(ErlNifEnv* env, ERL_NIF_TERM t, 
				ErlNifBignum* big,size_t min_size);
#endif
