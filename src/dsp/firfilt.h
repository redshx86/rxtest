/* ---------------------------------------------------------------------------------------------- */
/* FIR filter implementation with convolution. */

#pragma once

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include "complex.h"
#include "firdes.h"
#include "util/cpuid.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _DEBUG
#	define FIRFILT_MIN_SSE_LENGTH 0
#else
#	define FIRFILT_MIN_SSE_LENGTH 9
#endif

struct firfilt_state;

typedef size_t (__cdecl * firfilt_exec_fn)
	(struct firfilt_state *filt, void *buf, size_t nsamp);

/* FIR filter state. */
/* Note: structure used by asm implementation. */
typedef struct firfilt_state {
	size_t length;
	size_t decimf;
	size_t skipcnt;
	float *coef;
	void *z;
	firfilt_exec_fn exec_fn;
} firfilt_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize FIR filter state.
 *	length		: filter length,
 *	decimf		: decimation factor,
 *	is_cpx		: nonzero if filtered data is complex. */
int firfilt_init(firfilt_state_t *filt, size_t length, size_t decimf, int is_cpx);

/* Set FIR filter coefficients.
 *	coeff		: coefficient buffer. */
void firfilt_set_coefs(firfilt_state_t *filt, float *coef);

/* Free FIR filter state. */
void firfilt_cleanup(firfilt_state_t *filt);

/* Execute FIR filter.
 *	buf			: data buffer (input/output),
 *	nsamp		: number of input samples.
 * Returns: number of output samples. */
#define firfilt_exec(filt, buf, nsamp) (filt)->exec_fn(filt, buf, nsamp)

/* ---------------------------------------------------------------------------------------------- */
