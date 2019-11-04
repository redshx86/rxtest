/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "../dsp/dsp.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct levelmtr_sample {
	double mag;
	double peak;
} levelmtr_sample_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct levelmtr {

	CRITICAL_SECTION csec;

	size_t nch;

	size_t cap;
	size_t ofs;
	size_t num;
	unsigned int *dst_tick;
	levelmtr_sample_t **data;

	double tick_per_samp;
	size_t interval;
	size_t nsamp;
	float **buf;

} levelmtr_t;

/* ---------------------------------------------------------------------------------------------- */

int levelmtr_init_fs(levelmtr_t *c, size_t nch, size_t cap,
					 unsigned int fs, double tick_val, double meas_tick_int);

int levelmtr_init(levelmtr_t *c, size_t nch, size_t cap,
				  double tick_per_samp, double meas_tick_int);

void levelmtr_free(levelmtr_t *c);

void levelmtr_write(levelmtr_t *c, void *data, int is_data_cpx, size_t nsamp, unsigned int tick);

int levelmtr_read(levelmtr_t *c, unsigned int tick, double *pmag);

int levelmtr_read_peak(levelmtr_t *c, unsigned int tick_peak_from, unsigned int tick,
					   double *pmag, double *ppeak);

/* ---------------------------------------------------------------------------------------------- */
