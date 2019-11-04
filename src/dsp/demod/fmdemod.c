/* ---------------------------------------------------------------------------------------------- */

#include "fmdemod.h"

/* ---------------------------------------------------------------------------------------------- */

/* Initialize FM demodulator. */
void demod_fm_init(demod_fm_state_t *s)
{
	cpxf_zero(&(s->buf));
}

/* ---------------------------------------------------------------------------------------------- */

/* Free FM demodulator. */
void demod_fm_cleanup(demod_fm_state_t *s)
{
}

/* ---------------------------------------------------------------------------------------------- */

/* Set FM demodulator parameters.
 *	fdevi: max frequency deviation, fraction of sample rate. */
void demod_fm_tune(demod_fm_state_t *s, double fdevi)
{
	s->gain = 1.0 / (2.0 * M_PI * fdevi);
}

/* ---------------------------------------------------------------------------------------------- */

/* FM demodulate buffer.
 *	dst: destination buffer, real float,
 *	src: source buffer, complex float,
 *	nsamp: number of samples. */
void demod_fm_process(demod_fm_state_t *s, float *dst, cpxf_t *src, size_t nsamp)
{
	float sample;
	cpxf_t samp, temp;

	while(nsamp--)
	{
		cpxf_copy(&samp, src++);

		cpxf_conj(&temp, &(s->buf));
		cpxf_mul(&temp, &temp, &samp);
		sample = (float)(cpxf_arg(&temp) * s->gain);
		*(dst++) = sample;

		cpxf_copy(&(s->buf), &samp);
	}
}

/* ---------------------------------------------------------------------------------------------- */
