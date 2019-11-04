/* ---------------------------------------------------------------------------------------------- */
/* Simple radix 2 FFT implementation. */

#pragma once

#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "complex.h"
#include "util/cpuid.h"

/* ---------------------------------------------------------------------------------------------- */

/* Note: Structure used by asm implementation */
typedef struct fft_params {
	size_t length;		/* fft length, power of two */
	size_t bitnum;		/* log2(length) */
	size_t *brmap;		/* bit reversal map */
	cpxf_t *twiddle;	/* twiddle factors */
} fft_params_t;

/* ---------------------------------------------------------------------------------------------- */

/* Prepare FFT.
 *	length: FFT length, power of two only,
 *	inverse: nonzero to prepare IFFT. */
int fft_init(fft_params_t *ctx, size_t length, int inverse);

/* Free FFT data */
void fft_cleanup(fft_params_t *ctx);

/* Perofrm FFT.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float.
 * Note: src & dst can be same buffer for in-place transform.
 * Note: 1/N term omited in inverse transform. */
void fft_process(fft_params_t *ctx, cpxf_t *dst, cpxf_t *src);

/* ---------------------------------------------------------------------------------------------- */
