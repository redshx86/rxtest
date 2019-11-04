/* ---------------------------------------------------------------------------------------------- */

#include "goertz.h"

/* ---------------------------------------------------------------------------------------------- */

static void goertz_res_init(goertz_res_state_t *s, size_t n, int k, double r)
{
	double w;
	cpxf_t tw;

	cpxf_zero(&(s->buf));

	w = 2.0 * M_PI * (double)k / (double)n;
	cpxf_expj(&tw, (float)w);
	cpxf_mulr(&(s->tw), &tw, (float)r);
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize Goertzel filter
 *	n : differential delay, samples,
 *	k : harmonics number to tune to,
 *	r : damping factor to enforce stability. */
int goertz_init(goertz_state_t *s, size_t n, int k, double r)
{
	s->comb.n = n;
	s->comb.buf = calloc(n, sizeof(cpxf_t));
	s->comb.pos = 0;
	s->comb.alpha = -(float)pow(r, n);

	goertz_res_init(&(s->res), n, k, r);

	return (s->comb.buf != NULL);
}

/* ---------------------------------------------------------------------------------------------- */

/* Free Goertzel filter state */
void goertz_cleanup(goertz_state_t *s)
{
	if(s != NULL)
	{
		free(s->comb.buf);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Execute Goertzel filter
 *	data : data to process, complex float,
 *	nsamp : number of samples to process. */
void goertz_exec(goertz_state_t *s, cpxf_t *data, size_t nsamp)
{
	cpxf_t *comb_bufp, temp, comb_out;

	while(nsamp--)
	{
		/* execute comb stage */
		comb_bufp = s->comb.buf + s->comb.pos;
		cpxf_mulr(&temp, comb_bufp, s->comb.alpha);
		cpxf_copy(comb_bufp, data);
		cpxf_add(&comb_out, data, &temp);

		/* shift comb delay buffer */
		s->comb.pos++;
		if(s->comb.pos == s->comb.n)
			s->comb.pos = 0;

		/* execute complex resonator */
		cpxf_add(&temp, &comb_out, &(s->res.buf));
		cpxf_mul(data, &temp, &(s->res.tw));
		cpxf_copy(&(s->res.buf), data);

		/* move to next data sample */
		data++;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Initialize windowed Goertzel filter
 *	n : differential delay, samples,
 *	k : harmonic number to tune to,
 *	wn : number of window points (frequency domain, single direction),
 *	wf : window points (frequency domain, single direction),
 *	r : damping factor to enforce stability. */
int goertzw_init(goertzw_state_t *s, size_t n, int k, size_t wn, float *wf, double r)
{
	size_t i, rc;

	s->comb.n = n;
	s->comb.buf = calloc(n, sizeof(cpxf_t));
	s->comb.pos = 0;
	s->comb.alpha = -(float)pow(r, n);

	s->rescount = (wn - 1) * 2 + 1;
	s->res = calloc(s->rescount, sizeof(goertz_res_state_t));
	s->wf = malloc(s->rescount * sizeof(float));

	if((s->comb.buf != NULL) && (s->res != NULL) && (s->wf != NULL))
	{
		rc = (wn - 1);

		goertz_res_init(s->res + rc, n, k, r);
		s->wf[rc] = wf[0];

		for(i = 1; i < wn; i++)
		{
			goertz_res_init(s->res + rc - i, n, k - (int)i, r);
			s->wf[rc - i] = wf[i];

			goertz_res_init(s->res + rc + i, n, k + (int)i, r);
			s->wf[rc + i] = wf[i];
		}

		return 1;
	}

	free(s->wf);
	free(s->res);
	free(s->comb.buf);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Free windowed Goertzel filter state */
void goertzw_cleanup(goertzw_state_t *s)
{
	if(s != NULL)
	{
		free(s->wf);
		free(s->res);
		free(s->comb.buf);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Execute windowed Goertzel filter
 *	data : data to process, complex float,
 *	nsamp : number of samples to process. */
void goertzw_exec(goertzw_state_t *s, cpxf_t *data, size_t nsamp)
{
	cpxf_t *comb_bufp, temp, comb_out, res_out;
	size_t i;

	while(nsamp--)
	{
		/* execute comb stage */
		comb_bufp = s->comb.buf + s->comb.pos;
		cpxf_mulr(&temp, comb_bufp, s->comb.alpha);
		cpxf_copy(comb_bufp, data);
		cpxf_add(&comb_out, data, &temp);

		/* shift comb delay buffer */
		s->comb.pos++;
		if(s->comb.pos == s->comb.n)
			s->comb.pos = 0;

		/* execute complex resonators */
		cpxf_zero(data);
		for(i = 0; i < s->rescount; i++)
		{
			cpxf_add(&temp, &comb_out, &(s->res[i].buf));
			cpxf_mul(&res_out, &temp, &(s->res[i].tw));
			cpxf_copy(&(s->res[i].buf), &res_out);
			cpxf_mulr(&temp, &res_out, s->wf[i]);
			cpxf_add(data, data, &temp);
		}

		/* move to next data sample */
		data++;
	}
}

/* ---------------------------------------------------------------------------------------------- */
