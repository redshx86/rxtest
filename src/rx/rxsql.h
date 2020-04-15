/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>

#include "../dsp/dsp.h"
#include "../util/iniparse.h"
#include "../util/db.h"
#include "../util/delay.h"
#include "../util/macro.h"

/* ---------------------------------------------------------------------------------------------- */

/* processing buffer length, samples */
#define RXSQL_BUF_LEN					1024

/* ---------------------------------------------------------------------------------------------- */

/* possible sense bandwidth range */
#define RXSQL_BW_MIN					50.0		/* Hz */
#define RXSQL_BW_MAX					60000.0		/* Hz */

/* possible power envelope filter parameter range */
#define RXSQL_ENVEF_PARAM_MIN			0.001
#define RXSQL_ENVEF_PARAM_MAX			0.999

/* possible threshold range */
#define RXSQL_THRES_DB_MIN				-120.0		/* dB */
#define RXSQL_THRES_DB_MAX				60.0		/* dB */

/* possible delay range */
#define RXSQL_DLY_MS_MIN				0.01		/* ms */
#define RXSQL_DLY_MS_MAX				1000.0		/* ms */

/* ---------------------------------------------------------------------------------------------- */

/* squelch configuration */
typedef struct rxsql_cfg {

	int use_carr_filter;	/* filter carrier frequency */

	double bw;				/* sense bandwidth, Hz */

	double envef_param;		/* power envelope filter parameter */

	double op_thres_db;		/* open threshold, dB */
	double cl_thres_db;		/* close threshold, dB */

	double op_dly_ms;		/* open delay, ms */
	double cl_dly_ms;		/* close delay, ms */

} rxsql_cfg_t;

/* ---------------------------------------------------------------------------------------------- */

/* squelch delay buffer element */
typedef struct rxsql_dlyelem {
	cpxf_t sample;		/* delayed input data sample */
	float mag;			/* sense magnitude for data sample */
} rxsql_dlyelem_t;

/* ---------------------------------------------------------------------------------------------- */

/* squelch state */
typedef struct rxsql_state {

	rxsql_cfg_t *cfgp;		/* module configuration pointer */

	unsigned int fs_input;		/* input device sampling frequency, Hz */
	unsigned int fs_base;		/* baseband sampling frequency, Hz */

	size_t pwrbuf_len;			/* processing buffer length */
	cpxf_t *pwrbuf;				/* processing buffer */

	goertzw_state_t carrf;		/* carrier filter state */
	int carrf_is_inited;

	iirfilt_state1_t envef;		/* power envelope filter state */
	int envef_is_inited;

	double op_thres;			/* open threshold, linear magnitude */
	double cl_thres;			/* close threshold, linear magnitude */
	size_t op_dly;				/* open delay, samples */
	size_t cl_dly;				/* close delay, samples */
	size_t opn_cnt;				/* open delay counter */
	size_t cls_cnt;				/* close delay counter */
	int is_opn;					/* open flag */
	int thresholds_are_inited;

	delaybuf_t dlybuf;			/* delay buffer state */
	int dlybuf_is_inited;

	double sense;

} rxsql_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize squelch configuration */
void rxsql_cfg_set_defaults(rxsql_cfg_t *cfg);

/* load squelch configuration */
void rxsql_cfg_load(rxsql_cfg_t *cfg, ini_sect_t *sect);

/* save squelch configuration */
void rxsql_cfg_save(ini_sect_t *sect, const rxsql_cfg_t *cfg);

/* ---------------------------------------------------------------------------------------------- */

/* process data with squelch
 *	data : data to process, complex float,
 *	magbuf : output squelch sense magnitudes, linear,
 *	mutebuf : output buffer with mute flags. */
void rxsql_process(rxsql_state_t *s,
				   cpxf_t *data, float *magbuf, unsigned char *mutebuf, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */

/* (re-)configure squelch
 *	use_carr_filter : enable carrier filter,
 *	bw : new carrier filter sense bandwidth, Hz,
 *	envef_param : new power envelope filter parameter,
 *	op_thres : new open threshold, dB,
 *	cl_thres : close threshold, dB,
 *	op_dly : new open delay, ms,
 *	cl_dly : new close delay, ms,
 *	procname : processing channel name (for message formatting). */
int rxsql_configure(rxsql_state_t *s, int use_carr_filter, double bw, double envef_param,
					double op_thres, double cl_thres, double op_dly, double cl_dly,
					TCHAR *procname, TCHAR *errbuf, size_t errbufsize);

/* test squelch ready status */
int rxsql_check(rxsql_state_t *s);

/* initialize squelch state
 *	cfgp : filter configuration pointer,
 *	fs_input : input device sampling frequency (for psd scaling), Hz,
 *	fs_base : baseband sampling frequency,
 *	procname : processing channel name (for message formatting). */
int rxsql_init(rxsql_state_t *s, rxsql_cfg_t *cfgp,
			   unsigned int fs_input, unsigned int fs_base,
			   TCHAR *procname, TCHAR *errbuf, size_t errbufsize);

/* free squelch state */
void rxsql_cleanup(rxsql_state_t *s);

/* ---------------------------------------------------------------------------------------------- */
