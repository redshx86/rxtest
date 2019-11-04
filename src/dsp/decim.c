/* ---------------------------------------------------------------------------------------------- */

#include "decim.h"

/* ---------------------------------------------------------------------------------------------- */

/* Calculate comb filter coefficients.
 *	order		: comb order,
 *	decimf		: comb decimation factor. */
static size_t decim_comb_coefs(float *buf, unsigned int order, unsigned int decimf)
{
	unsigned int m, i;
	float *comb1, *temp, normf;
	size_t buflen, res = 0;

	buflen = (decimf - 1) * order + 1;

	comb1 = malloc(decimf * sizeof(float));
	temp = malloc(buflen * sizeof(float));

	if((comb1 != NULL) && (temp != NULL))
	{
		/* 1st order comb */
		normf = (float)(1.0 / decimf);
		for(i = 0; i < decimf; i++)
			comb1[i] = normf;

		/* delta function */
		m = 1;
		temp[0] = 1.0f;

		/* convolve delta function with 1st order combs */
		for(i = 0; i < order; i++) {
			m = (unsigned int)convf(buf, comb1, temp, decimf, m);
			memcpy(temp, buf, m * sizeof(float));
		}

		res = m;
	}

	free(temp);
	free(comb1);
	return res;
}

/* ---------------------------------------------------------------------------------------------- */

/* Calculate comb filter magnitude response.
 *	f			: frequency point, fraction of input sample rate,
 *	decimf		: comb decimation factor,
 *	order		: comb order. */
static double decim_comb_mag(double f, unsigned int decimf, unsigned int order)
{
	double tmp, mag;

	if(f == 0) {
		mag = 1.0f;
	} else {
		tmp = sin(M_PI * f * decimf) / (sin(M_PI * f) * decimf);
		mag = pow(fabs(tmp), order);
	}

	return mag;
}

/* ---------------------------------------------------------------------------------------------- */

/* Calculate required comb order.
 *	decimf		: comb decimation factor,
 *	f_stop		: required stopband edge frequency,
 *	atten_req	: required alias attenuation at f_stop.
 * f_stop is fraction of comb input sampling frequency. */
static unsigned int decim_comb_order(unsigned int decimf, double f_stop, double atten_req)
{
	unsigned int order;
	double fsa, magn, atten;

	/* first alias stopband edge */
	fsa = 1.0 / decimf - f_stop;

	/* start from 1 section */
	order = 1;

	for( ; ; )
	{
		/* calculate first alias attenuation */
		magn = decim_comb_mag(fsa, decimf, order);
		atten = -20.0 * log10(magn);

		/* is attenuation enough? */
		if(atten >= atten_req)
			break;

		/* add section */
		order++;
	}

	return order;
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize comb compensation filter.
 *	frgran		: frequency response granularity,
 *	compf_decimf: compensation filter decimation factor,
 *	comb_decimf	: comb decimation factor,
 *	comb_order	: comb order,
 *	fc			: cutoff frequency,
 *	df			: transition bandwidth,
 *	atten		: stopband attenuation, dB.
 * fc and df are fraction of compensation filter input sample rate. */
static int decim_compf_init(firfilt_state_t *filt, size_t frgran,
							unsigned int comb_decimf, unsigned int comb_order,
							unsigned int compf_decimf, double fc, double df, double atten)
{
	size_t length;
	float *coef, *frbuf;
	unsigned int i, npass;
	double fstep, f, mag;

	length = firdes_kaiser_length(atten, df);

	if(firfilt_init(filt, length, compf_decimf, 1))
	{
		if(frgran < filt->length)
			frgran = filt->length;

		/* allocate buffers */
		frbuf = malloc(sizeof(float) * frgran);
		coef = malloc(sizeof(float) * length);

		if( (frbuf != NULL) && (coef != NULL) )
		{
			/* number of passband samples */
			npass = (unsigned int)(2.0 * fc * frgran);

			/* passband samples: inverted comb magnitude */
			fstep = 0.5 / (double)(comb_decimf * (frgran - 1));
			for(i = 0; i < npass; i++)
			{
				f = i * fstep;
				mag = decim_comb_mag(f, comb_decimf, comb_order);
				frbuf[i] = (float)(1.0 / mag);
			}

			/* stopband samples: zero */
			for(i = npass; i < frgran; i++)
				frbuf[i] = 0;

			/* generate filter coefficients */
			if(firdes_afr(coef, length, frbuf, frgran,
				WND_KAISER, firdes_kaiser_beta(atten)))
			{
				firfilt_set_coefs(filt, coef);

				free(coef);
				free(frbuf);
				return 1;
			}
		}

		free(coef);
		free(frbuf);
		firfilt_cleanup(filt);
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Antialias filter design (no comb compensation).
 *	length		: filter length,
 *	decimf		: decimation factor,
 *	fc			: cutoff frequency,
 *	df			: transition bandwidth,
 *	atten		: stopband attenuation.
 * fc and df is fraction of antialias filter input sample rate. */
static int decim_afilt_init(firfilt_state_t *filt, size_t decimf, double fc, double df, double atten)
{
	size_t length;
	float *coef;

	length = firdes_kaiser_length(atten, df);

	if(firfilt_init(filt, length, decimf, 1))
	{
		if((coef = malloc(length * sizeof(float))) != NULL)
		{
			firdes_windowsinc(coef, length, fc,
				WND_KAISER, firdes_kaiser_beta(atten));
			firfilt_set_coefs(filt, coef);

			free(coef);
			return 1;
		}
		firfilt_cleanup(filt);
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize comb filter stage.
 *	comb_decimf	: comb decimation factor,
 *	comb_order	: comb order. */
static int decim_comb_init(firfilt_state_t *filt, unsigned int comb_decimf, unsigned int comb_order)
{
	float *coef;
	size_t comb_len;

	comb_len = (comb_decimf - 1) * comb_order + 1;

	if(firfilt_init(filt, comb_len, comb_decimf, 1))
	{
		if((coef = malloc(comb_len * sizeof(float))) != NULL)
		{
			if(decim_comb_coefs(coef, comb_order, comb_decimf))
			{
				firfilt_set_coefs(filt, coef);
				free(coef);
				return 1;
			}
			free(coef);
		}
		firfilt_cleanup(filt);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize decimator state.
 *	comb_decimf	: comb filter decimation factor,
 *	compf_decimf: compensation filter decimation factor,
 *	fc			: cutoff frequency, fraction of output sample rate,
 *	df			: transition bandwidth, fraction of output sample rate,
 *	atten		: stopband attenuation, dB,
 *	frgran		: frequency response granularity for filter design, samples. */
int decim_init(decim_state_t *ctx, unsigned int comb_decimf, unsigned int compf_decimf,
			   double fc, double df, double atten, size_t frgran)
{
	double compf_fc, compf_df, comb_fs;
	unsigned int i, comb_order, ncombstages;
	unsigned int facbuf[32];

	/* no comb stages, antialias filter */
	if(comb_decimf == 1)
	{
		ctx->nstages = 1;

		if( (ctx->stage = malloc(sizeof(firfilt_state_t))) != NULL )
		{
			compf_fc = fc / compf_decimf;
			compf_df = df / compf_decimf;

			if(decim_afilt_init(ctx->stage, compf_decimf, compf_fc, compf_df, atten))
				return 1;

			free(ctx->stage);
		}
	}

	/* comb stages and compensation filter */
	else
	{
		comb_fs = (fc + 0.5 * df) / (double)(comb_decimf * compf_decimf);
		comb_order = decim_comb_order(comb_decimf, comb_fs, atten);
		ncombstages = factor(facbuf, comb_decimf);

		if((ctx->stage = malloc( (ncombstages + 1) * sizeof(firfilt_state_t))) != NULL)
		{
			ctx->nstages = 0;

			/* initialize comb stages */
			for(i = 0; i < ncombstages; i++)
			{
				if(!decim_comb_init(ctx->stage + i, facbuf[i], comb_order))
					break;
				ctx->nstages++;
			}

			/* initialize compensation stage */
			if(ctx->nstages == ncombstages)
			{
				compf_fc = fc / compf_decimf;
				compf_df = df / compf_decimf;

				if(decim_compf_init(ctx->stage + ctx->nstages, frgran, comb_decimf, comb_order,
					compf_decimf, compf_fc, compf_df, atten))
				{
					ctx->nstages++;
					return 1;
				}
			}

			/* cleanup */
			for(i = 0; i < ctx->nstages; i++)
				firfilt_cleanup(ctx->stage + i);
			free(ctx->stage);
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free decimator state. */
void decim_cleanup(decim_state_t *ctx)
{
	unsigned int i;

	for(i = 0; i < ctx->nstages; i++)
		firfilt_cleanup(ctx->stage + i);
	free(ctx->stage);
}

/* ---------------------------------------------------------------------------------------------- */

/* Execute decimator.
 *	buf			: (input/output) data buffer,
 *	nsamp		: number of input samples. */
size_t decim_exec(decim_state_t *ctx, cpxf_t *buf, size_t nsamp)
{
	unsigned int i;

	for(i = 0; i < ctx->nstages; i++)
		nsamp = firfilt_exec(ctx->stage + i, buf, nsamp);
	return nsamp;
}

/* ---------------------------------------------------------------------------------------------- */

/* Calculate decimation factors:
 *	decimf: common decimation factor,
 *	mincompfdecimf: minimum compensation filter decimation factor,
 *	pcombdecimf: (output) comb stage decimation factor,
 *	pcompfdecimf: (output) compensation filter decimation factor. */
void decim_calc_factors(unsigned int decimf, unsigned int mincompfdecimf,
						unsigned int *pcombdecimf, unsigned int *pcompfdecimf)
{
	unsigned int combdecimf, compfdecimf, reducf;

	combdecimf = 1;
	compfdecimf = decimf;

	while(compfdecimf > mincompfdecimf)
	{
		for(reducf = 2; reducf < compfdecimf; reducf++)
		{
			if(compfdecimf % reducf == 0)
				break;
		}

		if(compfdecimf / reducf < mincompfdecimf)
			break;

		combdecimf *= reducf;
		compfdecimf /= reducf;
	}

	*pcombdecimf = combdecimf;
	*pcompfdecimf = compfdecimf;
}

/* ---------------------------------------------------------------------------------------------- */
