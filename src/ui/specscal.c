/* ---------------------------------------------------------------------------------------------- */

#include "specscal.h"

/* ---------------------------------------------------------------------------------------------- */

/* magnitude scale step values, dB */
static double specscale_mag_step[] =
{
	 0.1,
	 0.2,
	 0.3,
	 0.5,
	 1.0,
	 2.0,
	 3.0,
	 5.0,
	10.0,
	20.0,
	30.0,
	50.0
};

/* ---------------------------------------------------------------------------------------------- */

/* frequency scale step values, dB */
static double specscale_freq_step[] =
{
	1.0e0, /*2.5e0,*/ 5.0e0,
	1.0e1, 2.5e1, 5.0e1,
	1.0e2, 2.5e2, 5.0e2,
	1.0e3, /*2.5e3,*/ 5.0e3,
	1.0e4, 2.5e4, 5.0e4,
	1.0e5, 2.5e5, 5.0e5,
	1.0e6, /*2.5e6,*/ 5.0e6,
	1.0e7, 2.5e7, 5.0e7,
	1.0e8, 2.5e8, 5.0e8,
	1.0e9, /*2.5e9,*/ 5.0e9
};

/* ---------------------------------------------------------------------------------------------- */

int scale_mark_pos(double x_0, double x_1, double x, int dist, int invert)
{
	double dx;
	int pos;

	dx = (x_1 - x_0) / (double)(dist - 1);

	pos = (int)( (x - x_0) / dx + 0.5 );

	if(invert) {
		pos = dist - pos - 1;
	}

	return pos;
}

/* ---------------------------------------------------------------------------------------------- */

static int scale_step_sel(double x_0, double x_1, int dist, int s_min,
						  double *grid, int gridcount, double *pstep)
{
	int i;
	double s;

	for(i = 0; i < gridcount; i++)
	{
		s = (double)(dist - 1) * (grid[i] / (x_1 - x_0));

		if(s >= s_min) {
			*pstep = grid[i];
			return 1;
		}
	}

	*pstep = 0;
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int scale_step_mag(double x_0, double x_1, int dist, int s_min, double *pstep)
{
	int stepcount;

	stepcount = sizeof(specscale_mag_step) / sizeof(double);
	return scale_step_sel(x_0, x_1, dist, s_min,
		specscale_mag_step, stepcount, pstep);
}

/* ---------------------------------------------------------------------------------------------- */

int scale_step_freq(double x_0, double x_1, int dist, int s_min, double *pstep)
{
	int stepcount;

	stepcount = sizeof(specscale_freq_step) / sizeof(double);
	return scale_step_sel(x_0, x_1, dist, s_min,
		specscale_freq_step, stepcount, pstep);
}

/* ---------------------------------------------------------------------------------------------- */

double scale_first_mark(double x_0, double x_step, double eps)
{
	return (ceil((x_0 - eps) / x_step) * x_step);
}

/* ---------------------------------------------------------------------------------------------- */

void scale_pwr_spect(float *out_db, int out_len, double out_f_0, double out_f_1,
					 float *in_pwr, int in_len, double in_f_0, double in_f_1,
					 specscale_map_mode_t mode)
{
	int out_j, in_j, in_j_0, in_j_1;
	double out_df, in_df;
	double h_0, h_1;
	double in_jf_0, in_jf_1;
	double pwr, w;

	for(out_j = 0; out_j < out_len; out_j++)
		out_db[out_j] = -FLT_MAX;

	out_df = (out_f_1 - out_f_0) / (out_len - 1);
	in_df = (in_f_1 - in_f_0) / (in_len - 1);

	for(out_j = 0; out_j < out_len; out_j++)
	{
		h_0 = out_f_0 + out_df * (out_j - 0.5);
		h_1 = out_f_0 + out_df * (out_j + 0.5);

		if((h_0 <= in_f_1) && (h_1 >= in_f_0))
		{
			if(h_0 < in_f_0)
				h_0 = in_f_0;
			if(h_1 > in_f_1)
				h_1 = in_f_1;

			in_jf_0 = (h_0 - in_f_0) / in_df;
			in_jf_1 = (h_1 - in_f_0) / in_df;

			in_j_0 = (int)(in_jf_0 + 0.5 + 1e-3);
			in_j_1 = (int)(in_jf_1 + 0.5 - 1e-3);

			switch(mode)
			{
			case SPECSCALE_MAP_PEAK:
				pwr = in_pwr[in_j_0];
				for(in_j = in_j_0 + 1; in_j <= in_j_1; in_j++)
				{
					if(in_pwr[in_j] > pwr)
						pwr = in_pwr[in_j];
				}
				out_db[out_j] = (float)LINP2DB(pwr);
				break;

			case SPECSCALE_MAP_AVG:
				if(in_j_0 == in_j_1) {
					pwr = in_pwr[in_j_0];
				} else {
					pwr = 0;

					w = 1.0 - (in_jf_0 - (in_j_0 - 0.5));
					pwr += in_pwr[in_j_0] * w;

					w = in_jf_1 - (in_j_1 - 0.5);
					pwr += in_pwr[in_j_1] * w;

					for(in_j = in_j_0 + 1; in_j <= in_j_1 - 1; in_j++)
						pwr += in_pwr[in_j];

					pwr /= in_jf_1 - in_jf_0;
				}
				out_db[out_j] = (float)LINP2DB(pwr);
				break;
			}

		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

void scale_mag(int *out_pos, int dst_range, double m_0, double m_1,
			   float *in_db, int length)
{
	double dst_dm;
	float m;
	int i, dst_j;

	dst_dm = (m_1 - m_0) / (dst_range - 1);

	for(i = 0; i < length; i++)
	{
		m = in_db[i];

		if(m < m_0) {
			out_pos[i] = -1;
		} else if(m > m_1) {
			out_pos[i] = dst_range - 1;
		} else {
			dst_j = (int)((m - m_0) / dst_dm + 0.5);
			out_pos[i] = dst_j;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */
