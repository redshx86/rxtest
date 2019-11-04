/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <math.h>
#include <string.h>

/* ---------------------------------------------------------------------------------------------- */

typedef enum wnd_type {

	WND_HANN,
	WND_HAMMING,
	WND_BLACKMAN,
	WND_NUTTALL,
	WND_BLACKMAN_NUTTALL,
	WND_BLACKMAN_HARRIS,

	WND_KAISER,

	WND_COUNT

} wnd_type_t;

/* ---------------------------------------------------------------------------------------------- */

/* calculate specifed window point */
double wnd_point(size_t i, size_t n, wnd_type_t type, double arg);

/* calculate specifed window */
void wnd_buf(float *buf, size_t len, wnd_type_t type, double arg);

/* ---------------------------------------------------------------------------------------------- */
