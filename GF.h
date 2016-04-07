#include <stddef.h>
#include <stdint.h>
#define uchar unsigned char

#define FIELD 256

/**
	multi table
*/
uchar _mul[FIELD][FIELD];
/**
 * log (Alpha^x(a) => element(b))
 */
uchar _log[FIELD];
/**
 * exp (element(b) => Alpha^x(a))
 */
uchar _exp[FIELD+1];

uchar _inv[FIELD];

/**
 * isIniGF
 */
int isIniGF;

/**
	gf init
*/
int gf_init();

/**
 * gf_multi
 */
uchar gf_multi(uchar a, uchar b);

/**
 * gf_multi_table
 * log and exp
 */
uchar gf_multi_table(uchar a, uchar b);

/*
	gf div
*/
uchar gf_div_table(uchar a, uchar b);

/*
	gf inverse
*/
uchar gf_inverse_table(uchar a);



