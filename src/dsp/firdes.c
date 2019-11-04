/* ---------------------------------------------------------------------------------------------- */

#include "firdes.h"

/* ---------------------------------------------------------------------------------------------- */

/* sinc function */
static double sinc(double x)
{
	if(x == 0)
		return 1.0;
	return (sin(M_PI * x) / (M_PI * x));
}

/* ---------------------------------------------------------------------------------------------- */

/* Calculate Kaiser window sinc filter length.
 *	atten		: stopband attenuation level, dB,
 *	df			: transition bandwidth, fraction of sample rate. */
size_t firdes_kaiser_length(double atten, double df)
{
	double length;

	if(atten > 21.0) {
		length = ceil((atten - 7.95) / (14.26 * df));
	} else {
		length = ceil(0.92 / df);
	}

	/* prefer odd length */
	return ((size_t)length | 1UL);
}

/* ---------------------------------------------------------------------------------------------- */

/* Calculate Kaiser window sinc filter beta parameter.
 *	atten		: stopband attenuation, dB. */
double firdes_kaiser_beta(double atten)
{
	double beta;

	if(atten > 50.0) {
		beta = 0.1102 * (atten - 8.7);
	} else if(atten > 21.0) {
		beta = 0.5842 * pow(atten - 21.0, 0.4) + 0.07886 * (atten - 21.0);
	} else {
		beta = 0;
	}

	return beta;
}

/* ---------------------------------------------------------------------------------------------- */

/* Make window sinc filter.
 *	buf			: (output) coefficient buffer,
 *	length		: filter length.
 *	fc			: cutoff frequency (-6 dB), fraction of sample rate,
 *	wndtype		: window type,
 *	wndarg		: window argument. */
void firdes_windowsinc(float *buf, size_t length, double fc, wnd_type_t wndtype, double wndarg)
{
	double arg, xf, xw;
	double xh, normf;
	size_t i;

	/* build filter kernel */
	for(i = 0; i < length; i++)
	{
		arg = i - 0.5 * (length - 1);
		xf = sinc(2.0 * fc * arg);
		xw = wnd_point(i, length, wndtype, wndarg);
		xh = xf * xw;
		buf[i] = (float)xh;
	}

	/* normalize filter dc gain */
	normf = 1.0 / sumf(buf, length);
	for(i = 0; i < length; i++)
		buf[i] *= (float)normf;
}

/* ---------------------------------------------------------------------------------------------- */

/* Make window sinc filter with frequency offset.
 *	buf			: (output) coefficient buffer,
 *	length		: filter length.
 *	fc1			: first cutoff frequency (-6 dB), fraction of sample rate,
 *	fc2			: second cutoff frequency (-6 dB), fraction of sample rate,
 *	wndtype		: window type,
 *	wndarg		: window argument. */
void firdes_windowsinc_off(float *buf, size_t length, double fc1, double fc2,
						   wnd_type_t wndtype, double wndarg)
{
	size_t i;
	double f0, fc;
	double arg, xo;

	f0 = 0.5 * (fc1 + fc2);
	fc = 0.5 * (fc2 - fc1);

	firdes_windowsinc(buf, length, fc, wndtype, wndarg);

	/* offset response */
	for(i = 0; i < length; i++) {
		arg = i - 0.5 * (length - 1);
		xo = cos(2.0 * M_PI * f0 * arg);
		buf[i] *= (float)(2.0 * xo);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Make arbitrary response FIR filter
 *		using frequency sampling method with window.
 *	h			: output coefficients, real float,
 *	num_h		: output coefficients count,
 *	a			: frequency response samples, real float, evenly spaced from 0 to Fs/2,
 *	num_a		: frequency response samples count, should be >= num_h,
 *	wndtype		: window type,
 *	wndarg		: window argument. */
int firdes_afr(float *h, size_t num_h, float *a, size_t num_a, wnd_type_t wndtype, double wndarg)
{
	int success = 0;
	size_t i, k, buf_size;
	cpxf_t tmp, accum, *buf;
	double step, wk, hk;

	if((num_h == 0) || (num_h > num_a))
		return 0;

	buf_size = num_a * 2 - 2;
	buf = malloc(sizeof(cpxf_t) * buf_size);

	if(buf != NULL)
	{
		/* response phase shift */
		step = -0.5 * M_PI * (num_h - 1) / (num_a - 1);
		for(i = 0; i < num_a; i++) {
			cpxf_expj(&tmp, i * step);
			cpxf_mulr(buf + i, &tmp, a[i]);
		}

		/* make symmetric response */
		for(i = 1; i < num_a - 1; i++) {
			cpxf_conj(buf + buf_size - i, buf + i);
		}

		/* transform response */
		for(k = 0; k < (num_h + 1) / 2; k++)
		{
			cpxf_zero(&accum);
			step = 2.0 * M_PI * k / buf_size;
			for(i = 0; i < buf_size; i++) {
				cpxf_expj(&tmp, i * step);
				cpxf_mul(&tmp, &tmp, buf + i);
				cpxf_add(&accum, &accum, &tmp);
			}

			wk = wnd_point(k, num_h, wndtype, wndarg);
			hk = accum.re * wk / buf_size;
			h[k] = h[num_h - k - 1] = (float)hk;
		}

		success = 1;
	}

	free(buf);

	return success;
}

/* ---------------------------------------------------------------------------------------------- */
