/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <math.h>
#include "../complex.h"
#include "../iirfilt.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct demod_fm_state {

	cpxf_t buf;
	double gain;

} demod_fm_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize FM demodulator. */
void demod_fm_init(demod_fm_state_t *s);

/* Set FM demodulator parameters.
 *	fdevi: max frequency deviation, fraction of sample rate. */
void demod_fm_tune(demod_fm_state_t *s, double fdevi);

/* FM demodulate buffer.
 *	dst: destination buffer, real float,
 *	src: source buffer, complex float,
 *	nsamp: number of samples. */
void demod_fm_process(demod_fm_state_t *s, float *dst, cpxf_t *src, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */
