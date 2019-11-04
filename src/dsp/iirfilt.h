/* ---------------------------------------------------------------------------------------------- */
/* IIR filter implemented as Direct Form 1. */

#pragma once

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "complex.h"

/* ---------------------------------------------------------------------------------------------- */

/* 1st order IIR filter state */
typedef struct iirfilt_state1 {
	float b0, b1;
	float a1;
	cpxf_t x1;
	cpxf_t y1;
} iirfilt_state1_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize 1st order IIR filter.
 *	b: Tf numerator coefficients, real float, 2 points,
 *	a: Tf denominator coefficients, real float, 2 points.
 * Note: no cleanup needed. */
void iirfilt_init1(iirfilt_state1_t *f, float *b, float *a);

/* Process real float buffer with 1st order IIR filter.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process1(iirfilt_state1_t *f, float *dst, float *src, size_t num);

/* Process complex float buffer with 1st order IIR filter.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process1_complex(iirfilt_state1_t *f, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */

/* 2nd order IIR filter state */
typedef struct iirfilt_state2 {
	float b0, b1, b2;	/* Numerator coefficients */
	float a1, a2;		/* Denumenator coefficients */
	cpxf_t x1, x2;		/* Input delay line */
	cpxf_t y1, y2;		/* Output delay line */
} iirfilt_state2_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize 2nd order IIR filter.
 *	b: Tf numerator coefficients, real float, 3 points,
 *	a: Tf denominator coefficients, real float, 3 points.
 * Note: no cleanup needed. */
void iirfilt_init2(iirfilt_state2_t *f, float *b, float *a);

/* Process real float buffer with 2nd order IIR filter.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process2(iirfilt_state2_t *f, float *dst, float *src, size_t num);

/* Process complex float buffer with 2nd order IIR filter.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process2_complex(iirfilt_state2_t *f, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */

/* DF1 filter. */
typedef struct iirfilt_state {
	size_t blen;		/* Number of numerator coefficients */
	size_t alen;		/* Number of denumerator coefficients */
	float *b;			/* Numberator coefficients */
	float *a;			/* Denumerator coefficients */
	void *x;			/* Input delay line */
	void *y;			/* Output delay line */
} iirfilt_state_t;

/* Initialize IIR filter.
 *	blen: number of tf numerator coefficients,
 *	alen: number of tf denumerator coefficients,
 *	is_complex: nonzero to allocate delay lines for complex values. */
int iirfilt_init(iirfilt_state_t *f, size_t blen, size_t alen, int is_complex);

/* Free IIR filter stuff. */
void iirfilt_cleanup(iirfilt_state_t *f);

/* Set IIR filter coefficients.
 *	b: tf numerator coefficients, real float,
 *	a: tf denumerator coefficients, real float. */
void iirfilt_tune(iirfilt_state_t *f, float *b, float *a);

/* Process real float buffer with IIR filter.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of sampples.
 * Note: src and dst can be same buffer. */
void iirfilt_process(iirfilt_state_t *f, float *dst, float *src, size_t num);

/* Process complex float buffer with IIR filter.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of sampples.
 * Note: src and dst can be same buffer. */
void iirfilt_process_complex(iirfilt_state_t *f, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */

/* Initialize IIR DC remover (1st order IIR filter with
 *	b0 = 1, b1 = -1, a1 = -1 + alpha). */
void iir_dcrem_init(iirfilt_state1_t *f, double alpha);

/* Process DC removal on real float buffer.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iir_dcrem_process(iirfilt_state1_t *f, float *dst, float *src, size_t num);

/* Process DC removal on complex float buffer.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iir_dcrem_process_complex(iirfilt_state1_t *f, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */
