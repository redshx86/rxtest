/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <math.h>
#include <float.h>
#include "../util/numparse.h"
#include "../util/db.h"

/* ---------------------------------------------------------------------------------------------- */

typedef enum specscale_map_mode {
	SPECSCALE_MAP_PEAK,
	SPECSCALE_MAP_AVG,
	SPECSCAL_MAP_MODE_COUNT
} specscale_map_mode_t;

/* ---------------------------------------------------------------------------------------------- */

int scale_step_mag(double x_0, double x_1, int dist, int s_min, double *pstep);
int scale_step_freq(double x_0, double x_1, int dist, int s_min, double *pstep);

double scale_first_mark(double x_0, double x_step, double eps);

int scale_mark_pos(double x_0, double x_1, double x, int dist, int invert);

/* ---------------------------------------------------------------------------------------------- */

void scale_pwr_spect(float *out_db, int out_len, double out_f_0, double out_f_1,
					 float *in_pwr, int in_len, double in_f_0, double in_f_1,
					 specscale_map_mode_t mode);

void scale_mag(int *out_pos, int dst_range, double m_0, double m_1,
			   float *in_db, int length);

/* ---------------------------------------------------------------------------------------------- */
