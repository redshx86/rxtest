/* ---------------------------------------------------------------------------------------------- */
/* AM demodulator. */

#pragma once

#include "../complex.h"

/* ---------------------------------------------------------------------------------------------- */

/* AM demodulate buffer.
 *	dst: destination buffer, real float,
 *	src: source buffer, complex float,
 *	nsamp: number of samples to process. */
size_t demod_am_process(float *dst, cpxf_t *src, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */
