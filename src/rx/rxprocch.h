/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <stdlib.h>
#include <process.h>
#include <crtdbg.h>
#include "rxconfig.h"
#include "rxprccfg.h"
#include "rxaudout.h"
#include "../dsp/dsp.h"
#include "../audio/sndmix.h"
#include "../util/buf.h"
#include "../util/macro.h"
#include "rx_fm.h"
#include "rxsql.h"
#include "levelmtr.h"

/* ---------------------------------------------------------------------------------------------- */

/* output activity status pulse extension */
#define RXPROC_OUT_ACTSTATE_EXT_MS		50.0	/* ms */

/* ---------------------------------------------------------------------------------------------- */

struct rxproc;

typedef void (*rxproc_activity_callback_t)(void *ctx, struct rxproc *proc, int active);

/* ---------------------------------------------------------------------------------------------- */

/* rx processing channel */

/* (init) : variable valid after initialization
 * (run)  : variable valid when started only */
typedef struct rxproc {

	/* rx pointer */
	struct rxstate *rx;					/* (init) parent rx state pointer */

	/* channel id */
	unsigned int chid;					/* (external) */

	/* channel list pointers */
	struct rxproc *proc_next;			/* (external) */
	struct rxproc *proc_prev;			/* (external) */
	struct rxproc *proc_hash_next;		/* (external) */
	struct rxproc *proc_hash_prev;		/* (external) */

	/* channel configuration */
	rxprocconfig_t cfg;

	/* processing lock */
	CRITICAL_SECTION cs_proc;			/* (init) critical section to avoid
										 * configuration change when processing data */

	/* sampling rates */
	unsigned int fs_input;				/* (run) input sampling frequency, Hz */
	unsigned int fs_base;				/* (run) baseband sampling frequency, Hz */
	unsigned int decimf;				/* (run) downsampling factor = fs_input / fs_base */

	/* input data buffer */
	HANDLE evt_input;					/* (init) event indicates new data to process */
	buf_state_t buf_input;				/* (run) input data buffer */

	/* working buffer */
	size_t work_buf_len;				/* (run) working buffer length, samples */
	cpxf_t *work_buf;					/* (run) working buffer data */
	float *sql_sense_buf;				/* (run) squelch sense data */
	unsigned char *sql_mute_buf;		/* (run) squelch mute data */

	/* demodulated audio data buffer */
	size_t audio_buf_len;				/* (run) audio output buffer length, samples */
	float *audio_buf;					/* (run) audio output buffer data */

	/* oscillator section */
	double fc_input;					/* (run) input stream center frequency, Hz */
	osc_state_t osc;					/* (run) oscillator state */

	/* decimator section state */
	unsigned int decim_comb_decimf;		/* (run) downsampling factor, comb stage */
	unsigned int decim_compf_decimf;	/* (run) downsampling factor, compensator stage */
	decim_state_t decim;				/* (run) decimator state */
	int decim_inited;					/* (run) decimator inited flag */

	/* filter stage */
	firfilt_state_t filter;				/* (run) baseband filter state */
	int filter_inited;					/* (run) baseband filter inited flag */
	int filter_tuned;					/* (run) baseband filter tuned flag */

	/* demodulator state */
	union {
		rxproc_fm_t fm;					/* (run) FM demodulator state */
	} demod;
	int demod_inited;					/* (run) current demodulator inited flag */

	/* squelch state */
	rxsql_state_t sql;					/* (run) squelch state */
	levelmtr_t sql_sense_meter;			/* (run) sqluelch sense level meter */
	int sql_inited;						/* (run) squelch iinited flag */

	/* output section */
	iirfilt_state1_t dcrem_state;		/* (run) DC remover filter state */

	/* audio output */
	rx_audio_output_sink_t *audio_sink;	/* (run) audio data sink */

	/* processing thread stuff */
	HANDLE evt_proc_stop;				/* (init) processing thread stop event */
	HANDLE hthread_proc;				/* (run) processing thread handle */

	/* baseband level meter */
	levelmtr_t levelmtr;				/* (run) baseband level meter state */
	int levelmtr_inited;

	/* activity status */
	int out_act_stat;					/* (run) output activity status */
	size_t out_act_extctr;				/* (run) pulse extender counter, samples */
	size_t out_act_extcnt;				/* (run) pulse extender value, samples */
	rxproc_activity_callback_t act_cb;	/* (init) activity status callback function */
	void *act_ctx;						/* (init) activity status callback context */

	/* flags */
	int is_started;						/* (init) started flag */

} rxproc_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize new processing channel
 *	name : channel name,
 *	fc : channel center frequency, Hz,
 *	config : global configuration for defaults,
 *	loadsect : ini section to load configuration,
 *	act_cb : activity status callback function,
 *	act_ctx : activity status callback context. */
rxproc_t *rxproc_init(struct rxstate *rx, TCHAR *name, double fc,
					  rxconfig_t *config, ini_sect_t *loadsect,
					  rxproc_activity_callback_t act_cb, void *act_ctx,
					  TCHAR *errbuf, size_t errbufsize);

/* free processing channel
 *	savesect : ini section to save configuration to. */
void rxproc_clenaup(rxproc_t *proc, ini_sect_t *savesect);

/* ---------------------------------------------------------------------------------------------- */

/* start processing channel
 *	fs_input : input sampling frequency, Hz,
 *	fs_base : baseband sampling frequency, Hz,
 *	audio_sink : audio mixer input,
 *	fc_input : input center frequency, Hz,
 *	config : global configuration (for buffer lengths). */
int rxproc_start(rxproc_t *proc, TCHAR *errbuf, size_t errbufsize,
				 unsigned int fs_input, unsigned int fs_base,
				 rx_audio_output_sink_t *audio_sink, double fc_input,
				 rxconfig_t *config);

/* stop processing channel */
void rxproc_stop(rxproc_t *proc);

/* ---------------------------------------------------------------------------------------------- */

/* send data to processing channel
 *	data : data from input device at input sampling frequency, complex float,
 *	nsamp : number of input samples. */
size_t rxproc_write_data(rxproc_t *proc, cpxf_t *data, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */

/* change channel name */
int rxproc_set_name(rxproc_t *proc, TCHAR *name);

/* set channel center frequency */
/*     fc: channel center frequency, Hz */
int rxproc_set_fc(rxproc_t *proc, double fc);

/* set input center frequency */
/*     fc_input: input center frequency, Hz */
int rxproc_set_fc_input(rxproc_t *proc, double fc_input);

/* set demodulator type */
/*     demod_type: new demodulator type */
int rxproc_set_demod_type(rxproc_t *proc, rxproc_demodtype_t demod_type,
						  TCHAR *errbuf, size_t errbufsize);

/* set decimator parameters */
/*     frgran: frequency response granularity, samples
 *     mcsdf: minimum compensation stage decimation factor
 *     dff: transition bandwidth, fractional
 *     as: stopband attenuation, dB */
int rxproc_set_decim_params(rxproc_t *proc, unsigned int frgran, unsigned int mcsdf,
							double dff, double as, TCHAR *errbuf, size_t errbufsize);

/* set baseband filter parameters */
/*     fc : cutoff frequency, Hz
 *     df : transition bandwidth, Hz,
 *     as : stopband attenuation, dB */
int rxproc_set_filter_params(rxproc_t *proc, double fc, double df, double as,
							 TCHAR *errbuf, size_t errbufsize);

/* set FM demodulator parameters
 *	df : frequency deviation, Hz. */
int rxproc_set_fm_demod_params(rxproc_t *proc, double df, TCHAR *errbuf, size_t errbufsize);

/* set squelch parameters */
/*	use_carr_filter : use carrier filter,
 *	bw : carrier filter sense bandwidth, Hz,
 *	op_thres : open threshold, dB,
 *	cl_thres : close threshold, dB,
 *	op_dly : open delay, ms,
 *	cl_dly : close delay, ms. */
int rxproc_set_sql_params(rxproc_t *proc, int use_carr_filter,
							  double bw, double envef_param,
							  double op_thres, double cl_thres,
							  double op_dly, double cl_dly,
							  TCHAR *errbuf, size_t errbufsize);

/* set DC offset remover filter alpha coefficient */
int rxproc_set_output_dcrem_alpha(rxproc_t *proc, double output_dcrem_alpha);

/* set audio output gain */
int rxproc_set_output_gain(rxproc_t *proc, double gain);

/* read level meter */
int rxproc_read_level(rxproc_t *proc, unsigned int tick, double *plevel);

/* read squelch sense meter */
int rxproc_read_sql_sense(rxproc_t *proc, unsigned int tick, double *psense);

/* ---------------------------------------------------------------------------------------------- */

typedef enum rxproc_status {

	RXPROC_STOPPED,
	RXPROC_ERROR,
	RXPROC_RANGEUP,
	RXPROC_RANGEDOWN,
	RXPROC_SQLCLOSED,
	RXPROC_SQLOPENED,

	RXPROC_STATUS_COUNT

} rxproc_status_t;

rxproc_status_t rxproc_status_query(rxproc_t *proc);

/* ---------------------------------------------------------------------------------------------- */
