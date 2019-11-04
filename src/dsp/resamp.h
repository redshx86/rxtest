/* ---------------------------------------------------------------------------------------------- */
/* Polyphase FIR resampler implementation */

#pragma once

#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "firdes.h"
#include "util/misc.h"

/* ---------------------------------------------------------------------------------------------- */

/* FIR resampler state */
typedef struct resamp_state {

	unsigned int nch;

	unsigned int fs_in;
	unsigned int fs_out;
	unsigned int fs_temp;
	unsigned int L;
	unsigned int M;

	size_t len_kern;
	size_t len_phase;

	float **pcoef;
	float **pz;

	size_t skipcnt;

} resamp_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize resampler state.
 *	nch			: number of interleaved channels,
 *	fs_in		: input sampling frequency, Hz,
 *	fs_out		: output sampling frequency, Hz,
 *	atten		: alias attenuation, dB,
 *	f_tran		: filter transition bandwidth, Hz. */
int resamp_init(resamp_state_t *resamp, unsigned int nch, unsigned int fs_in,
				unsigned int fs_out, double atten, unsigned int f_tran);

/* Free resampler state. */
void resamp_cleanup(resamp_state_t *resamp);

/* Execute resampler.
 *	out			: output buffer, float,
 *	src			: input buffer, float,
 *	nsamp		: number of input samples.
 * Returns: number of samples stored to output buffer. */
size_t resamp_exec(resamp_state_t *resamp, float *out, float *src, size_t nsamp);

/* returns resampler delay (input samples) */
size_t resamp_get_delay(resamp_state_t *resamp);

/* ---------------------------------------------------------------------------------------------- */
