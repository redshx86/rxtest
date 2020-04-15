/* ---------------------------------------------------------------------------------------------- */

#include "rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

/* start processing channel */
static int rx_proc_start(rxproc_t *proc, TCHAR *errbuf, size_t errbufsize)
{
	rxstate_t *rx = proc->rx;
	rx_audio_output_sink_t *sink;

	_tcscpy(errbuf, _T(""));

	/* check channel is not started */
	if(proc->is_started) {
		_sntprintf(errbuf, errbufsize,
			_T("Channel '%s' is already started."), proc->cfg.name);
	} else {

		/* create audio output for channel */
		if( (sink = rx_audio_output_createsink(&(rx->audioout))) == NULL ) {
			_sntprintf(errbuf, errbufsize,
				_T("Can't create audio sink for channel '%s'."), proc->cfg.name);
		} else {

			/* start channel */
			if(rxproc_start(proc, errbuf, errbufsize, rx->fs_input, rx->fs_base,
				sink, rx->fc_input, &(rx->config)))
			{
				return 1;
			}

			/* destroy audio output */
			rx_audio_output_deletesink(sink);
		}
	}

	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize, _T("Can't start channel '%s'."), proc->cfg.name);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* stop processing channel */
static void rx_proc_stop(rxproc_t *proc)
{
	rx_audio_output_sink_t *sink = proc->audio_sink;

	/* check channel not stopped */
	if(proc->is_started)
	{
		/* stop channel */
		rxproc_stop(proc);

		/* destroy audio output */
		rx_audio_output_deletesink(sink);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* register processing channel */
static void rx_proc_reg(rxstate_t *rx, rxproc_t *proc)
{
	unsigned int index;

	EnterCriticalSection(&(rx->cs_proc));

	/* update channel list */
	DLIST_INSBACK(proc, rx->proc_first, rx->proc_last, proc_next, proc_prev);

	/* update channel hash map */
	index = proc->chid % RXSTATE_PROC_HASHSIZE;
	DLIST_INSBACK(proc, rx->proc_hash_heads[index], rx->proc_hash_tails[index],
		proc_hash_next, proc_hash_prev);

	LeaveCriticalSection(&(rx->cs_proc));
}

/* ---------------------------------------------------------------------------------------------- */

/* unregister channel */
static void rx_proc_unreg(rxstate_t *rx, rxproc_t *proc)
{
	unsigned int index;

	EnterCriticalSection(&(rx->cs_proc));

	/* update channel list */
	DLIST_REMOVE(proc, rx->proc_first, rx->proc_last, proc_next, proc_prev);

	/* update channel hash map */
	index = proc->chid % RXSTATE_PROC_HASHSIZE;
	DLIST_REMOVE(proc, rx->proc_hash_heads[index], rx->proc_hash_tails[index],
		proc_hash_next, proc_hash_prev);

	LeaveCriticalSection(&(rx->cs_proc));
}

/* ---------------------------------------------------------------------------------------------- */

/* create new processing channel
 *	cfg_init: init configuration. */
rxproc_t *rx_proc_create(rxstate_t *rx, rxprocconfig_t *cfg_init, TCHAR *errbuf, size_t errbufsize)
{
	rxproc_t *proc;

	_tcscpy(errbuf, _T(""));

	/* initialize channel */
	if( (proc = rxproc_init(rx, cfg_init,
			rx->proc_act_cb, rx->proc_act_ctx, errbuf, errbufsize)) != NULL ) 
	{
		/* assign channel id */
		proc->chid = rx->proc_nextchid++;

		/* register channel */
		rx_proc_reg(rx, proc);

		/* if rx stopped, do not start channel */
		if(!rx->is_started)
			return proc;

		/* start channel */
		if(rx_proc_start(proc, errbuf, errbufsize))
			return proc;

		/* unregister channel */
		rx_proc_unreg(rx, proc);

		rxproc_clenaup(proc);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* delete processing channel */
void rx_proc_delete(rxproc_t *proc)
{
	rxstate_t *rx = proc->rx;

	/* stop channel */
	rx_proc_stop(proc);

	/* unregister channel */
	rx_proc_unreg(rx, proc);

	/* destroy channel */
	rxproc_clenaup(proc);
}

/* ---------------------------------------------------------------------------------------------- */

/* get number of processing channels */
int rx_proc_count(rxstate_t *rx)
{
	int count;
	rxproc_t *proc;

	count = 0;
	for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next)
		count++;
	return count;
}

/* ---------------------------------------------------------------------------------------------- */

/* move channel after another (reorder channel list). */
void rx_proc_putafter(rxproc_t *proc, rxproc_t *after)
{
	rxstate_t *rx = proc->rx;

	if(proc != after)
	{
		EnterCriticalSection(&(rx->cs_proc));
		DLIST_REMOVE(proc, rx->proc_first, rx->proc_last, proc_next, proc_prev);
		if(after == NULL) {
			DLIST_INSBACK(proc, rx->proc_first, rx->proc_last, proc_next, proc_prev);
		} else {
			DLIST_INSAFTER(after, proc, rx->proc_last, proc_next, proc_prev);
		}
		LeaveCriticalSection(&(rx->cs_proc));
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move channel before another (reorder channel list). */
void rx_proc_putbefore(rxproc_t *proc, rxproc_t *before)
{
	rxstate_t *rx = proc->rx;

	if(proc != before)
	{
		EnterCriticalSection(&(rx->cs_proc));
		DLIST_REMOVE(proc, rx->proc_first, rx->proc_last, proc_next, proc_prev);
		if(before == NULL) {
			DLIST_INSFRONT(proc, rx->proc_first, rx->proc_last, proc_next, proc_prev);
		} else {
			DLIST_INSBEFORE(before, proc, rx->proc_first, proc_next, proc_prev);
		}
		LeaveCriticalSection(&(rx->cs_proc));
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* find processing channel by channel id */
rxproc_t *rx_proc_find(rxstate_t *rx, unsigned int chid)
{
	unsigned int index;
	rxproc_t *proc;

	index = chid % RXSTATE_PROC_HASHSIZE;
	for(proc = rx->proc_hash_heads[index]; proc != NULL; proc = proc->proc_hash_next)
	{
		if(proc->chid == chid)
			return proc;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* set input device center frequency
 *	fcp : input device center frequency, Hz (in/out). */
int rx_set_input_fc(rxstate_t *rx, double *fcp, TCHAR *errbuf, size_t errbufsize)
{
	double fc_set;
	rxproc_t *proc;

	fc_set = *fcp;

	_tcscpy(errbuf, _T(""));

	if(!rx->is_started) {
		_sntprintf(errbuf, errbufsize,
			_T("Can't set input center frequency. Stream is not started."));
		return 0;
	}

	if(rx->input_mod_desc->fn_set_fc == NULL) {
		_sntprintf(errbuf, errbufsize,
			_T("Setting input center frequency isn't supported by input device."));
		return 0;
	}

	if(fabs(fc_set - rx->fc_input) < 1e-3)
		return -1;

	EnterCriticalSection(&(rx->cs_proc));

	if(!rx->input_mod_desc->fn_set_fc(rx->input_mod_ctx, &fc_set)) {
		_sntprintf(errbuf, errbufsize,
			_T("Can't set input device center frequency."));
		LeaveCriticalSection(&(rx->cs_proc));
		return 0;
	}

	if(fabs(fc_set - rx->fc_input) < 1e-3) {
		LeaveCriticalSection(&(rx->cs_proc));
		return -1;
	}

	rx->fc_input = fc_set;

	for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next)
		rxproc_set_fc_input(proc, rx->fc_input);

	LeaveCriticalSection(&(rx->cs_proc));

	*fcp = fc_set;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize spectrum analyzer */
static int rx_spect_init(rxstate_t *rx, TCHAR *errbuf, size_t errbufsize)
{
	_tcscpy(errbuf, _T(""));

	if( (!rx->spect_inited) && (rx->spect_cb != NULL) )
	{
		if( !rxspect_init(&(rx->spect), rx->fs_input, rx->config.spect_ups_req,
			rx->config.spect_length, rx->config.spect_bufcount,
			rx->config.spect_wndtype, rx->config.spect_wndarg,
			rx->config.spect_magref,
			rx->spect_cb, rx->spect_cb_param, errbuf, errbufsize) )
		{
			return 0;
		}

		rx->spect_inited = 1;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* deinitialize spectrum analyzer */
static void rx_spect_deinit(rxstate_t *rx)
{
	if(rx->spect_inited)
	{
		rxspect_cleanup(&(rx->spect));
		rx->spect_inited = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* set spectrum analyzer configuration
 *	ups_req : wanted refresh rate, updates per second,
 *	length : output spectrum length, samples, power of two,
 *	bufcount : number of spectrum analyzer input buffers,
 *	wndtype : window type to use,
 *	wndarg : window parameter,
 *	magref : reference magnitude (0 dB), linear. */
int rx_set_spect_params(rxstate_t *rx, unsigned int ups_req, size_t length, size_t bufcount,
						int wndtype, double wndarg, double magref, TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	_tcscpy(errbuf, _T(""));

	/* check parameters */
	if( (!INRANGE_MINMAX(ups_req, RXLIM_SPECT_UPS_REQ)) ||
		(!INRANGE_MINMAX(length, RXLIM_SPECT_LENGTH)) ||
		(!ISPOWEROFTWO(length)) ||
		(!INRANGE_MINMAX(bufcount, RXLIM_SPECT_BUF_COUNT)) ||
		(wndtype < 0) || (wndtype >= WND_COUNT) )
	{
		_sntprintf(errbuf, errbufsize, _T("Bad spectrum analyzer configuration."));
		return 0;
	}

	/* if receiver stopped, update config and exit */
	if(!rx->is_started)
	{
		rx->config.spect_ups_req = ups_req;
		rx->config.spect_length = length;
		rx->config.spect_bufcount = bufcount;
		rx->config.spect_wndtype = wndtype;
		rx->config.spect_wndarg = wndarg;
		rx->config.spect_magref = magref;
		return 1;
	}

	/* check parameters changed */
	if( (ups_req != rx->config.spect_ups_req) ||
		(length != rx->config.spect_length) ||
		(bufcount != rx->config.spect_bufcount) ||
		(wndtype != rx->config.spect_wndtype) ||
		(fabs(wndarg - rx->config.spect_wndarg) >= 1e-6) ||
		(fabs(magref - rx->config.spect_magref) >= 1e-6) )
	{
		EnterCriticalSection(&(rx->cs_spect));

		/* deinit spectrum analyzer */
		rx_spect_deinit(rx);

		/* set new parameters */
		rx->config.spect_ups_req = ups_req;
		rx->config.spect_length = length;
		rx->config.spect_bufcount = bufcount;
		rx->config.spect_wndtype = wndtype;
		rx->config.spect_wndarg = wndarg;
		rx->config.spect_magref = magref;

		/* reinit spectrum analyzer */
		status = rx_spect_init(rx, errbuf, errbufsize);

		LeaveCriticalSection(&(rx->cs_spect));
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* set output gain
 *	gain : new output gain, dB. */
int rx_set_output_gain(rxstate_t *rx, double gain, TCHAR *errbuf, size_t errbufsize)
{
	if(!INRANGE_MINMAX(gain, RXLIM_AUDIO_GAIN))
	{
		_sntprintf(errbuf, errbufsize, _T("Bad output gain."));
		return 0;
	}

	if(fabs(rx->config.output_gain - gain) >= 1e-6)
	{
		rx->config.output_gain = gain;

		if(rx->is_started) {
			rx_audio_output_set_output_gain(&(rx->audioout), gain);
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle input data */
static void rx_input_process(rxstate_t *rx)
{
	size_t temp, base_nsamp_process;
	input_buf_t *buf;
	rxproc_t *proc;

	/* process captured buffers */
	while((buf = rx->input_mod_desc->fn_remove_buf(rx->input_mod_ctx)) != NULL)
	{
		/* feed data to processing channels */
		EnterCriticalSection(&(rx->cs_proc));
		for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next)
			rxproc_write_data(proc, buf->data, buf->nsamples);
		LeaveCriticalSection(&(rx->cs_proc));

		/* feed data to spectrum analyzer */
		if(rx->spect_inited) {
			EnterCriticalSection(&(rx->cs_spect));
			rxspect_writedata(&(rx->spect), buf->data, buf->nsamples);
			LeaveCriticalSection(&(rx->cs_spect));
		}

		/* calculate syncronized number of baseband samples. */
		temp = buf->nsamples + rx->input_nsamp_remain;
		base_nsamp_process = temp / rx->decimf;
		rx->input_nsamp_remain = temp - base_nsamp_process * rx->decimf;

		/* send output data from channels to audio device */
		rx_audio_output_processdata(&(rx->audioout), base_nsamp_process);

		/* send buffer back to input queue */
		rx->input_mod_desc->fn_add_bufs(rx->input_mod_ctx, buf, 1);
	}

	/* handle error */
	if(rx->input_mod_desc->fn_check_err(rx->input_mod_ctx)) {
		/* todo */
	}
}

/* ---------------------------------------------------------------------------------------------- */

enum {
	RXIO_EVT_STOP,
	RXIO_EVT_INPUT,

	RXIO_EVT_COUNT
};

static unsigned int __stdcall rx_io_threadproc(rxstate_t *rx)
{
	DWORD res;
	HANDLE events[RXIO_EVT_COUNT];

	events[RXIO_EVT_STOP] = rx->evt_iothread_stop;
	events[RXIO_EVT_INPUT] = rx->evt_input;

	for( ; ; )
	{
		res = WaitForMultipleObjects(RXIO_EVT_COUNT, events, FALSE, INFINITE);

		if(res == RXIO_EVT_STOP)
			break;

		if(res == RXIO_EVT_INPUT)
			rx_input_process(rx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* calculate decimation factor */
static unsigned int rx_calc_decimf(
	unsigned int fs_in, unsigned int fs_out_min, unsigned int fs_out_max)
{
	unsigned int decimf;

	decimf = fs_in / fs_out_min;
	if(decimf == 0)
		return 0;

	while(fs_in % decimf != 0)
		decimf--;

	if(fs_in / decimf > fs_out_max)
		return 0;

	return decimf;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize input stream headers */
static int rx_init_input_buffers(rxstate_t *rx)
{
	size_t i;

	/* calculate header length */
	rx->input_buflen = CEIL2POT(MSECS2NSAMP(rx->config.input_hdr_ms, rx->fs_input), 256UL);
	rx->input_numbufs = rx->config.input_hdr_count;

	/* allocate headers */
	rx->input_bufhdrs = calloc(rx->input_numbufs, sizeof(input_buf_t));
	rx->input_bufdata = malloc(rx->input_numbufs * rx->input_buflen * sizeof(cpxf_t));

	if((rx->input_bufhdrs != NULL) && (rx->input_bufdata != NULL))
	{
		/* initialize headers */
		for(i = 0; i < rx->input_numbufs; i++) {
			rx->input_bufhdrs[i].capacity = rx->input_buflen;
			rx->input_bufhdrs[i].data = rx->input_bufdata + i * rx->input_buflen;
		}

		return 1;
	}

	free(rx->input_bufdata);
	free(rx->input_bufhdrs);
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* start receiver and all processing channels. */
int rx_start(rxstate_t *rx, TCHAR *errbuf, size_t errbufsize)
{
	unsigned int tid;
	size_t base_framelen;
	rxproc_t *proc;

	_tcscpy(errbuf, _T(""));

	/* check rx is not started */
	if(rx->is_started)
	{
		_sntprintf(errbuf, errbufsize,
			_T("Receiver is already started."));
	}

	/* check input module is selected */
	else if(rx->input_mod_ctx == NULL)
	{
		_sntprintf(errbuf, errbufsize,
			_T("Input device is not selected."));
	}
	else
	{
		/* open input module */
		if(rx->input_mod_desc->fn_open(
			rx->input_mod_ctx, rx->evt_input, errbuf, errbufsize) )
		{
			/* read sampling and input center frequencies from input module */
			rx->fs_input = rx->input_mod_desc->fn_get_fs(rx->input_mod_ctx);
			if(!rx->input_mod_desc->fn_get_fc(rx->input_mod_ctx, &(rx->fc_input)))
			{
				_sntprintf(errbuf, errbufsize,
					_T("Can't read input center frequency from input device."));
			}
			else
			{
				/* initialize input buffers */
				if(rx_init_input_buffers(rx))
				{
					/* calculate decimation factor */
					rx->decimf = rx_calc_decimf(rx->fs_input,
						rx->config.base_fsmin, rx->config.base_fsmax);

					if(rx->decimf == 0)
					{
						_sntprintf(errbuf, errbufsize,
							_T("Input sampling frequency %u Hz is not divisible by ")
							_T("anything in specified baseband range of %u to %u Hz. ")
							_T("Can't select downsampling factor."),
							rx->fs_input, rx->config.base_fsmin, rx->config.base_fsmax);
					}
					else
					{
						/* calculate baseband frequency and frame length */
						rx->fs_base = rx->fs_input / rx->decimf;
						rx->input_nsamp_remain = 0;
						base_framelen = (rx->input_buflen + rx->decimf - 1) / rx->decimf;

						/* open audio output device */
						if(rx_audio_output_open(&(rx->audioout), rx->fs_base, base_framelen,
							&(rx->config), errbuf, errbufsize))
						{
							/* initialize spectrum analyzer */
							rx->spect_inited = 0;

							if(rx_spect_init(rx, errbuf, errbufsize))
							{
								/* start processing channels */
								for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next) {
									if(!rx_proc_start(proc, errbuf, errbufsize))
										break;
								}

								/* all processing channels started? */
								if(proc == NULL)
								{
									/* add buffers to input module queue */
									rx->input_mod_desc->fn_add_bufs(rx->input_mod_ctx,
										rx->input_bufhdrs, rx->input_numbufs);

									/* start input module */
									if(rx->input_mod_desc->fn_start(rx->input_mod_ctx,
										errbuf, errbufsize))
									{
										/* start io thread */
										rx->hthread_io = (HANDLE)_beginthreadex(
											NULL, 0, rx_io_threadproc, rx, 0, &tid);

										if(rx->hthread_io != NULL)
										{
											SetThreadPriority(rx->hthread_io,
												THREAD_PRIORITY_ABOVE_NORMAL);

											rx->is_started = 1;

											return 1;
										}

										/* stop input module */
										rx->input_mod_desc->fn_stop(rx->input_mod_ctx);
									}
								}

								/* stop processing channels */
								for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next)
									rx_proc_stop(proc);

								/* deinit spectrum analyzer */
								rx_spect_deinit(rx);
							}
							/* close audio output device */
							rx_audio_output_close(&(rx->audioout));
						}
					}
					/* free input buffers */
					free(rx->input_bufdata);
					free(rx->input_bufhdrs);
				}
			}
			/* close input module */
			rx->input_mod_desc->fn_close(rx->input_mod_ctx);
		}
	}

	/* default error message */
	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize, _T("Can't start receiver (out of memory?)"));
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* stop receiver and all processing channels. */
void rx_stop(rxstate_t *rx)
{
	rxproc_t *proc;

	/* check not stopped */
	if(rx->is_started)
	{
		/* mark as stopped */
		rx->is_started = 0;

		/* stop io thread */
		SetEvent(rx->evt_iothread_stop);
		WaitForSingleObject(rx->hthread_io, INFINITE);
		CloseHandle(rx->hthread_io);

		/* stop input module */
		rx->input_mod_desc->fn_stop(rx->input_mod_ctx);

		/* stop channels */
		for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next)
			rx_proc_stop(proc);

		/* deinit spectrum analyzer */
		rx_spect_deinit(rx);

		/* close audio output device */
		rx_audio_output_close(&(rx->audioout));

		/* free input buffers */
		free(rx->input_bufdata);
		free(rx->input_bufhdrs);

		/* close input module */
		rx->input_mod_desc->fn_close(rx->input_mod_ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* set input module and load configuration
 *	desc : module description context,
 *	ini : main ini file data (for loading input module config). */
int rx_input_mod_set(rxstate_t *rx, input_module_desc_t *desc, ini_data_t *ini,
					 TCHAR *errbuf, size_t errbufsize)
{
	void *mod_ctx;

	_tcscpy(errbuf, _T(""));

	/* check initialized and not started */
	if( (rx != NULL) && (!rx->is_started) )
	{
		/* unset previous module */
		rx_input_mod_unset(rx, ini);

		if( (mod_ctx = desc->fn_init(desc, errbuf, errbufsize)) != NULL )
		{
			/* load module config */
			desc->fn_config_load(mod_ctx, ini);

			/* set input module */
			rx->input_mod_desc = desc;
			rx->input_mod_ctx = mod_ctx;

			return 1;
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* unset input module and save configuration
 *	ini : main ini file data (for storing input module config). */
void rx_input_mod_unset(rxstate_t *rx, ini_data_t *ini)
{
	/* check initialized and not started */
	if( (rx != NULL) && (!rx->is_started) && (rx->input_mod_ctx != NULL) )
	{
		rx->input_mod_desc->fn_config_save(rx->input_mod_ctx, ini);
		rx->input_mod_desc->fn_cleanup(rx->input_mod_ctx);

		rx->input_mod_desc = NULL;
		rx->input_mod_ctx = NULL;
	}
}

/* ---------------------------------------------------------------------------------------------- */

int rx_set_config(rxstate_t *rx, const rxconfig_t *cfg_new, TCHAR *errbuf, size_t errbufsize)
{
	int success = 1;

	/* input headers */
	rx->config.input_hdr_count					= cfg_new->input_hdr_count;
	rx->config.input_hdr_ms						= cfg_new->input_hdr_ms;

	/* processing channel buffers */
	rx->config.proc_buf_len_ms					= cfg_new->proc_buf_len_ms;
	rx->config.proc_in_whyst_ms					= cfg_new->proc_in_whyst_ms;
	rx->config.proc_out_rhyst_ms				= cfg_new->proc_out_rhyst_ms;
	rx->config.proc_workbuf_ms					= cfg_new->proc_workbuf_ms;
	
	/* processing channel baseband sampling frequency */
	rx->config.base_fsmin						= cfg_new->base_fsmin;
	rx->config.base_fsmax						= cfg_new->base_fsmax;

	/* audio output */
	rx->config.output_static_gain				= cfg_new->output_static_gain;
	rx->config.output_resamp_df					= cfg_new->output_resamp_df;
	rx->config.output_resamp_as					= cfg_new->output_resamp_as;
	rx->config.output_lim_thres					= cfg_new->output_lim_thres;
	rx->config.output_lim_range					= cfg_new->output_lim_range;
	rx->config.output_device_id					= cfg_new->output_device_id;
	rx->config.output_fs						= cfg_new->output_fs;
	rx->config.output_bps						= cfg_new->output_bps;
	rx->config.output_hdr_cnt					= cfg_new->output_hdr_cnt;
	rx->config.output_hdr_ms					= cfg_new->output_hdr_ms;

	/* spectrum analyzer */
	if( (cfg_new->spect_ups_req != rx->config.spect_ups_req) ||
		(cfg_new->spect_length != rx->config.spect_length) ||
		(cfg_new->spect_bufcount != rx->config.spect_bufcount) ||
		(cfg_new->spect_wndtype != rx->config.spect_wndtype) ||
		(fabs(cfg_new->spect_wndarg - rx->config.spect_wndarg) >= 1e-6) ||
		(fabs(cfg_new->spect_magref - rx->config.spect_magref) >= 1e-6) )
	{
		if(!rx_set_spect_params(rx, cfg_new->spect_ups_req, cfg_new->spect_length,
			cfg_new->spect_bufcount, cfg_new->spect_wndtype, cfg_new->spect_wndarg,
			cfg_new->spect_magref, errbuf, errbufsize))
		{
			success = 0;
		}
	}

	return success;
}

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
				   TCHAR *errbuf, size_t errbufsize)
{
	rxstate_t *rx;

	_tcscpy(errbuf, _T(""));

	/* alloc memory */
	if((rx = calloc(1, sizeof(rxstate_t))) != NULL)
	{
		/* initialize structure */
		rx->proc_act_cb = proc_act_cb;
		rx->proc_act_ctx = proc_act_ctx;

		/* initialize events and critical sections*/
		InitializeCriticalSection(&(rx->cs_proc));
		InitializeCriticalSection(&(rx->cs_spect));
		rx->evt_iothread_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
		rx->evt_input = CreateEvent(NULL, FALSE, FALSE, NULL);

		if( (rx->evt_iothread_stop != NULL) &&
			(rx->evt_input != NULL) )
		{

			/* initialize config */
			if(rxconfig_init(&(rx->config)))
			{
				/* load config */
				rxconfig_copy(&(rx->config), cfg_init);

				/* load input module list */
				if(!input_module_list_load(&(rx->input_mod_list))) {
					_sntprintf(errbuf, errbufsize,
						_T("Can't load input module list."));
				} else {

					/* spectrum analyzer params */
					rx->spect_cb = spect_cb;
					rx->spect_cb_param = spect_cb_param;

					return rx;
				}
				/* free config */
				rxconfig_cleanup(&(rx->config));
			}
		}

		/* cleanup */
		CloseHandle(rx->evt_input);
		CloseHandle(rx->evt_iothread_stop);
		DeleteCriticalSection(&(rx->cs_spect));
		DeleteCriticalSection(&(rx->cs_proc));

		free(rx);
	}

	/* default error message */
	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize, _T("Can't initialize receiver (out of memory?)"));
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* stop receiver and processing channels, unset input module,
 * cleanup receiver, channels, input module list. */
void rx_cleanup(rxstate_t *rx, ini_data_t *ini)
{
	rxproc_t *proc, *proc_next;

	if(rx != NULL)
	{
		/* stop processing */
		rx_stop(rx);

		/* delete processing channels */
		for(proc = rx->proc_first; proc != NULL; proc = proc_next) {
			proc_next = proc->proc_next;
			rx_proc_delete(proc);
		}

		/* unload input module */
		rx_input_mod_unset(rx, ini);

		/* free input module list */
		input_module_list_cleanup(&(rx->input_mod_list));

		/* save and free config */
		rxconfig_cleanup(&(rx->config));

		/* cleanup */
		CloseHandle(rx->evt_input);
		CloseHandle(rx->evt_iothread_stop);
		DeleteCriticalSection(&(rx->cs_spect));
		DeleteCriticalSection(&(rx->cs_proc));

		free(rx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* load group of processing channels
 *	ini : channel ini file data,
 *	groupname : group of channels to load.
 * note : nothing returned in errbuf. */
void rx_proc_group_load(rxstate_t *rx, ini_data_t *ini, TCHAR *groupname,
						TCHAR *errbuf, size_t errbufsize)
{
	rxprocconfig_t cfg;
	TCHAR *sectname, proc_name[32];
	unsigned int i;
	ini_sect_t *sect;

	if(rxprocconfig_init(&cfg))
	{
		/* alloc section name buffer */
		if( (sectname = malloc( (_tcslen(groupname) + 32) * sizeof(TCHAR) )) != NULL )
		{
			i = 0; /* current channel index */

			for( ; ; )
			{
				/* get next section */
				_stprintf(sectname, _T("proc%u_%s"), i++, groupname);
				if( (sect = ini_sect_get(ini, sectname, 0)) == NULL )
					break;

				/* load channel data */
				_stprintf(proc_name, _T("noname%u"), rx->proc_nextchid);
				rxprocconfig_set_defaults(&cfg, proc_name);
				rxprocconfig_load(&cfg, sect);

				/* create channel */
				rx_proc_create(rx, &cfg, errbuf, errbufsize);
			}

			free(sectname);
		}

		rxprocconfig_cleanup(&cfg);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save group of processing channels
 *	ini : channel ini file data,
 *	groupname : name of channel group.
 * note : nothing returned in errbuf. */
void rx_proc_group_save(rxstate_t *rx, ini_data_t *ini, TCHAR *groupname)
{
	unsigned int i;
	TCHAR *sectname;
	rxproc_t *proc;
	ini_sect_t *sect;

	/* alloc section name buffer */
	if( (sectname = malloc( (_tcslen(groupname) + 32) * sizeof(TCHAR) )) != NULL )
	{
		i = 0; /* current channel index */

		/* loop through existing channels */
		for(proc = rx->proc_first; proc != NULL; proc = proc->proc_next)
		{
			_stprintf(sectname, _T("proc%u_%s"), i, groupname);
			if( (sect = ini_sect_get(ini, sectname, 1)) == NULL )
				break;

			rxprocconfig_save(sect, &(proc->cfg));

			i++;
		}

		/* delete extra sections */
		for( ; ; )
		{
			_stprintf(sectname, _T("proc%u_%s"), i, groupname);
			if( (sect = ini_sect_get(ini, sectname, 0)) == NULL )
				break;

			ini_sect_delete(sect);

			i++;
		}

		free(sectname);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* delete group of processing channels
 *	groupname : name of group to delete. */
void rx_proc_group_delete(ini_data_t *ini, TCHAR *groupname)
{
	TCHAR *sectname;
	unsigned int i;
	ini_sect_t *sect;

	/* alloc section name buffer */
	if( (sectname = malloc( (_tcslen(groupname) + 32) * sizeof(TCHAR) )) != NULL )
	{
		i = 0; /* current channel index */

		for( ; ; )
		{
			/* lookup next section */
			_stprintf(sectname, _T("proc%u_%s"), i++, groupname);
			if( (sect = ini_sect_get(ini, sectname, 0)) == NULL )
				break;

			/* delete section */
			ini_sect_delete(sect);
		}

		free(sectname);
	}
}

/* ---------------------------------------------------------------------------------------------- */
