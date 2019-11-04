/* ---------------------------------------------------------------------------------------------- */

#include "cvtbuf.h"

/* ---------------------------------------------------------------------------------------------- */

/* Convert samples, float to unsigned char (0..255, center = 128). */
void buf_f2uc(unsigned char *dst, float *src, size_t nsamp)
{
	int temp;

	while(nsamp--)
	{
		temp = (int)( *(src++) * 128.0f + 128.5f );

		if(temp < 0)
			temp = 0;
		else if(temp > 255)
			temp = 255;

		*(dst++) = (unsigned char)temp;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Convert samples, float to signed short. */
void buf_f2ss(short *dst, float *src, size_t nsamp)
{
	int temp;

	while(nsamp--)
	{
		temp = (int)( *(src++) * 32768.0f + 32768.5f );

		if(temp < 0)
			temp = 0;
		else if(temp > 65535)
			temp = 65535;

		*(dst++) = (short)(temp - 32768);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Convert samples, unsigned char (0..255, center = 128) to float. */
void buf_uc2f(float *dst, unsigned char *src, size_t nsamp)
{
	while(nsamp--)
	{
		*(dst++) = ((float)(*(src++)) - 128.0f) / 128.0f;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Convert samples, signed short to float. */
void buf_ss2f(float *dst, short *src, size_t nsamp)
{
	while(nsamp--)
	{
		*(dst++) = (float)(*(src++)) / 32768.0f;
	}
}

/* ---------------------------------------------------------------------------------------------- */
