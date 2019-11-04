/* ---------------------------------------------------------------------------------------------- */
/* Comb decimator. */

#pragma once

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "firdes.h"
#include "firfilt.h"
#include "util/misc.h"

/* ---------------------------------------------------------------------------------------------- */

/* Decimator state. */
typedef struct decim_state {
	unsigned int nstages;
	firfilt_state_t *stage;
} decim_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize decimator state.
 *	comb_decimf	: comb filter decimation factor,
 *	compf_decimf: compensation filter decimation factor,
 *	fc			: cutoff frequency, fraction of output sample rate,
 *	df			: transition bandwidth, fraction of output sample rate,
 *	atten		: stopband attenuation, dB,
 *	frgran		: frequency response granularity for filter design, samples. */
int decim_init(decim_state_t *ctx, unsigned int comb_decimf, unsigned int compf_decimf,
			   double fc, double df, double atten, size_t frgran);

/* Free decimator state. */
void decim_cleanup(decim_state_t *ctx);

/* Execute decimator.
 *	buf			: (input/output) data buffer,
 *	nsamp		: number of input samples. */
size_t decim_exec(decim_state_t *ctx, cpxf_t *buf, size_t nsamp);

/* Calculate decimation factors:
 *	decimf: common decimation factor,
 *	mincompfdecimf: minimum compensation filter decimation factor,
 *	pcombdecimf: (output) comb stage decimation factor,
 *	pcompfdecimf: (output) compensation filter decimation factor. */
void decim_calc_factors(unsigned int decimf, unsigned int mincompfdecimf,
						unsigned int *pcombdecimf, unsigned int *pcompfdecimf);

/* ---------------------------------------------------------------------------------------------- */
