/* ---------------------------------------------------------------------------------------------- */
/* Integer/float sample conversion routines. */

#pragma once

#include <string.h>

/* ---------------------------------------------------------------------------------------------- */

/* Convert samples, float to unsigned char (0..255, center = 128). */
void buf_f2uc(unsigned char *dst, float *src, size_t nsamp);

/* Convert samples, float to signed short. */
void buf_f2ss(short *dst, float *src, size_t nsamp);

/* Convert samples, unsigned char (0..255, center = 128) to float. */
void buf_uc2f(float *dst, unsigned char *src, size_t nsamp);

/* Convert samples, signed short to float. */
void buf_ss2f(float *dst, short *src, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */
