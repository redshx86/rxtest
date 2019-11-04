/* ---------------------------------------------------------------------------------------------- */

#include "filein.h"

/* ---------------------------------------------------------------------------------------------- */

static int filein_config_init(filein_config_t *conf)
{
	conf->fname_max_len = MAX_PATH;
	conf->fname = malloc(conf->fname_max_len * sizeof(TCHAR));

	if(conf->fname != NULL)
	{
		_tcscpy(conf->fname, _T("input.wav"));
		conf->fc = 0;
		conf->read_interval = 20;
		conf->invert_iq = 1;
		conf->filewnd_fc_prefix = 2;
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_config_cleanup(filein_config_t *conf)
{
	free(conf->fname);
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_config_load(filein_ctx_t *ctx, ini_data_t *ini)
{
	ini_sect_t *sect;
	filein_config_t *conf;

	conf = &(ctx->conf);

	sect = ini_sect_get(ini, _T("filein"), 0);
	ini_copys(sect, _T("fname"), conf->fname, conf->fname_max_len);
	ini_getfr(sect, _T("fc"), &(conf->fc), FILEIN_FC_MIN, FILEIN_FC_MAX);
	ini_getur(sect, _T("interval"), &(conf->read_interval),
		FILEIN_READINT_MIN, FILEIN_READINT_MAX);
	ini_getb(sect, _T("invert"), &(conf->invert_iq));
	ini_geti(sect, _T("fc_unit_prefix"), &(conf->filewnd_fc_prefix));
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_config_save(filein_ctx_t *ctx, ini_data_t *ini)
{
	ini_sect_t *sect;
	filein_config_t *conf;

	conf = &(ctx->conf);

	sect = ini_sect_get(ini, _T("filein"), 1);
	ini_set(sect, _T("fname"), conf->fname);
	ini_setf(sect, _T("fc"), 3, conf->fc);
	ini_setu(sect, _T("interval"), conf->read_interval);
	ini_setb(sect, _T("invert"), conf->invert_iq);
	ini_seti(sect, _T("fc_unit_prefix"), conf->filewnd_fc_prefix);
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_config_showwnd(filein_ctx_t *ctx, uicommon_t *uidata, HWND hwndowner)
{
	if(ctx->filewnd != NULL) {
		DestroyWindow(ctx->filewnd->hwnd);
	} else {
		filewnd_create(uidata, hwndowner, ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static size_t filein_read(filein_state_t *s, cpxf_t *buf, size_t nsamp)
{
	float temp;
	size_t i, nsamp_avail, ns;
	size_t total_nsampread = 0;

	while(nsamp != 0)
	{
		/* no samples left? rewind file */
		if(s->pos == s->nsamp)
		{
			if(!wav_read_rewind(&(s->wavin)))
				break;
			s->pos = 0;
		}

		/* number of samples left */
		nsamp_avail = s->nsamp - s->pos;

		/* number of samples to read */
		ns = nsamp;
		if(ns > nsamp_avail)
			ns = nsamp_avail;
		if(ns > s->sampbuflen)
			ns = s->sampbuflen;

		/* read data from file */
		if(!wav_read_data(&(s->wavin), s->sampbuf, ns * s->blocksize))
			break;

		/* convert data to float samples */
		if(s->bps == 8) {
			buf_uc2f((float*)buf, s->sampbuf, ns * 2);
		} else if(s->bps == 16) {
			buf_ss2f((float*)buf, s->sampbuf, ns * 2);
		}

		/* invert components if required */
		if(s->invert_iq)
		{
			for(i = 0; i < ns; i++) {
				temp = buf[i].re;
				buf[i].re = buf[i].im;
				buf[i].im = temp;
			}
		}

		/* update read pointers */
		total_nsampread += ns;
		nsamp -= ns;
		s->pos += ns;
		buf += ns;
	}

	return total_nsampread;
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_process(filein_state_t *s)
{
	input_buf_t *buf;
	unsigned __int64 temp;
	size_t nsamp, nsamp_hdr, ns_read, ns;
	int buf_signal = 0;

	/* check error state */
	if(!s->error)
	{
		/* calculate number of samples to read per this interval */
		temp = (unsigned __int64)(s->fs) * s->timer_interval + s->nsamp_frac_rem;
		nsamp = (size_t)(temp / 1000ULL);
		s->nsamp_frac_rem = temp % 1000ULL;

		EnterCriticalSection(&(s->buf_csec));

		while(nsamp != 0)
		{
			/* get current block header */
			buf = s->buf_next_fill;
			if(buf == NULL)
				break;

			/* number of samples to read to this header */
			nsamp_hdr = buf->capacity - buf->nsamples;
			ns = min(nsamp, nsamp_hdr);

			/* fill header */
			ns_read = filein_read(s, buf->data + buf->nsamples, ns);
			buf->nsamples += ns_read;

			/* buffer full? */
			if(buf->nsamples == buf->capacity)
			{
				/* remove buffer from input queue */
				s->buf_next_fill = buf->next;
				if(s->buf_next_fill == NULL)
					s->buf_last_fill = NULL;

				/* put buffer to output queue */
				buf->next = NULL;
				if(s->buf_next_out != NULL) {
					s->buf_last_out->next = buf;
					s->buf_last_out = buf;
				} else {
					s->buf_last_out = buf;
					s->buf_next_out = buf;
				}

				/* send signal */
				buf_signal = 1;
			}

			/* check for incomplete read */
			if(ns_read != ns) {
				s->error = 1;
				break;
			}

			nsamp -= ns;
		}

		LeaveCriticalSection(&(s->buf_csec));
	}

	/* send signal */
	if(buf_signal || s->error) {
		SetEvent(s->buf_event);
	}
}

/* ---------------------------------------------------------------------------------------------- */

enum {
	FILEIN_EVT_STOP,
	FILEIN_EVT_TIMER,
	FILEIN_EVT_COUNT
};

static unsigned int __stdcall filein_thread_proc(filein_state_t *s)
{
	DWORD res;
	HANDLE events[FILEIN_EVT_COUNT];

	events[FILEIN_EVT_STOP] = s->filein_event_stop;
	events[FILEIN_EVT_TIMER] = s->filein_event_timer;

	for( ; ; )
	{
		res = WaitForMultipleObjects(FILEIN_EVT_COUNT, events, FALSE, INFINITE);

		if(res == FILEIN_EVT_STOP)
			break;

		if(res == FILEIN_EVT_TIMER)
			filein_process(s);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int filein_start(filein_ctx_t *ctx, TCHAR *errbuf, size_t errbufsize)
{
	unsigned int tid;
	filein_state_t *s = ctx->s;

	/* initialize timer */
	if(timeBeginPeriod(s->timer_tick) != TIMERR_NOERROR)
	{
		_sntprintf(errbuf, errbufsize, _T("Error: timeBeginPeriod"));
	}
	else
	{
		s->filein_event_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
		s->filein_event_timer = CreateEvent(NULL, FALSE, FALSE, NULL);

		/* start timer */
		s->timer_id = timeSetEvent(s->timer_interval, 0,
			(LPTIMECALLBACK)(s->filein_event_timer), 0,
			TIME_PERIODIC|TIME_CALLBACK_EVENT_SET);

		if(s->timer_id == 0)
		{
			_sntprintf(errbuf, errbufsize, _T("Error: timeSetEvent"));
		}
		else
		{
			/* start thread */
			s->filein_thread = (HANDLE)_beginthreadex(
				NULL, 0, filein_thread_proc, s, 0, &tid);
			if(s->filein_thread != NULL)
				return 1;

			/* stop timer */
			timeKillEvent(s->timer_id);
			s->timer_id = 0;
		}

		/* cleanup */
		CloseHandle(s->filein_event_timer);
		CloseHandle(s->filein_event_stop);
		timeEndPeriod(s->timer_tick);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_stop(filein_ctx_t *ctx)
{
	filein_state_t *s = ctx->s;

	/* stop timer */
	timeKillEvent(s->timer_id);
	s->timer_id = 0;

	/* stop thread */
	SetEvent(s->filein_event_stop);
	WaitForSingleObject(s->filein_thread, INFINITE);
	CloseHandle(s->filein_thread);

	/* cleanup */
	CloseHandle(s->filein_event_timer);
	CloseHandle(s->filein_event_stop);
	timeEndPeriod(s->timer_tick);
}

/* ---------------------------------------------------------------------------------------------- */

static int filein_calc_timer_tick(filein_state_t *s, unsigned int interval,
								  TCHAR *errbuf, size_t errbufsize)
{
	TIMECAPS tcaps;
	unsigned int tick;

	/* get timer caps */
	if(timeGetDevCaps(&tcaps, sizeof(tcaps)) != TIMERR_NOERROR) {
		_sntprintf(errbuf, errbufsize, _T("Error: timeGetDevCaps"));
		return 0;
	}
	if(tcaps.wPeriodMin == 0)
		return 0;

	/* calculate tick */
	tick = min(interval, tcaps.wPeriodMax);
	while(tick >= tcaps.wPeriodMin) {
		if(interval % tick == 0)
			break;
		tick--;
	}

	if(tick < tcaps.wPeriodMin) {
		_sntprintf(errbuf, errbufsize, _T("Can't select timer period ")
			_T("(read interval: %u, min period: %hu, max period: %hu)."),
			interval, tcaps.wPeriodMin, tcaps.wPeriodMax);
		return 0;
	}

	s->timer_tick = tick;
	s->timer_interval = interval;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int filein_open(filein_ctx_t *ctx, HANDLE event, TCHAR *errbuf, size_t errbufsize)
{
	wav_io_error_t waverr;
	filein_state_t *s;
	filein_config_t *conf;
	unsigned int nch;

	conf = &(ctx->conf);
	
	_tcscpy(errbuf, _T(""));

	/* file name is empty? */
	if( _tcscmp(conf->fname, _T("")) == 0 ) {
		_sntprintf(errbuf, errbufsize, _T("File name is empty."));
		return 0;
	}

	if((s = malloc(sizeof(filein_state_t))) != NULL)
	{
		InitializeCriticalSection(&(s->buf_csec));
		s->buf_event = event;
		s->buf_last_fill = NULL;
		s->buf_next_fill = NULL;
		s->buf_last_out = NULL;
		s->buf_next_out = NULL;

		if(!wav_read_open(&(s->wavin), conf->fname, &(s->fs), &nch, &(s->bps), &(s->nsamp), &waverr)) {
			_sntprintf(errbuf, errbufsize, _T("Error opening '%s': %s."),
				conf->fname, wav_io_errmsg(waverr));
		}
		else
		{
			if(nch != 2) {
				_sntprintf(errbuf, errbufsize, 
					_T("Can't use '%s': 2 channels required."), conf->fname);
			}
			else
			{
				s->pos = 0;

				s->blocksize = (size_t)(s->bps / 8) * 2;
				s->sampbuflen = 4096;
				s->sampbuf = malloc(s->sampbuflen * s->blocksize);

				if(s->sampbuf != NULL)
				{
					/* calculate timer period */
					if(filein_calc_timer_tick(s, conf->read_interval, errbuf, errbufsize))
					{
						s->timer_id = 0;
						s->nsamp_frac_rem = 0;
						s->error = 0;

						s->fc = conf->fc;
						s->invert_iq = conf->invert_iq;

						filewnd_setstate(ctx->filewnd, TRUE);
						ctx->s = s;

						return 1;
					}
				}

				free(s->sampbuf);
			}

			wav_read_close(&(s->wavin));
		}

		DeleteCriticalSection(&(s->buf_csec));
		free(s);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_close(filein_ctx_t *ctx)
{
	filein_state_t *s = ctx->s;

	free(s->sampbuf);
	wav_read_close(&(s->wavin));
	DeleteCriticalSection(&(s->buf_csec));
	free(s);
	ctx->s = NULL;

	filewnd_setstate(ctx->filewnd, FALSE);
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_add_bufs(filein_ctx_t *ctx, input_buf_t *bufs, size_t count)
{
	filein_state_t *s = ctx->s;
	input_buf_t *buf;

	EnterCriticalSection(&(s->buf_csec));

	while(count--)
	{
		buf = bufs++;

		buf->nsamples = 0;
		buf->next = NULL;
		if(s->buf_next_fill != NULL) {
			s->buf_last_fill->next = buf;
			s->buf_last_fill = buf;
		} else {
			s->buf_last_fill = buf;
			s->buf_next_fill = buf;
		}
	}

	LeaveCriticalSection(&(s->buf_csec));
}

/* ---------------------------------------------------------------------------------------------- */

static input_buf_t *finein_remove_buf(filein_ctx_t *ctx)
{
	filein_state_t *s = ctx->s;
	input_buf_t *buf = NULL;

	EnterCriticalSection(&(s->buf_csec));
	/* get buffer from output queue */
	if(s->buf_next_out != NULL) {
		buf = s->buf_next_out;
		s->buf_next_out = buf->next;
		if(s->buf_next_out == NULL)
			s->buf_last_out = NULL;
	}
	LeaveCriticalSection(&(s->buf_csec));
	return buf;
}

/* ---------------------------------------------------------------------------------------------- */

static int finein_check_error(filein_ctx_t *ctx)
{
	return ctx->s->error;
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_reset_bufs(filein_ctx_t *ctx)
{
	filein_state_t *s = ctx->s;

	EnterCriticalSection(&(s->buf_csec));
	s->buf_last_fill = NULL;
	s->buf_next_fill = NULL;
	s->buf_last_out = NULL;
	s->buf_next_out = NULL;
	LeaveCriticalSection(&(s->buf_csec));
}

/* ---------------------------------------------------------------------------------------------- */

static unsigned int filein_get_fs(filein_ctx_t *ctx)
{
	return ctx->s->fs;
}

/* ---------------------------------------------------------------------------------------------- */

static int filein_get_fc(filein_ctx_t *ctx, double *fcp)
{
	*fcp = ctx->s->fc;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static filein_ctx_t *filein_init(input_module_desc_t *desc, TCHAR *errbuf, size_t errbufsize)
{
	filein_ctx_t *ctx;

	_tcscpy(errbuf, _T(""));

	if( (ctx = calloc(1, sizeof(filein_ctx_t))) != NULL )
	{
		if(filein_config_init(&(ctx->conf)))
			return ctx;
		free(ctx);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_cleanup(filein_ctx_t *ctx)
{
	if(ctx->filewnd != NULL)
		DestroyWindow(ctx->filewnd->hwnd);
	filein_config_cleanup(&(ctx->conf));
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static void filein_cleanup_desc(input_module_desc_t *desc)
{
	free(desc);
}

/* ---------------------------------------------------------------------------------------------- */

input_module_desc_t *filein_get_desc()
{
	input_module_desc_t *desc;

	if((desc = calloc(1, sizeof(input_module_desc_t))) != NULL)
	{
		desc->name = _T("filein");
		desc->display_name = _T("File input");

		desc->fn_open = filein_open;
		desc->fn_close = filein_close;
		desc->fn_start = filein_start;
		desc->fn_stop = filein_stop;

		desc->fn_reset_bufs = filein_reset_bufs;
		desc->fn_add_bufs = filein_add_bufs;
		desc->fn_remove_buf = finein_remove_buf;
		desc->fn_check_err = finein_check_error;

		desc->fn_get_fs = filein_get_fs;
		desc->fn_get_fc = filein_get_fc;

		desc->fn_init = filein_init;
		desc->fn_cleanup = filein_cleanup;
		desc->fn_config_showdialog = filein_config_showwnd;
		desc->fn_config_load = filein_config_load;
		desc->fn_config_save = filein_config_save;

		desc->fn_cleanup_desc = filein_cleanup_desc;

		return desc;
	}
	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */
