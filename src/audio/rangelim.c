/* ---------------------------------------------------------------------------------------------- */

#include "rangelim.h"

/* ---------------------------------------------------------------------------------------------- */

/* Compute alpha parameter by range and threshold,
 *	(1 + alpha) ^ (1 / alpha) = exp( (1 - thres) / (range - thres) )
 * where
 *	1.01 <= range <= 100.00
 *	0.00 <= thres <= 0.99 */
double rangelim_compute_alpha(double range, double thres)
{
	int i;
	double xl, xh, x = 0;
	double a, b, eps;

	xl = 0.01;
	xh = 1e+6;
	eps = 1e-18;

	b = exp( (1.0 - thres) / (range - thres) );

	for(i = 0; i < 100; i++)
	{
		x = 0.5 * (xl + xh);
		a = pow(1.0 + x, 1.0 / x);

		if(fabs(a - b) < eps)
			break;

		if(a > b) xl = x;
		else xh = x;
	}

	return x;
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize range limiter.
 *	range: input samples range to fit, 1.01 to 100.00,
 *	thres: limiting threshold, 0.00 to 0.99.
 * (no cleanup needed) */
int rangelim_init(rangelim_params_t *ctx, double range, double thres)
{
	double alpha;

	/* check parameters */
	if( (range < 1.01) || (range > 100.0) || (thres < 0) || (thres > 0.99) )
		return 0;

	ctx->range = range;
	ctx->thres = thres;

	alpha = rangelim_compute_alpha(range, thres);
	ctx->b = alpha / (range - thres);
	ctx->c = 1.0 / log(1.0 + alpha);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* Perform range limiting on buffer.
 *	dst: destination buffer, float, may be equal to src,
 *	src: source buffer, float, may be equal to dst,
 *	num: number of samples to process. */
void rangelim_process(rangelim_params_t *ctx, float *dst, float *src, size_t num)
{
	float x, y;

	while(num--)
	{
		x = *(src++);

		if(fabs(x) < ctx->thres)
		{
			y = x;
		}
		else
		{
			if(x < 0)
			{
				y = (float)(-ctx->thres - (1.0 - ctx->thres) *
					log(1.0 - (x + ctx->thres) * ctx->b) * ctx->c);
			}
			else
			{
				y = (float)(ctx->thres + (1.0 - ctx->thres) *
					log(1.0 + (x - ctx->thres) * ctx->b) * ctx->c);
			}
		}

		*(dst++) = y;
	}
}

/* ---------------------------------------------------------------------------------------------- */
