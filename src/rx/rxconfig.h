/* ---------------------------------------------------------------------------------------------- */
/* Rx settings */

#pragma once

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>

#include "../util/iniparse.h"
#include "rxlimits.h"
#include "rx_fm.h"
#include "rxsql.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct rxconfig {

	/* input headers */
	size_t input_hdr_count;					/* number of input headers */
	double input_hdr_ms;					/* length of input header, ms */

	/* processing channel buffers */
	double proc_buf_len_ms;					/* input/ourput buffer length, ms */
	double proc_in_whyst_ms;				/* input write hysteresis, ms */
	double proc_out_rhyst_ms;				/* output read hysteresis, ms */
	double proc_workbuf_ms;					/* length of working buffer, ms */

	/* processing channel baseband sampling frequency */
	unsigned int base_fsmin;				/* minimum baseband frequency, Hz */
	unsigned int base_fsmax;				/* maximum baseband frequency, Hz */

	/* processing channel decimator section defaults */
	unsigned int proc_decim_frgran;			/* frequency response granularity, samples */
	unsigned int proc_decim_mcsdf;			/* minimum compensation stage downsampling factor */
	double proc_decim_dff;					/* fractional transition bandwidth */
	double proc_decim_as;					/* stopband attenuation */

	/* processing channel filter section defaults */
	double proc_filter_fc;					/* default cutoff frequency, Hz */
	double proc_filter_df;					/* default transition bandwidth, Hz */
	double proc_filter_as;					/* default stopband attenuation, dB */

	/* processing channel output section defaults */
	double proc_dcrem_alpha;				/* DC remover filter param */

	/* processing channel level meters */
	size_t proc_level_num_points;			/* level meter buffer length */
	double proc_level_update_int;			/* level meter update interval */

	/* demodulator and squelch defaults */
	rxproc_fm_config_t proc_fm_def;			/* FM demodulator defaults */
	rxsql_cfg_t proc_sql_def;				/* squelch defaults */

	/* audio output */
	double output_static_gain;				/* static gain, dB */
	unsigned int output_resamp_df;			/* resampler transition bandwidth, Hz */
	double output_resamp_as;				/* resampler alias attenuation, dB */
	double output_lim_thres;				/* limiter input threshold */
	double output_lim_range;				/* limiter input range */
	size_t output_hdr_cnt;					/* number of audio output WAVEHDRs */
	double output_hdr_ms;					/* length of WAVEHDR buffer, ms */
	int output_device_id;					/* device id */
	unsigned int output_fs;					/* sampling frequency, Hz */
	unsigned int output_bps;				/* bits per sample */
	size_t output_levelmtr_num_points;		/* level meter point count */
	double output_levelmtr_tick_interval;	/* level meter point interval */
	double output_gain;					/* output volume, dB */

	/* spectrum analyzer */
	unsigned int spect_ups_req;				/* required refresh rate */
	size_t spect_length;					/* output/fft length */
	size_t spect_bufcount;					/* input buffer count */
	int spect_wndtype;						/* window type */
	double spect_wndarg;					/* window argument */
	double spect_magref;					/* reference magnitude (0 dB) */

} rxconfig_t;

/* ---------------------------------------------------------------------------------------------- */

int rxconfig_init(rxconfig_t *p);
void rxconfig_cleanup(rxconfig_t *p);

void rxconfig_load(rxconfig_t *p, ini_data_t *ini);
void rxconfig_save(rxconfig_t *p, ini_data_t *ini);

/* ---------------------------------------------------------------------------------------------- */
