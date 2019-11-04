/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "window.h"
#include "complex.h"
#include "util/misc.h"

/* ---------------------------------------------------------------------------------------------- */

/* Calculate Kaiser window sinc filter length.
 *	atten		: stopband attenuation level, dB,
 *	df			: transition bandwidth, fraction of sample rate. */
size_t firdes_kaiser_length(double atten, double df);

/* Calculate Kaiser window sinc filter beta parameter.
 *	atten		: stopband attenuation, dB. */
double firdes_kaiser_beta(double atten);

/* Make window sinc filter.
 *	buf			: (output) coefficient buffer,
 *	length		: filter length.
 *	fc			: cutoff frequency (-6 dB), fraction of sample rate,
 *	wndtype		: window type,
 *	wndarg		: window argument. */
void firdes_windowsinc(float *buf, size_t length, double fc, wnd_type_t wndtype, double arg);

/* Make window sinc filter with frequency offset.
 *	buf			: (output) coefficient buffer,
 *	length		: filter length.
 *	fc1			: first cutoff frequency (-6 dB), fraction of sample rate,
 *	fc2			: second cutoff frequency (-6 dB), fraction of sample rate,
 *	wndtype		: window type,
 *	wndarg		: window argument. */
void firdes_windowsinc_off(float *buf, size_t length, double fc1, double fc2,
						   wnd_type_t wndtype, double arg);

/* Make arbitrary response FIR filter
 *		using frequency sampling method with window.
 *	h			: output coefficients, real float,
 *	num_h		: output coefficients count,
 *	a			: frequency response samples, real float, evenly spaced from 0 to Fs/2,
 *	num_a		: frequency response samples count, should be >= num_h,
 *	wndtype		: window type,
 *	wndarg		: window argument. */
int firdes_afr(float *h, size_t num_h, float *a, size_t num_a, wnd_type_t wndtype, double arg);

/* ---------------------------------------------------------------------------------------------- */
