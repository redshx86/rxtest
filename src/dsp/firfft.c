/* ---------------------------------------------------------------------------------------------- */

#include "firfft.h"

/* ---------------------------------------------------------------------------------------------- */

/* Prepare FFT FIR filter data.
 *	fft_len: fft length, power of two,
 *	filt_len: filter kernel length, sub fft length. */
int firfilt_fft_init(firfilt_fft_t *ff, size_t fft_len, size_t filt_len)
{
	if( (fft_len == 0) || ((fft_len & (fft_len - 1)) != 0) )
		return 0;
	if( (filt_len == 0) || (filt_len >= fft_len) )
		return 0;

	// Prepare forward FFT data
	if(fft_init(&(ff->fft_fwd), fft_len, 0))
	{
		// Prepare IFFT data
		if(fft_init(&(ff->fft_back), fft_len, 1))
		{
			// Allocate buffers
			ff->buf_in = _aligned_malloc(sizeof(cpxf_t) * fft_len, 16);
			ff->buf_temp = _aligned_malloc(sizeof(cpxf_t) * fft_len, 16);
			ff->H = _aligned_malloc(sizeof(cpxf_t) * fft_len, 16);
			ff->tail = _aligned_malloc(sizeof(cpxf_t) * filt_len, 16);

			if( (ff->buf_in != NULL) && (ff->buf_temp != NULL) &&
				(ff->H != NULL) && (ff->tail != NULL) )
			{
				// Clear buffers
				memset(ff->buf_in, 0, sizeof(cpxf_t) * fft_len);
				memset(ff->tail, 0, sizeof(cpxf_t) * filt_len);

				// Set variables
				ff->fft_len = fft_len;
				ff->filt_len = filt_len;
				ff->data_len = (fft_len - filt_len);
				ff->num_avail = 0;

				return 1;
			}

			// Cleanup on failure
			_aligned_free(ff->tail);
			_aligned_free(ff->H);
			_aligned_free(ff->buf_temp);
			_aligned_free(ff->buf_in);

			fft_cleanup(&(ff->fft_back));
		}

		fft_cleanup(&(ff->fft_fwd));
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free FFT FIR filter data. */
void firfilt_fft_cleanup(firfilt_fft_t *ff)
{
	// Free buffers
	_aligned_free(ff->tail);
	_aligned_free(ff->H);
	_aligned_free(ff->buf_temp);
	_aligned_free(ff->buf_in);

	// Free prepared FFT data.
	fft_cleanup(&(ff->fft_back));
	fft_cleanup(&(ff->fft_fwd));
}

/* ---------------------------------------------------------------------------------------------- */

/* Set FFT FIR filter coefficients. */
void firfilt_fft_tune(firfilt_fft_t *ff, float *h)
{
	size_t i;

	// Copy kernel to temp buffer
	for(i = 0; i < ff->filt_len; i++) {
		ff->buf_temp[i].re = h[i];
		ff->buf_temp[i].im = 0;
	}
	// Pad kernel with zeros to FFT size
	memset(ff->buf_temp + ff->filt_len, 0,
		ff->data_len * sizeof(cpxf_t));

	// Perform FFT on padded kernel
	fft_process(&(ff->fft_fwd), ff->H, ff->buf_temp);
}

/* ---------------------------------------------------------------------------------------------- */

/* Process buffer with FFT FIR filter.
 *	dst: destination buffer, float complex,
 *	src: source buffer, float compex,
 *	num: number of samples in src buffer,
 *	return: number of samples written to dst buffer.
 * Minimum dst buffer size: fft_len + num. */
size_t firfilt_fft_process(firfilt_fft_t *ff, cpxf_t *dst, cpxf_t *src, size_t num)
{
	size_t num_to_copy, num_out = 0;

	// Have enough samples to process block?
	while(ff->num_avail + num >= ff->data_len)
	{
		// Fill input buffer
		num_to_copy = ff->data_len - ff->num_avail;
		memcpy(ff->buf_in + ff->num_avail, src, num_to_copy * sizeof(cpxf_t));
		src += num_to_copy;
		num -= num_to_copy;

		// Perform forward FFT on input buffer
		fft_process(&(ff->fft_fwd), ff->buf_temp, ff->buf_in);

		// Multiply FFT result by frequency response
		cpxf_block_mul(ff->buf_temp, ff->H, ff->fft_len);

		// Perform IFFT on product
		fft_process(&(ff->fft_back), ff->buf_temp, ff->buf_temp);
		cpxf_block_mulr(ff->buf_temp, (float)(1.0 / ff->fft_len), ff->fft_len);

		// Add tail from previous block to IFFT result
		cpxf_block_add(ff->buf_temp, ff->tail, ff->filt_len);

		// Save tail to use in next block
		memcpy(ff->tail, ff->buf_temp+ff->data_len, ff->filt_len * sizeof(cpxf_t));

		// Copy data to destination buffer
		memcpy(dst, ff->buf_temp, ff->data_len * sizeof(cpxf_t));
		dst += ff->data_len;
		num_out += ff->data_len;

		ff->num_avail = 0;
	}

	// Copy remaining input samples to input buffer
	memcpy(ff->buf_in + ff->num_avail, src, num * sizeof(cpxf_t));
	ff->num_avail += num;

	return num_out;
}

/* ---------------------------------------------------------------------------------------------- */
