/* ---------------------------------------------------------------------------------------------- */

#include "firfilt.h"

/* ---------------------------------------------------------------------------------------------- */

static size_t __cdecl firfilt_exec_fpu(firfilt_state_t *filt, float *buf, size_t nsamp);
static size_t __cdecl firfilt_exec_cpx_fpu(firfilt_state_t *filt, cpxf_t *buf, size_t nsamp);

size_t __cdecl firfilt_exec_sse(firfilt_state_t *filt, float *buf, size_t nsamp);
size_t __cdecl firfilt_exec_cpx_sse(firfilt_state_t *filt, cpxf_t *buf, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */

/* Initialize FIR filter state.
 *	length		: filter length,
 *	decimf		: decimation factor,
 *	is_cpx		: nonzero if filtered data is complex. */
int firfilt_init(firfilt_state_t *filt, size_t length, size_t decimf, int is_cpx)
{
	size_t elemsize;

	if( (!length) || (!decimf) || (decimf > length))
		return 0;

	filt->length = length;
	filt->decimf = decimf;
	filt->skipcnt = 0;

	elemsize = is_cpx ? sizeof(cpxf_t) : sizeof(float);
	filt->coef = _aligned_malloc(length * sizeof(float), 16);
	filt->z = _aligned_malloc(length * elemsize, 16);

	if((filt->coef != NULL) && (filt->z != NULL))
	{
		memset(filt->z, 0, length * elemsize);

		if( is_sse_enabled && (length >= FIRFILT_MIN_SSE_LENGTH) ) {
			filt->exec_fn = (is_cpx ?
				(firfilt_exec_fn)firfilt_exec_cpx_sse :
				(firfilt_exec_fn)firfilt_exec_sse);
		} else {
			filt->exec_fn = (is_cpx ?
				(firfilt_exec_fn)firfilt_exec_cpx_fpu :
				(firfilt_exec_fn)firfilt_exec_fpu);
		}

		return 1;
	}

	_aligned_free(filt->z);
	_aligned_free(filt->coef);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free FIR filter state. */
void firfilt_cleanup(firfilt_state_t *filt)
{
	_aligned_free(filt->z);
	_aligned_free(filt->coef);
}

/* ---------------------------------------------------------------------------------------------- */

/* Set FIR filter coefficients.
 *	coeff		: coefficient buffer. */
void firfilt_set_coefs(firfilt_state_t *filt, float *coef)
{
	size_t i;

	/* load coefs in reverse */
	for(i = 0; i < filt->length; i++)
		filt->coef[i] = coef[filt->length - i - 1];
}

/* ---------------------------------------------------------------------------------------------- */

/* Execute FIR filter for real float buffer.
 *	buf			: data buffer (input/output),
 *	nsamp		: number of input samples.
 * Returns: number of output samples. */
static size_t __cdecl firfilt_exec_fpu(firfilt_state_t *filt, float *buf, size_t nsamp)
{
	size_t num_out = 0;
	size_t n, i;
	float *src, *z;
	double accum;

	src = buf;
	z = filt->z;

	while(nsamp)
	{
		/* downsample output */
		if(!filt->skipcnt)
		{
			/* compute output sample */
			accum = 0;
			for(i = 0; i < filt->length - 1; i++) {
				accum += z[i] * filt->coef[i];
				z[i] = z[i + 1];
			}
			z[i - 1] = *(src++);
			accum += z[i - 1] * filt->coef[i];
			/* store output sample */
			*(buf++) = (float)accum;
			num_out++;
			/* update counters */
			nsamp--;
			filt->skipcnt = filt->decimf - 1;
		}
		else
		{
			/* skip output samples */
			n = min(filt->skipcnt, nsamp);
			for(i = 0; i < filt->length - n - 1; i++)
				z[i] = z[i + n];
			for(; i < filt->length - 1; i++)
				z[i] = *(src++);
			/* update counters */
			nsamp -= n;
			filt->skipcnt -= n;
		}
	}

	return num_out;
}

/* ---------------------------------------------------------------------------------------------- */

/* Execute FIR filter for complex float buffer.
 *	buf			: data buffer (input/output),
 *	nsamp		: number of input samples.
 * Returns: number of output samples. */
static size_t __cdecl firfilt_exec_cpx_fpu(firfilt_state_t *filt, cpxf_t *buf, size_t nsamp)
{
	size_t num_out = 0;
	size_t n, i;
	cpxf_t *src, *z;
	cpxf_t temp, accum;

	src = buf;
	z = filt->z;

	while(nsamp)
	{
		/* downsample output */
		if(!filt->skipcnt)
		{
			/* compute output sample */
			cpxf_zero(&accum);
			for(i = 0; i < filt->length - 1; i++) {
				cpxf_mulr(&temp, z+i, filt->coef[i]);
				cpxf_add(&accum, &accum, &temp);
				cpxf_copy(z+i, z+i+1);
			}
			cpxf_copy(z+i-1, src++);
			cpxf_mulr(&temp, z+i-1, filt->coef[i]);
			cpxf_add(&accum, &accum, &temp);
			/* store output sample */
			cpxf_copy(buf++, &accum);
			num_out++;
			/* update counters */
			nsamp--;
			filt->skipcnt = filt->decimf - 1;
		}
		else
		{
			/* skip output samples */
			n = min(filt->skipcnt, nsamp);
			for(i = 0; i < filt->length - n - 1; i++)
				cpxf_copy(z+i, z+i+n);
			for(; i < filt->length - 1; i++)
				cpxf_copy(z+i, src++);
			/* update counters */
			nsamp -= n;
			filt->skipcnt -= n;
		}
	}

	return num_out;
}

/* ---------------------------------------------------------------------------------------------- */
