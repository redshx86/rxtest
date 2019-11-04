/* ---------------------------------------------------------------------------------------------- */

#include "resamp.h"

/* ---------------------------------------------------------------------------------------------- */

/* Initialize resampler state.
 *	nch			: number of interleaved channels,
 *	fs_in		: input sampling frequency, Hz,
 *	fs_out		: output sampling frequency, Hz,
 *	atten		: alias attenuation, dB,
 *	f_tran		: filter transition bandwidth, Hz. */
int resamp_init(resamp_state_t *resamp, unsigned int nch, unsigned int fs_in,
				unsigned int fs_out, double atten, unsigned int f_tran)
{
	unsigned int fs_temp, f_stop, f_cutoff;
	double df, fc;
	float *buf, *coef, *z;
	size_t n, i, j, index;

	/* antialias filter parameters */
	fs_temp = lcm(fs_in, fs_out);
	f_stop = min(fs_in, fs_out) / 2;
	f_cutoff = f_stop - f_tran / 2;
	df = (double)f_tran / (double)fs_temp;
	fc = (double)f_cutoff / (double)fs_temp;

	/* initialize structure */
	resamp->nch = nch;
	resamp->fs_in = fs_in;
	resamp->fs_out = fs_out;
	resamp->fs_temp = fs_temp;
	resamp->L = fs_temp / fs_in;
	resamp->M = fs_temp / fs_out;

	n = firdes_kaiser_length(atten, df);
	resamp->len_phase = (n + resamp->L - 1) / resamp->L;
	resamp->len_kern = resamp->L * resamp->len_phase;

	/* alloc buffers */
	resamp->pcoef = malloc(resamp->L * sizeof(float*));
	resamp->pz = malloc(nch * sizeof(float*));
	coef = malloc(resamp->len_kern * sizeof(float));
	z = malloc(nch * resamp->len_phase * sizeof(float));
	buf = malloc(resamp->len_kern * sizeof(float));

	if( (resamp->pcoef != NULL) && (resamp->pz != NULL) &&
		(coef != NULL) && (z != NULL) && (buf != NULL) )
	{
		/* set filter coefficients */
		firdes_windowsinc(buf, resamp->len_kern, fc,
			WND_KAISER, firdes_kaiser_beta(atten));
		for(i = 0; i < resamp->L; i++) {
			resamp->pcoef[i] = coef + i * resamp->len_phase;
			for(j = 0; j < resamp->len_phase; j++) {
				index = (resamp->len_phase - j - 1) * resamp->L + i;
				resamp->pcoef[i][j] = buf[index] * resamp->L;
			}
		}

		/* set delay buffers */
		for(i = 0; i < nch; i++) {
			resamp->pz[i] = z + i * resamp->len_phase;
			memset(resamp->pz[i], 0, resamp->len_phase * sizeof(float));
		}

		resamp->skipcnt = 0;

		free(buf);
		return 1;
	}

	free(buf);
	free(z);
	free(coef);
	free(resamp->pz);
	free(resamp->pcoef);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free resampler state. */
void resamp_cleanup(resamp_state_t *resamp)
{
	free(resamp->pz[0]);
	free(resamp->pcoef[0]);
	free(resamp->pz);
	free(resamp->pcoef);
}

/* ---------------------------------------------------------------------------------------------- */

/* Execute resampler.
 *	out			: output buffer, float,
 *	src			: input buffer, float,
 *	nsamp		: number of input samples.
 * Returns: number of samples stored to output buffer. */
size_t resamp_exec(resamp_state_t *resamp, float *out, float *src, size_t nsamp)
{
	unsigned int c;
	size_t nsamp_out = 0;
	size_t i, curphase;
	double accum;

	/* for all input samples */
	while(nsamp--)
	{
		/* load samples to delay line buffers */
		for(c = 0; c < resamp->nch; c++) {
			for(i = 0; i < resamp->len_phase - 1; i++)
				resamp->pz[c][i] = resamp->pz[c][i + 1];
			resamp->pz[c][resamp->len_phase - 1] = *(src++);
		}

		/* downsample output */
		curphase = 0;

		while(resamp->skipcnt < resamp->L - curphase)
		{
			/* skip downsampled phases */
			curphase += resamp->skipcnt;

			/* calculate output samples */
			for(c = 0; c < resamp->nch; c++)
			{
				accum = 0;
				for(i = 0; i < resamp->len_phase; i++) {
					accum += resamp->pz[c][i] *
						resamp->pcoef[curphase][i];
				}
				*(out++) = (float)accum;
			}

			/* reset downsample counter */
			resamp->skipcnt = resamp->M;

			nsamp_out++;
		}

		/* skip remaining phases */
		resamp->skipcnt -= resamp->L - curphase;
	}

	return nsamp_out;
}

/* ---------------------------------------------------------------------------------------------- */

/* returns resampler delay (input samples) */
size_t resamp_get_delay(resamp_state_t *resamp)
{
	return (size_t)(resamp->len_phase);
}

/* ---------------------------------------------------------------------------------------------- */
