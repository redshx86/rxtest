/* ---------------------------------------------------------------------------------------------- */

#include "osc.h"

/* ---------------------------------------------------------------------------------------------- */

void __cdecl osc_generate_sse(osc_state_t *osc, cpxf_t *buf, size_t num);
void __cdecl osc_mix_sse(osc_state_t *osc, cpxf_t *dst, cpxf_t *src, size_t num);

/* ---------------------------------------------------------------------------------------------- */

/* Initialize oscillator.
 * Set phase and frequency to zero.
 * Note: no cleanup needed. */
void osc_init(osc_state_t *osc, double gain)
{
	cpxf_setr(&(osc->astep), 1.0f);
	cpxf_setr(&(osc->phasor), (float)M_SQRT1_2);
	osc->gain = (float)(gain * M_SQRT2);
}

/* ---------------------------------------------------------------------------------------------- */

/* Set oscillator frequency.
 *	freq: oscillator frequency, fraction of sample rate. */
void osc_set_freq(osc_state_t *osc, double f)
{
	cpxf_expj(&(osc->astep), 2.0 * M_PI * f);
}

/* ---------------------------------------------------------------------------------------------- */

/* Read samples from local oscillator.
 *	buf: output buffer, complex float,
 *	num: number of samples to read. */
void osc_generate(osc_state_t *osc, cpxf_t *buf, size_t num)
{
	float pwr;

	if(is_sse_enabled) {
		osc_generate_sse(osc, buf, num);
		return;
	}

	while(num--)
	{
		/* Scale output to unity vector */
		cpxf_mulr(buf++, &(osc->phasor), osc->gain);

		/* Rotate phasor by angular step */
		cpxf_mul(&(osc->phasor), &(osc->phasor), &(osc->astep));

		/* Correct amplitude */
		pwr = cpxf_modsqr(&(osc->phasor));
		cpxf_mulr(&(osc->phasor), &(osc->phasor), 1.5f - pwr);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Mix buffer with local oscillator output.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples to process,
 * Note: dst and src can point to same buffer. */
void osc_mix(osc_state_t *osc, cpxf_t *dst, cpxf_t *src, size_t num)
{
	float pwr;
	cpxf_t out;

	if(is_sse_enabled) {
		osc_mix_sse(osc, dst, src, num);
		return;
	}

	while(num--)
	{
		/* Scale output to unity vector */
		cpxf_mulr(&out, &(osc->phasor), osc->gain);

		/* Mix buffer with oscillator output */
		cpxf_mul(dst++, src++, &out);

		/* Rotate phasor by angular step */
		cpxf_mul(&(osc->phasor), &(osc->phasor), &(osc->astep));

		/* Correct amplitude */
		pwr = cpxf_modsqr(&(osc->phasor));
		cpxf_mulr(&(osc->phasor), &(osc->phasor), 1.5f - pwr);
	}
}

/* ---------------------------------------------------------------------------------------------- */
