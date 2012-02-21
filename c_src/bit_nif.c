//
// Integer math C primitives
//

#include <stdint.h>
#include "erl_nif.h"
#include "erl_nif_big.h"

// #define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

static int bit_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info);
static int bit_reload(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info);
static int bit_upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, 
			 ERL_NIF_TERM load_info);
static void bit_unload(ErlNifEnv* env, void* priv_data);


static ERL_NIF_TERM bit_size(ErlNifEnv* env, int argc, 
			     const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM bit_test(ErlNifEnv* env, int argc, 
			     const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM bit_set(ErlNifEnv* env, int argc, 
			    const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM bit_clear(ErlNifEnv* env, int argc, 
			      const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM bit_toggle(ErlNifEnv* env, int argc, 
			       const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM bit_count(ErlNifEnv* env, int argc, 
			      const ERL_NIF_TERM argv[]);

ErlNifFunc bit_funcs[] =
{
    { "size",   1, bit_size },
    { "test",   2, bit_test },
    { "set",    2, bit_set },
    { "clear",  2, bit_clear },
    { "toggle", 2, bit_toggle },
    { "count",  1, bit_count },
};

// Calculate size of representation
ERL_NIF_TERM bit_size(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBignum  big;
    unsigned long size;
    unsigned long d;
    
    if (!enif_get_number(env, argv[0], &big))
	return enif_make_badarg(env);
    
    size = (big.size - 1)*DIGIT_BITS;
    d = big.digits[big.size-1];
    while(d) {
	size++;
	d >>= 1;
    }
    return enif_make_ulong(env, size);
}

ERL_NIF_TERM bit_test(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBignum  big;
    unsigned long bn;
    int bit = 0;

    if (!enif_get_ulong(env, argv[1], &bn))
	return enif_make_badarg(env);

    if (enif_get_number(env, argv[0], &big) && !big.sign) {
	int d = 0;
	while((d < big.size) && (bn >= DIGIT_BITS)) {
	    bn -= DIGIT_BITS;
	    d++;
	}
	if ((d < big.size) && (bn < DIGIT_BITS))
	    bit = (big.digits[d] & (((ErlNifBigDigit)1) << bn)) != 0;
    }
    else
	return enif_make_badarg(env);
    return enif_make_int(env, bit);
}

ERL_NIF_TERM bit_set(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBignum  big;
    unsigned long bn;
    size_t min_size;

    if (!enif_get_ulong(env, argv[1], &bn))
	return enif_make_badarg(env);

    min_size = (bn/DIGIT_BITS) + 1;
    if (enif_get_number(env, argv[0], &big) && !big.sign) {
	if (enif_copy_number(env, &big, min_size)) {
	    ERL_NIF_TERM r;
	    int d = 0;
	    while((d < big.size) && (bn >= DIGIT_BITS)) {
		bn -= DIGIT_BITS;
		d++;
	    }
	    big.digits[d] |= (((ErlNifBigDigit)1) << bn);

	    r = enif_make_number(env, &big);
	    enif_release_number(env, &big);
	    return r;
	}
    }
    return enif_make_badarg(env);
}

ERL_NIF_TERM bit_clear(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBignum  big;
    unsigned long bn;
    size_t min_size;

    if (!enif_get_ulong(env, argv[1], &bn))
	return enif_make_badarg(env);
    if (!enif_get_number(env, argv[0], &big) && !big.sign)
	return enif_make_badarg(env);
    min_size = (bn / DIGIT_BITS) + 1;
    if (big.size < min_size)
	return argv[0];
    else {
	ERL_NIF_TERM r;
	int d = 0;

	enif_copy_number(env, &big, min_size);
	while((d < big.size) && (bn >= DIGIT_BITS)) {
	    bn -= DIGIT_BITS;
	    d++;
	}
	big.digits[d] &= ~(((ErlNifBigDigit)1) << bn);
	r = enif_make_number(env, &big);
	enif_release_number(env, &big);
	return r;
    }
    return enif_make_badarg(env);
}

ERL_NIF_TERM bit_toggle(ErlNifEnv* env, int argc,  const ERL_NIF_TERM argv[])
{
    ErlNifBignum  big;
    unsigned long bn;
    size_t min_size;

    if (!enif_get_ulong(env, argv[1], &bn))
	return enif_make_badarg(env);
    min_size = (bn/ DIGIT_BITS) + 1;
    if (enif_get_number(env, argv[0], &big)  && !big.sign) {
	if (enif_copy_number(env, &big, min_size)) {
	    ERL_NIF_TERM r;
	    int d = 0;

	    while((d < big.size) && (bn >= DIGIT_BITS)) {
		bn -= DIGIT_BITS;
		d++;
	    }
	    big.digits[d] ^= (((ErlNifBigDigit)1) << bn);
	    r = enif_make_number(env, &big);
	    enif_release_number(env, &big);
	    return r;
	}
    }
    return enif_make_badarg(env);
}

#if defined(HALFWORD_HEAP_EMULATOR) || (SIZEOF_LONG == 4)
#define B7 0x77777777
#define B3 0x33333333
#define B1 0x11111111
#define BM 0x0F0F0F0F
#elif (SIZEOF_LONG == 8)
#define B7 0x7777777777777777LL
#define B3 0x3333333333333333LL
#define B1 0x1111111111111111LL
#define BM 0x0F0F0F0F0F0F0F0FLL
#else 
#error "bad erl_int_size integer configuration"
#endif

ERL_NIF_TERM bit_count(ErlNifEnv* env, int argc,  const ERL_NIF_TERM argv[])
{
    ErlNifBignum  big;
    int count;
    int i;
    if (!enif_get_number(env, argv[0], &big))
	return enif_make_badarg(env);
    i = 0;
    count = 0;
    while(i < big.size) {
	ErlNifBigDigit d = big.digits[i];
	d = (d) - (((d>>1)&B7) + ((d>>2)&B3) + ((d>>3)&B1));
	d = d + (d>>4);
	count += ((d & BM) % 255);
	i++;
    }
    return enif_make_int(env, count);
}


static int bit_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    (void) env;
    (void) load_info;
    DBG("bit_load\r\n");
    *priv_data = 0;
    return 0;
}

static int bit_reload(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    (void) env;
    (void) load_info;
    DBG("bit_reload\r\n");
    return 0;
}

static int bit_upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, 
			 ERL_NIF_TERM load_info)
{
    (void) env;
    (void) load_info;
    DBG("bit_upgrade\r\n");
    *priv_data = *old_priv_data;
    return 0;
}

static void bit_unload(ErlNifEnv* env, void* priv_data)
{
    (void) env;
    DBG("bit_unload\r\n");
}



ERL_NIF_INIT(bit, bit_funcs,
	     bit_load, bit_reload, 
	     bit_upgrade, bit_unload)





