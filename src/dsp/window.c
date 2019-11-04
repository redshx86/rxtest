/* ---------------------------------------------------------------------------------------------- */

#include "window.h"

/* ---------------------------------------------------------------------------------------------- */

/* 0th order modified Bessel function */
static double bessel_i0(double x)
{
	int i;
	double numer, denum;
	double temp, sum;

	sum = 1.0;
	numer = x * 0.5;
	denum = 1.0;
	for(i = 2; i <= 20; i++)
	{
		temp = numer / denum;
		sum += temp * temp;
		numer *= x * 0.5;
		denum *= i;
	}
	return sum;
}

/* ---------------------------------------------------------------------------------------------- */

/* Kaiser window. */
double wnd_kaiser(size_t i, size_t n, double beta)
{
	double t, numer, denum;

	t = 2.0 * i / (n - 1) - 1.0;
	numer = bessel_i0(beta * sqrt(1.0 - t * t));
	denum = bessel_i0(beta);
	return (numer / denum);
}

/* ---------------------------------------------------------------------------------------------- */

/* 1st order generalized cosine window */
static double wnd_cosine1(double a0, double a1, size_t i, size_t n)
{
	double arg;

	arg = 2.0 * M_PI * i / (n - 1);
	return (a0 - a1 * cos(arg));
}

/* ---------------------------------------------------------------------------------------------- */

/* 2nd order generalized cosine window */
static double wnd_cosine2(double a0, double a1, double a2, size_t i, size_t n)
{
	double arg;

	arg = 2.0 * M_PI * i / (n - 1);
	return (a0 - a1 * cos(arg) + a2 * cos(2.0 * arg));
}

/* ---------------------------------------------------------------------------------------------- */

/* 3rd order generalized cosine window */
static double wnd_cosine3(double a0, double a1, double a2, double a3, size_t i, size_t n)
{
	double arg;

	arg = 2.0 * M_PI * i / (n - 1);
	return (a0 - a1 * cos(arg) + a2 * cos(2.0 * arg) - a3 * cos(4.0 * arg));
}

/* ---------------------------------------------------------------------------------------------- */

/* calculate specifed window point */
double wnd_point(size_t i, size_t n, wnd_type_t type, double arg)
{
	switch(type)
	{
	case WND_HANN:
		return wnd_cosine1(0.5, 0.5, i, n);
	case WND_HAMMING:
		return wnd_cosine1(0.54, 0.46, i, n);
	case WND_BLACKMAN:
		return wnd_cosine2(0.42659, 0.49656, 0.076849, i, n);
	case WND_NUTTALL:
		return wnd_cosine3(0.355768, 0.487396, 0.144232, 0.012604, i, n);
	case WND_BLACKMAN_NUTTALL:
		return wnd_cosine3(0.3635819, 0.4891775, 0.1365995, 0.0106411, i, n);
	case WND_BLACKMAN_HARRIS:
		return wnd_cosine3(0.35875, 0.48829, 0.14128, 0.01168, i, n);
	case WND_KAISER:
		return wnd_kaiser(i, n, arg);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* calculate specifed window */
void wnd_buf(float *buf, size_t len, wnd_type_t type, double arg)
{
	size_t i;
	double xw;

	for(i = 0; i < len; i++) {
		xw = wnd_point(i, len, type, arg);
		buf[i] = (float)xw;
	}
}

/* ---------------------------------------------------------------------------------------------- */
