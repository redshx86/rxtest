/* ---------------------------------------------------------------------------------------------- */
/* Goertzel filter state */

#pragma once

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>

#include "complex.h"

/* ---------------------------------------------------------------------------------------------- */

/* Goertzel filter comb section state */
typedef struct goertz_comb_state {
	size_t n;				/* differential delay, samples */
	cpxf_t *buf;			/* delay buffer */
	size_t pos;				/* delay buffer pointer */
	float alpha;			/* comb factor */
} goertz_comb_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Goertzel filter resonator state */
typedef struct goertz_res_state {
	cpxf_t buf;				/* resonator delay buffer */
	cpxf_t tw;				/* resonator twiddle factor */
} goertz_res_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Goertzel filter state */
typedef struct goertz_state {
	goertz_comb_state_t comb;	/* comb filter section */
	goertz_res_state_t res;		/* complex resonator section */
} goertz_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize Goertzel filter
 *	n : differential delay, samples,
 *	k : harmonics number to tune to,
 *	r : damping factor to enforce stability. */
int goertz_init(goertz_state_t *s, size_t n, int k, double r);

/* Free Goertzel filter state */
void goertz_cleanup(goertz_state_t *s);

/* Execute Goertzel filter
 *	data : data to process, complex float,
 *	nsamp : number of samples to process. */
void goertz_exec(goertz_state_t *s, cpxf_t *data, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */

/* Windowed Goertzel filter state */
typedef struct goertzw_state {
	goertz_comb_state_t comb;	/* comb filter section */
	size_t rescount;			/* number of resonators */
	goertz_res_state_t *res;	/* complex resonator sections */
	float *wf;					/* window coefficients */
} goertzw_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* Initialize windowed Goertzel filter
 *	n : differential delay, samples,
 *	k : harmonic number to tune to,
 *	wn : number of window points (frequency domain, single direction),
 *	wf : window points (frequency domain, single direction),
 *	r : damping factor to enforce stability. */
int goertzw_init(goertzw_state_t *s, size_t n, int k, size_t wn, float *wf, double r);

/* Free windowed Goertzel filter state */
void goertzw_cleanup(goertzw_state_t *s);

/* Execute windowed Goertzel filter
 *	data : data to process, complex float,
 *	nsamp : number of samples to process. */
void goertzw_exec(goertzw_state_t *s, cpxf_t *data, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */
