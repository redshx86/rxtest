/* ---------------------------------------------------------------------------------------------- */

#include "fft.h"

/* ---------------------------------------------------------------------------------------------- */

void __cdecl fft_bfly_sse(fft_params_t *ctx, cpxf_t *buf);

/* ---------------------------------------------------------------------------------------------- */

/* Build bit reversed map. */
static void fft_brmap(size_t num, size_t stride, size_t offset, size_t *buf)
{
	if(num != 1) {
		fft_brmap(num / 2, stride * 2, offset, buf);
		fft_brmap(num / 2, stride * 2, offset + stride, buf + num / 2);
	} else {
		*buf = offset;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Bit reversal permutation. */
static void fft_bperm(size_t num, size_t *brmap, cpxf_t *dst, cpxf_t *src)
{
	size_t i, k;

	for(i = 0; i < num; i++) {
		k = brmap[i];
		cpxf_copy(dst + i, src + k);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* In-place bit reversal permutation. */
static void fft_bperm1(size_t num, size_t *brmap, cpxf_t *buf)
{
	size_t i, k;

	for(i = 0; i < num; i++)
	{
		k = brmap[i];
		if(i < k) {
			cpxf_swap(buf + i, buf + k);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Radix 2 FFT butterfly. */
static void fft_bfly(fft_params_t *ctx, cpxf_t *buf)
{
	cpxf_t *pevn, *podd, *ptwid, tmp;
	size_t seglen, nsegs;
	size_t level, seg, i;

	seglen = 2;
	nsegs = ctx->length / seglen;

	for(level = 0; level < ctx->bitnum; level++)
	{
		for(seg = 0; seg < nsegs; seg++)
		{
			pevn = buf + seg * seglen;
			podd = pevn + seglen / 2;
			ptwid = ctx->twiddle;

			for(i = 0; i < seglen / 2; i++)
			{
				cpxf_mul(&tmp, podd, ptwid);
				cpxf_sub(podd, pevn, &tmp);
				cpxf_add(pevn, pevn, &tmp);
				pevn++; podd++;
				ptwid += nsegs;
			}
		}
		seglen *= 2;
		nsegs /= 2;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Perofrm FFT.
 *	dst: destination buffer, complex float,
 *	src: source buffer, complex float.
 * Note: src & dst can be same buffer for in-place transform.
 * Note: 1/N term omited in inverse transform. */
void fft_process(fft_params_t *ctx, cpxf_t *dst, cpxf_t *src)
{
	/* Bit reversal */
	if(src == dst) {
		fft_bperm1(ctx->length, ctx->brmap, dst);
	} else {
		fft_bperm(ctx->length, ctx->brmap, dst, src);
	}

	/* Butterfly combining */
	if(is_sse_enabled) {
		fft_bfly_sse(ctx, dst);
	} else {
		fft_bfly(ctx, dst);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Prepare FFT.
 *	length: FFT length, power of two only,
 *	inverse: nonzero to prepare IFFT. */
int fft_init(fft_params_t *ctx, size_t length, int inverse)
{
	size_t i, bitnum;
	double step;

	if( (length == 0) || ((length & (length - 1)) != 0) )
		return 0;

	for(bitnum = 0; (1U << bitnum) != length; bitnum++)
		;

	ctx->brmap = malloc(sizeof(size_t) * length);
	ctx->twiddle = malloc(sizeof(cpxf_t) * length / 2);

	if( (ctx->brmap != NULL) && (ctx->twiddle != NULL) )
	{
		ctx->length = length;
		ctx->bitnum = bitnum;

		fft_brmap(ctx->length, 1, 0, ctx->brmap);

		step = -2.0 * M_PI / length;
		if(inverse) step = -step;
		for(i = 0; i < length / 2; i++)
			cpxf_expj(ctx->twiddle + i, i * step);

		return 1;
	}

	free(ctx->twiddle);
	free(ctx->brmap);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free FFT data. */
void fft_cleanup(fft_params_t *ctx)
{
	free(ctx->twiddle);
	free(ctx->brmap);
}

/* ---------------------------------------------------------------------------------------------- */
