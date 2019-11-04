/* ---------------------------------------------------------------------------------------------- */
/* FIR filter implementation with FFT. */

#pragma once

#include <string.h>
#include "fft.h"
#include "util/cpuid.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct firfilt_fft {

	size_t fft_len;			/* FFT length, 2^n */
	size_t filt_len;		/* Filter kernel length, samples */
	size_t data_len;		/* Data samples per FFT (fft_len - filt_len) */

	fft_params_t fft_fwd;	/* Forward FFT params */
	fft_params_t fft_back;	/* Inverse FFT params */

	size_t num_avail;		/* Buffered input samples count */
	cpxf_t *buf_in;		/* Input samples */

	cpxf_t *H;				/* Frequency response of filter */

	cpxf_t *tail;			/* Chaining buffer */

	cpxf_t *buf_temp;		/* Temp buffer */

} firfilt_fft_t;

/* ---------------------------------------------------------------------------------------------- */

/* Prepare FFT FIR filter data.
 *	fft_len: fft length, power of two,
 *	filt_len: filter kernel length, sub fft length. */
int firfilt_fft_init(firfilt_fft_t *ff, size_t fft_len, size_t filt_len);

/* Free FFT FIR filter data. */
void firfilt_fft_cleanup(firfilt_fft_t *ff);

/* Set FFT FIR filter coefficients. */
void firfilt_fft_tune(firfilt_fft_t *ff, float *h);

/* Process buffer with FFT FIR filter.
 *	dst: destination buffer, float complex,
 *	src: source buffer, float compex,
 *	num: number of samples in src buffer,
 *	return: number of samples written to dst buffer.
 * Minimum dst buffer size: ff->data_len + num. */
size_t firfilt_fft_process(firfilt_fft_t *ff, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */
