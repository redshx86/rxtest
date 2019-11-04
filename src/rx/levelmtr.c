/* ---------------------------------------------------------------------------------------------- */

#include "levelmtr.h"

/* ---------------------------------------------------------------------------------------------- */

int levelmtr_init_fs(levelmtr_t *c, size_t nch, size_t cap,
					 unsigned int fs, double tick_val, double meas_tick_int)
{
	return levelmtr_init(c, nch, cap, 1.0 / (tick_val * (double)fs), meas_tick_int);
}

/* ---------------------------------------------------------------------------------------------- */

int levelmtr_init(levelmtr_t *c, size_t nch, size_t cap,
				  double tick_per_samp, double meas_tick_int)
{
	size_t i;
	int error;

	InitializeCriticalSection(&(c->csec));

	c->nch = nch;

	c->cap = cap;
	c->ofs = 0;
	c->num = 0;

	c->tick_per_samp = tick_per_samp;
	c->interval = (size_t)(meas_tick_int / tick_per_samp + 0.5);
	c->nsamp = 0;

	c->data = calloc(nch, sizeof(void*));
	c->buf = calloc(nch, sizeof(void*));
	c->dst_tick = malloc(cap * sizeof(int));

	if((c->data != NULL) && (c->buf != NULL) && (c->dst_tick != NULL))
	{
		error = 0;

		for(i = 0; i < nch; i++)
		{
			c->data[i] = malloc(cap * sizeof(levelmtr_sample_t));
			c->buf[i] = malloc(c->interval * sizeof(float));
			if((c->data[i] == NULL) || (c->buf[i] == NULL))
				error = 1;
			
		}

		if(!error)
			return 1;

		for(i = 0; i < nch; i++)
		{
			free(c->buf[i]);
			free(c->data[i]);
		}
	}

	free(c->dst_tick);
	free(c->buf);
	free(c->data);

	DeleteCriticalSection(&(c->csec));

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void levelmtr_free(levelmtr_t *c)
{
	size_t i;

	for(i = 0; i < c->nch; i++)
	{
		free(c->buf[i]);
		free(c->data[i]);
	}

	free(c->dst_tick);
	free(c->buf);
	free(c->data);

	DeleteCriticalSection(&(c->csec));
}

/* ---------------------------------------------------------------------------------------------- */

void levelmtr_write(levelmtr_t *c, void *data, int is_data_cpx, size_t nsamp, unsigned int tick)
{
	size_t ch, nsamp_need, nsamp_copy, n;
	cpxf_t *data_cpx, *dcp;
	float *data_float, *dfp, *buf_ptr, val;
	double sumsqr, meansqr, peakpwr;
	size_t i, index;
	double tick_ofs;

	EnterCriticalSection(&(c->csec));

	data_cpx = data;
	data_float = data;
	
	tick_ofs = 0.0;

	while(nsamp != 0)
	{
		nsamp_need = c->interval - c->nsamp;
		nsamp_copy = min(nsamp, nsamp_need);

		/* copy input power samples to buffer */
		for(ch = 0; ch < c->nch; ch++)
		{
			buf_ptr = c->buf[ch] + c->nsamp;

			if(is_data_cpx)
			{
				dcp = data_cpx + ch;
				for(n = nsamp_copy; n != 0; n--)
				{
					*(buf_ptr++) = cpxf_modsqr(dcp);
					dcp += c->nch;
				}
			}
			else
			{
				dfp = data_float + ch;
				for(n = nsamp_copy; n != 0; n--)
				{
					val = *dfp;
					dfp += c->nch;
					*(buf_ptr++) = val * val;
				}
			}
		}

		c->nsamp += nsamp_copy;

		data_cpx += nsamp_copy * c->nch;
		data_float += nsamp_copy * c->nch;
		nsamp -= nsamp_copy;

		/* is frame completed? */
		if(c->nsamp == c->interval)
		{
			/* calculate frame index */
			if(c->num != c->cap)
			{
				index = c->ofs + c->num;
				if(index >= c->cap)
					index -= c->cap;
				c->num++;
			}
			else
			{
				index = c->ofs;
				c->ofs++;
				if(c->ofs == c->cap)
					c->ofs = 0;
			}

			/* calculate frame rms and peak */
			for(ch = 0; ch < c->nch; ch++)
			{
				peakpwr = 0.0;
				sumsqr = 0.0;

				buf_ptr = c->buf[ch];

				for(i = 0; i < c->nsamp; i++)
				{
					if(*buf_ptr > peakpwr)
						peakpwr = *buf_ptr;
					sumsqr += *buf_ptr;
					buf_ptr++;
				}
				meansqr = sumsqr / (double)(c->nsamp);

				c->data[ch][index].mag = sqrt(meansqr);
				c->data[ch][index].peak = sqrt(peakpwr);
			}

			/* calculate frame output tick */
			c->dst_tick[index] = (unsigned int)(tick + tick_ofs + 0.5);

			/* reset buffer */
			c->nsamp = 0;
		}

		/* update tick offset */
		tick_ofs += c->tick_per_samp * nsamp_copy;
	}

	LeaveCriticalSection(&(c->csec));
}

/* ---------------------------------------------------------------------------------------------- */

int levelmtr_read(levelmtr_t *c, unsigned int tick, double *pmag)
{
	int status = 0;
	size_t ch, i, index;
	int dt;

	EnterCriticalSection(&(c->csec));

	for(i = 0; i < c->num; i++)
	{
		index = c->ofs + (c->num - 1UL) - i;
		if(index >= c->cap)
			index -= c->cap;

		dt = tick - c->dst_tick[index];
		
		if(dt >= 0)
		{
			for(ch = 0; ch < c->nch; ch++)
				pmag[ch] = c->data[ch][index].mag;
			status = 1;
			break;
		}
	}

	LeaveCriticalSection(&(c->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

int levelmtr_read_peak(levelmtr_t *c, unsigned int tick_peak_from, unsigned int tick,
					   double *pmag, double *ppeak)
{
	size_t ch, i, index;
	int dt, range_found = 0;

	memset(pmag, 0, c->nch * sizeof(double));
	memset(ppeak, 0, c->nch * sizeof(double));

	EnterCriticalSection(&(c->csec));

	for(i = 0; i < c->num; i++)
	{
		index = c->ofs + (c->num - 1UL) - i;
		if(index >= c->cap)
			index -= c->cap;

		dt = tick - c->dst_tick[index];
		
		if(dt >= 0)
		{
			if(!range_found)
			{
				for(ch = 0; ch < c->nch; ch++)
					pmag[ch] = c->data[ch][index].mag;
				range_found = 1;
			}

			dt = c->dst_tick[index] - tick_peak_from;

			if(dt < 0)
				break;

			for(ch = 0; ch < c->nch; ch++)
			{
				if(c->data[ch][index].peak > ppeak[ch])
					ppeak[ch] = c->data[ch][index].peak;
			}
		}
	}

	LeaveCriticalSection(&(c->csec));

	return range_found;
}

/* ---------------------------------------------------------------------------------------------- */
