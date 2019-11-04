/* ---------------------------------------------------------------------------------------------- */
/* audio output */

#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "rxconfig.h"
#include "levelmtr.h"
#include "../audio/sndmix.h"
#include "../audio/sndout.h"
#include "../audio/cvtbuf.h"
#include "../audio/rangelim.h"
#include "../dsp/dsp.h"
#include "../util/macro.h"
#include "../util/list.h"

/* ---------------------------------------------------------------------------------------------- */

/* rx audio output state */
typedef struct rx_audio_output {

	unsigned int fs_base;		/* input sampling frequency */
	size_t base_framelen;		/* input frame length */

	unsigned int output_fs;		/* output sampling frequency */
	unsigned int output_bps;	/* output bits per sample */

	/* mixing buffer (baseband sampling rate) */
	size_t mix_buf_len;			/* mixing buffer length, blocks */
	float *mix_buf;				/* mixing buffer data */

	/* output buffer (output sampling rate) */
	size_t out_buf_len;			/* output buffer length, blocks */
	float *out_buf;				/* output data buffer */
	float *out_lim_buf;			/* output data buffer (after limiter) */

	/* output raw data buffer */
	size_t raw_blocksize;		/* raw block size */
	size_t raw_buf_len;			/* raw buffer length, blocks */
	void *raw_buf;				/* raw buffer */

	/* mixer input buffer size */
	size_t mix_inbuf_len;
	size_t mix_inbuf_rhyst;

	CRITICAL_SECTION cs_proc;	/* processing lock */
	sndmix_state_t st_mix;		/* mixer state */
	resamp_state_t st_resamp;	/* resampler state */
	rangelim_params_t st_lim;	/* limiter state */
	sndout_state_t st_outdev;	/* output device */
	levelmtr_t st_levelmtr;		/* level meter state */

	struct rx_audio_output_sink *sink_head;
	struct rx_audio_output_sink *sink_tail;

} rx_audio_output_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct rx_audio_output_sink {

	struct rx_audio_output_sink *sink_next;
	struct rx_audio_output_sink *sink_prev;

	rx_audio_output_t *c;
	sndmix_input_t *in;

} rx_audio_output_sink_t;

/* ---------------------------------------------------------------------------------------------- */

/* open output device */
int rx_audio_output_open(rx_audio_output_t *c,
						 unsigned int fs_base, size_t base_framelen, rxconfig_t *params,
						 TCHAR *errbuf, size_t errbufsize);

/* close output device */
void rx_audio_output_close(rx_audio_output_t *c);

/* copy data from mixer buffers to output device input */
void rx_audio_output_processdata(rx_audio_output_t *c, size_t nsamp_total);

int rx_audio_output_read_level(rx_audio_output_t *c, double *mag, double *peak,
							   unsigned int tick_peak_from, unsigned int tick);

void rx_audio_output_set_output_gain(rx_audio_output_t *c, double gain);

/* ---------------------------------------------------------------------------------------------- */

/* create data sink */
rx_audio_output_sink_t *rx_audio_output_createsink(rx_audio_output_t *c);

/* delete data sink */
void rx_audio_output_deletesink(rx_audio_output_sink_t *sink);

/* write to data sink and calculate delay estimate */
size_t rx_audio_output_writesink(rx_audio_output_sink_t *sink,
								 float *data, size_t nsamp, sndmix_input_mode_t mode,
								 size_t *pnsamp_est_delay);

/* ---------------------------------------------------------------------------------------------- */
