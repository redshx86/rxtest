/* ---------------------------------------------------------------------------------------------- */
/* Logarithmic dynamic range limiter implementation. */
/* http://www.voegler.eu/pub/audio/digital-audio-mixing-and-normalization.html */

#pragma once

#include <string.h>
#include <math.h>

/* ---------------------------------------------------------------------------------------------- */

typedef struct rangelim_params {
	double range;	/* input range to fit to [-1,1] */
	double thres;	/* limiting threshold */
	double b;
	double c;
} rangelim_params_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize range limiter.
 *	range: input samples range to fit, 1.01 to 100.00,
 *	thres: limiting threshold, 0.00 to 0.99.
 * (no cleanup needed) */
int rangelim_init(rangelim_params_t *ctx, double range, double thres);

/* Perform range limiting on buffer.
 *	dst: destination buffer, float, may be equal to src,
 *	src: source buffer, float, may be equal to dst,
 *	num: number of samples to process. */
void rangelim_process(rangelim_params_t *ctx, float *dst, float *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */
