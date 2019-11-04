/* ---------------------------------------------------------------------------------------------- */
/* Local oscillator and mixer. */

#pragma once

#include <math.h>
#include "complex.h"
#include "util/cpuid.h"

/* ---------------------------------------------------------------------------------------------- */

/* Note: structure used by asm implementation */
typedef struct osc_state {
	cpxf_t astep;		/* angular step, exp(j*2*pi*f/fs) */
	cpxf_t phasor;		/* current oscillator position */
	float gain;			/* oscillator gain * sqrt(2) */
} osc_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize oscillator.
 * Set phase and frequency to zero.
 * Note: no cleanup needed. */
void osc_init(osc_state_t *osc, double gain);

/* Set oscillator frequency.
 *	freq: oscillator frequency, fraction of sample rate. */
void osc_set_freq(osc_state_t *osc, double f);

/* Read samples from local oscillator.
 *	buf: output buffer, complex float,
 *	num: number of samples to read. */
void osc_generate(osc_state_t *osc, cpxf_t *buf, size_t num);

/* Mix buffer with local oscillator output.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples to process,
 * Note: dst and src can point to same buffer. */
void osc_mix(osc_state_t *osc, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */
