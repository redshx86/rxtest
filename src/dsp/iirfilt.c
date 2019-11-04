/* ---------------------------------------------------------------------------------------------- */

#include "iirfilt.h"

/* ---------------------------------------------------------------------------------------------- */

/* Initialize 1st order IIR filter.
 *	b: Tf numerator coefficients, real float, 2 points,
 *	a: Tf denominator coefficients, real float, 2 points.
 * Note: no cleanup needed. */
void iirfilt_init1(iirfilt_state1_t *f, float *b, float *a)
{
	float normf;

	/* fill coefficients */
	normf = 1.0f / a[0];
	f->b0 = b[0] * normf;
	f->b1 = b[1] * normf;
	f->a1 = -a[1] * normf;

	/* clear delay lines */
	cpxf_zero(&(f->x1));
	cpxf_zero(&(f->y1));
}

/* ---------------------------------------------------------------------------------------------- */

/* Process real float buffer with 1st order IIR filter.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process1(iirfilt_state1_t *f, float *dst, float *src, size_t num)
{
	float x, y;

	/* for each sample */
	while(num--)
	{
		/* load input sample */
		x = *(src++);

		/* calculate filter output */
		y = x * f->b0 + f->x1.re * f->b1 + f->y1.re * f->a1;

		/* push delay lines */
		f->x1.re = x;
		f->y1.re = y;

		/* store output sample */
		*(dst++) = y;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Process complex float buffer with 1st order IIR filter.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process1_complex(iirfilt_state1_t *f, cpxf_t *dst, cpxf_t *src, size_t num)
{
	cpxf_t x, y;

	/* for each sample */
	while(num--)
	{
		/* load input sample */
		cpxf_copy(&x, src++);

		/* calculate filter output */
		y.re = x.re * f->b0 + f->x1.re * f->b1 + f->y1.re * f->a1;
		y.im = x.im * f->b0 + f->x1.im * f->b1 + f->y1.im * f->a1;

		/* push delay lines */
		cpxf_copy(&(f->x1), &x);
		cpxf_copy(&(f->y1), &y);

		/* store output sample */
		cpxf_copy(dst++, &y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize 2nd order IIR filter.
 *	b: Tf numerator coefficients, real float, 3 points,
 *	a: Tf denominator coefficients, real float, 3 points.
 * Note: no cleanup needed. */
void iirfilt_init2(iirfilt_state2_t *f, float *b, float *a)
{
	float normf;

	/* fill coefficients */
	normf = 1.0f / a[0];
	f->b0 = b[0] * normf;
	f->b1 = b[1] * normf;
	f->b2 = b[2] * normf;
	f->a1 = -a[1] * normf;
	f->a2 = -a[2] * normf;

	/* clear delay lines */
	cpxf_zero(&(f->x1));
	cpxf_zero(&(f->x2));
	cpxf_zero(&(f->y1));
	cpxf_zero(&(f->y2));
}

/* ---------------------------------------------------------------------------------------------- */

/* Process real float buffer with 2nd order IIR filter.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process2(iirfilt_state2_t *f, float *dst, float *src, size_t num)
{
	float x, y;

	/* for each sample */
	while(num--)
	{
		/* load input sample */
		x = *(src++);

		/* calculate filter output */
		y = x * f->b0 + f->x1.re * f->b1 + f->x2.re * f->b2
			+ f->y1.re * f->a1 + f->y2.re * f->a2;

		/* push delay lines */
		f->x2.re = f->x1.re;
		f->x1.re = x;
		f->y2.re = f->y1.re;
		f->y1.re = y;

		/* store output sample */
		*(dst++) = y;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Process complex float buffer with 2nd order IIR filter.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iirfilt_process2_complex(iirfilt_state2_t *f, cpxf_t *dst, cpxf_t *src, size_t num)
{
	cpxf_t x, y;

	/* for each sample */
	while(num--)
	{
		/* load input sample */
		cpxf_copy(&x, src++);

		/* calculate filter output */
		y.re = x.re * f->b0 + f->x1.re * f->b1 + f->x2.re * f->b2
			+ f->y1.re * f->a1 + f->y2.re * f->a2;
		y.im = x.im * f->b0 + f->x1.im * f->b1 + f->x2.im * f->b2
			+ f->y1.im * f->a1 + f->y2.im * f->a2;

		/* push delay lines */
		cpxf_copy(&(f->x2), &(f->x1));
		cpxf_copy(&(f->x1), &x);
		cpxf_copy(&(f->y2), &(f->y1));
		cpxf_copy(&(f->y1), &y);

		/* store output sample */
		cpxf_copy(dst++, &y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize IIR filter.
 *	blen: number of tf numerator coefficients,
 *	alen: number of tf denumerator coefficients,
 *	is_complex: nonzero to allocate delay lines for complex values. */
int iirfilt_init(iirfilt_state_t *f, size_t blen, size_t alen, int is_complex)
{
	if( (blen == 0) || (alen == 0) )
		return 0;

	f->blen = blen;
	f->alen = alen;

	/* allocate coefficient buffers */
	f->b = malloc(sizeof(float) * blen);
	f->a = malloc(sizeof(float) * alen);

	/* allocate delay lines */
	if(is_complex) {
		f->x = calloc(sizeof(cpxf_t), blen);
		f->y = calloc(sizeof(cpxf_t), alen);
	} else {
		f->x = calloc(sizeof(float), blen);
		f->y = calloc(sizeof(float), alen);
	}

	/* check for allocation errors */
	if( (f->a != NULL) && (f->b != NULL) && (f->x != NULL) && (f->y != NULL) )
		return 1;

	/* free memory in case of error */
	free(f->y);
	free(f->x);
	free(f->a);
	free(f->b);
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free IIR filter stuff. */
void iirfilt_cleanup(iirfilt_state_t *f)
{
	free(f->y);
	free(f->x);
	free(f->a);
	free(f->b);
}

/* ---------------------------------------------------------------------------------------------- */

/* Set IIR filter coefficients.
 *	b: tf numerator coefficients, real float,
 *	a: tf denumerator coefficients, real float. */
void iirfilt_tune(iirfilt_state_t *f, float *b, float *a)
{
	float normf;
	size_t i;

	normf = 1.0f / a[0];

	for(i = 0; i < f->blen; i++)
		f->b[i] = b[i] * normf;

	f->a[0] = 1.0f;
	for(i = 1; i < f->alen; i++)
		f->a[i] = -a[i] * normf;
}

/* ---------------------------------------------------------------------------------------------- */

/* Process real float buffer with IIR filter.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of sampples.
 * Note: src and dst can be same buffer. */
void iirfilt_process(iirfilt_state_t *f, float *dst, float *src, size_t num)
{
	float *xp, *yp;
	float sum;
	size_t i;

	xp = f->x;
	yp = f->y;

	/* for each sample */
	while(num--)
	{
		/* put input sample to delay line */
		xp[0] = *(src++);

		/* calculate filter output */
		sum = 0;
		for(i = 0; i < f->blen; i++)
			sum += xp[i] * f->b[i];
		for(i = 1; i < f->alen; i++)
			sum += yp[i] * f->a[i];

		/* push delay lines */
		for(i = f->blen-1; i >= 1; i--)
			xp[i] = xp[i-1];
		for(i = f->alen-1; i >= 2; i--)
			yp[i] = yp[i-1];

		/* put output sample to delay line */
		yp[1] = sum;

		/* store output sample */
		*(dst++) = sum;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Process complex float buffer with IIR filter.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of sampples.
 * Note: src and dst can be same buffer. */
void iirfilt_process_complex(iirfilt_state_t *f, cpxf_t *dst, cpxf_t *src, size_t num)
{
	cpxf_t *xp, *yp;
	cpxf_t temp, sum;
	size_t i;

	xp = f->x;
	yp = f->y;

	/* for each sample */
	while(num--)
	{
		/* put input sample to delay line */
		cpxf_copy(xp, src++);

		/* calculate filter output */
		cpxf_zero(&sum);
		for(i = 0; i < f->blen; i++) {
			cpxf_mulr(&temp, xp+i, f->b[i]);
			cpxf_add(&sum, &sum, &temp);
		}
		for(i = 1; i < f->alen; i++) {
			cpxf_mulr(&temp, yp+i, f->a[i]);
			cpxf_add(&sum, &sum, &temp);
		}

		/* push delay lines */
		for(i = f->blen-1; i >= 1; i--)
			cpxf_copy(xp+i, xp+i-1);
		for(i = f->alen-1; i >= 2; i--)
			cpxf_copy(yp+i, yp+i-1);

		/* put output sample to delay line */
		cpxf_copy(yp+1, &sum);

		/* store output sample */
		cpxf_copy(dst++, &sum);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize IIR DC remover (1st order IIR filter with
 *	b0 = 1, b1 = -1, a1 = -1 + alpha. */
void iir_dcrem_init(iirfilt_state1_t *f, double alpha)
{
	/* fill coefficients */
	f->b0 = 1.0f;
	f->b1 = -1.0f;
	f->a1 = (float)(1.0 - alpha);

	/* clear delay lines */
	cpxf_zero(&(f->x1));
	cpxf_zero(&(f->y1));
}

/* ---------------------------------------------------------------------------------------------- */

/* Process DC removal on real float buffer.
 *	dst: destination buffer, real float,
 *	src: source buffer, real float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iir_dcrem_process(iirfilt_state1_t *f, float *dst, float *src, size_t num)
{
	float x, y;

	/* for each sample */
	while(num--)
	{
		/* load input sample */
		x = *(src++);

		/* calculate filter output */
		y = x - f->x1.re + f->y1.re * f->a1;

		/* push delay lines */
		f->x1.re = x;
		f->y1.re = y;

		/* store output sample */
		*(dst++) = y;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Process DC removal on complex float buffer.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float,
 *	num: number of samples.
 * Note: src and dst can be same buffer. */
void iir_dcrem_process_complex(iirfilt_state1_t *f, cpxf_t *dst, cpxf_t *src, size_t num)
{
	cpxf_t x, y;

	/* for each sample */
	while(num--)
	{
		/* load input sample */
		cpxf_copy(&x, src++);

		/* calculate filter output */
		y.re = x.re - f->x1.re + f->y1.re * f->a1;
		y.im = x.im - f->x1.im + f->y1.im * f->a1;

		/* push delay lines */
		cpxf_copy(&(f->x1), &x);
		cpxf_copy(&(f->y1), &y);

		/* store output sample */
		cpxf_copy(dst++, &y);
	}
}

/* ---------------------------------------------------------------------------------------------- */
