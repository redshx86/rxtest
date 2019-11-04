/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <crtdbg.h>
#include <math.h>
#include <float.h>
#include "../dsp/dsp.h"
#include "../util/db.h"
#include "../util/macro.h"
#include "rxlimits.h"

/* ---------------------------------------------------------------------------------------------- */

typedef void (*rxspect_callback_t)(void *param, float *buf, size_t buf_size, size_t input_framelen);

/* ---------------------------------------------------------------------------------------------- */

typedef struct rxspect_buf {
	cpxf_t *data;
	size_t nsamp_in_used;
} rxspect_buf_t;

/* ---------------------------------------------------------------------------------------------- */

/* spectrum analyzer module state */
typedef struct rxspect_ctx {

	CRITICAL_SECTION csec_buf;		/* critical section for buffer indices */

	HANDLE h_thread;				/* processing thread handle */
	HANDLE h_evt_stop;				/* signalled to stop processing thread */
	HANDLE h_evt_bufdone;			/* signalled when having some buffers done */

	size_t input_framelen;			/* input frame length, samples */
	size_t input_ptr;				/* number of input samples accumulated */

	size_t output_framelen;			/* output frame length, samples */
	size_t decimf;					/* decimation factor, input_framelen / output_framelen */

	float *wndbuf;					/* window (kernel) buffer */

	size_t buf_count;				/* number of buffers used */
	size_t buf_doneptr;				/* offset to buffers done */
	size_t buf_donecnt;				/* number of buffers done */
	rxspect_buf_t *buf;

	size_t cur_buf_nsamp_in_used;	/* number of input samples used for current frame */

	fft_params_t fft_st;			/* fft state */

	float *output_magbuf;			/* output magnitude buffer */

	void *cb_param;					/* output callback param */
	rxspect_callback_t cb;			/* output callback function */

} rxspect_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize spectrum analyzer */
int rxspect_init(rxspect_ctx_t *ctx, unsigned int fs_in, unsigned int ups_req,
				 size_t output_framelen, size_t buf_count,
				 wnd_type_t wndtype, double wndarg, double magref,
				 rxspect_callback_t cb, void *cb_param,
				 TCHAR *errbuf, size_t errbufsize);

/* free spectrum analyzer */
void rxspect_cleanup(rxspect_ctx_t *ctx);

/* write data to spectrum analyzer */
size_t rxspect_writedata(rxspect_ctx_t *ctx, cpxf_t *data, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */
