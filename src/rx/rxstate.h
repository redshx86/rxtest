/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <malloc.h>
#include <process.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "../input/indesc.h"
#include "../input/inputlst.h"
#include "rxaudout.h"
#include "rxprocch.h"
#include "rxspect.h"
#include "rxconfig.h"
#include "../util/macro.h"

/* ---------------------------------------------------------------------------------------------- */

#define RXSTATE_PROC_HASHSIZE		64

#define RXSTATE_AUDIO_METER_LEN		100

/* ---------------------------------------------------------------------------------------------- */

/* rx state */
/* (init) : variable valid after initialization
 * (run)  : variable valid when started only */
typedef struct rxstate {

	/* config */
	rxconfig_t config;						/* (init) */

	/* input module list */
	input_module_list_t input_mod_list;		/* (init) */

	/* selected input module */
	input_module_desc_t *input_mod_desc;	/* (init) */
	void *input_mod_ctx;					/* (init) */

	/* io thread */
	HANDLE evt_iothread_stop;				/* (init) io thread stop event */
	HANDLE hthread_io;						/* (run) io thread handle */

	/* opened input module data */
	HANDLE evt_input;						/* (init) opened input module event */
	double fc_input;						/* (run) input module center frequency */

	/* input headers state */
	size_t input_numbufs;					/* (run) number of input headers */
	size_t input_buflen;					/* (run) length of input header, samples */
	cpxf_t *input_bufdata;					/* (run) input headers data memory */
	input_buf_t *input_bufhdrs;				/* (run) input headers header memory*/

	/* sampling frequencies */
	unsigned int fs_input;					/* (run) input sampling frequency, Hz */
	unsigned int fs_base;					/* (run) baseband frequency, Hz */
	unsigned int decimf;					/* (run) downsampling factor = fs_input / fs_base */
	size_t input_nsamp_remain;				/* (run) remaining input samples for baseband */

	/* processing channels */
	CRITICAL_SECTION cs_proc;				/* (init) processing channel list update lock */
	rxproc_t *proc_first;					/* (init) processing channel list head */
	rxproc_t *proc_last;					/* (init) processing channel list tail */
	rxproc_t *proc_hash_heads[RXSTATE_PROC_HASHSIZE];
	rxproc_t *proc_hash_tails[RXSTATE_PROC_HASHSIZE];
	unsigned int proc_nextchid;				/* (init) next processing channel identifier */

	/* output device */
	rx_audio_output_t audioout;				/* (init) audio mixer/output state*/

	/* spectrum analyzer */
	CRITICAL_SECTION cs_spect;				/* (init) spectrum analyzer update lock */
	rxspect_callback_t spect_cb;			/* (init) spectrum analyzer callback */
	void *spect_cb_param;					/* (init) spectrum analyzer callback param */
	rxspect_ctx_t spect;					/* (run) spectrum analyzer */
	int spect_inited;						/* (run) spectrum analyzer inited*/

	/* processing channel activity callback */
	rxproc_activity_callback_t proc_act_cb;	/* (init) callback function */
	void *proc_act_ctx;						/* (init) callback context */

	/* flags */
	int is_started;							/* (init) started flag */

} rxstate_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize receiver and input module list, load config.
 *	cfg_init : init configuration,
 *	spect_cb : spectrum analyzer output callback function,
 *	spect_ctx : spectrum analyzer output callback context,
 *	proc_act_cb : processing channel activity status callback,
 *	proc_act_ctx : processing channel activity status context. */
rxstate_t *rx_init(const rxconfig_t *cfg_init,
				   rxspect_callback_t spect_cb, void *spect_cb_param,
				   rxproc_activity_callback_t proc_act_cb, void *proc_act_ctx,
				   TCHAR *errbuf, size_t errbufsize);

/* stop receiver and processing channels, unset input module,
 * cleanup receiver, channels, input module list. */
void rx_cleanup(rxstate_t *rx, ini_data_t *ini);

/* ---------------------------------------------------------------------------------------------- */

int rx_set_config(rxstate_t *rx, const rxconfig_t *cfg_new, TCHAR *errbuf, size_t errbufsize);

/* ---------------------------------------------------------------------------------------------- */

/* start receiver and all processing channels. */
int rx_start(rxstate_t *rx, TCHAR *errbuf, size_t errbufsize);

/* stop receiver and all processing channels. */
void rx_stop(rxstate_t *rx);

/* ---------------------------------------------------------------------------------------------- */

/* set input module and load configuration
 *	desc : module description context,
 *	ini : main ini file data (for loading input module config). */
int rx_input_mod_set(rxstate_t *rx, input_module_desc_t *desc, ini_data_t *ini,
					 TCHAR *errbuf, size_t errbufsize);

/* unset input module and save configuration
 *	ini : main ini file data (for storing input module config). */
void rx_input_mod_unset(rxstate_t *rx, ini_data_t *ini);

/* ---------------------------------------------------------------------------------------------- */

/* set input device center frequency
 *	fcp : input device center frequency, Hz (in/out). */
int rx_set_input_fc(rxstate_t *rx, double *fcp, TCHAR *errbuf, size_t errbufsize);

/* set spectrum analyzer configuration
 *	ups_req : wanted refresh rate, updates per second,
 *	length : output spectrum length, samples, power of two,
 *	bufcount : number of spectrum analyzer input buffers,
 *	wndtype : window type to use,
 *	wndarg : window parameter,
 *	magref : reference magnitude (0 dB), linear. */
//int rx_set_spect_params(rxstate_t *rx, unsigned int ups_req, size_t length, size_t bufcount,
//						int wndtype, double wndarg, double magref, TCHAR *errbuf, size_t errbufsize);

/* set output gain
 *	gain : new output gain, dB. */
int rx_set_output_gain(rxstate_t *rx, double gain, TCHAR *errbuf, size_t errbufsize);

/* ---------------------------------------------------------------------------------------------- */

/* create new processing channel
 *	cfg_init: init configuration. */
rxproc_t *rx_proc_create(rxstate_t *rx, rxprocconfig_t *cfg_init, TCHAR *errbuf, size_t errbufsize);

/* delete processing channel
 *	savesect : channel config ini section. */
void rx_proc_delete(rxproc_t *proc);

/* get number of processing channels */
int rx_proc_count(rxstate_t *rx);

/* move channel after another (reorder channel list). */
void rx_proc_putafter(rxproc_t *proc, rxproc_t *after);

/* move channel before another (reorder channel list). */
void rx_proc_putbefore(rxproc_t *proc, rxproc_t *before);

/* find processing channel by channel id */
rxproc_t *rx_proc_find(rxstate_t *rx, unsigned int chid);

/* ---------------------------------------------------------------------------------------------- */

/* load group of processing channels
 *	ini : channel ini file data,
 *	groupname : group of channels to load.
 * note : nothing returned in errbuf. */
void rx_proc_group_load(rxstate_t *rx, ini_data_t *ini, TCHAR *groupname,
						TCHAR *errbuf, size_t errbufsize);

/* save group of processing channels
 *	ini : channel ini file data,
 *	groupname : name of channel group.
 * note : nothing returned in errbuf. */
void rx_proc_group_save(rxstate_t *rx, ini_data_t *ini, TCHAR *groupname);

/* delete group of processing channels
 *	groupname : name of group to delete. */
void rx_proc_group_delete(ini_data_t *ini, TCHAR *groupname);

/* ---------------------------------------------------------------------------------------------- */
