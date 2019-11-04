/* ---------------------------------------------------------------------------------------------- */

#include "rxspect.h"

/* ---------------------------------------------------------------------------------------------- */

/* calculate input decimation factor for target refresh rate */
static size_t rxspect_decimf(unsigned int fs_in, unsigned int ups_req, size_t output_framelen)
{
	size_t decimf;

	decimf = fs_in / (ups_req * output_framelen);
	if(decimf == 0)
		decimf = 1;

	return decimf;
}

/* ---------------------------------------------------------------------------------------------- */

/* fft accumulated buffer and execute callback */
static void rxspect_processbuf(rxspect_ctx_t *ctx)
{
	rxspect_buf_t *buf = NULL;
	size_t i, j, bias;
	float value;
	size_t buf_used_nsamp_in;

	/* select next ready buffer */
	EnterCriticalSection(&(ctx->csec_buf));
	if(ctx->buf_donecnt != 0)
		buf = &(ctx->buf[ctx->buf_doneptr]);
	LeaveCriticalSection(&(ctx->csec_buf));

	if(buf == NULL)
		return;

	/* perform fft on ready buffer */
	fft_process(&(ctx->fft_st), buf->data, buf->data);

	/* fill output magnitude buffer */
	bias = ctx->output_framelen / 2 + 1;
	for(i = 0; i < ctx->output_framelen; i++)
	{
		j = (i + bias) & (ctx->output_framelen - 1);
		value = cpxf_modsqr(buf->data + j);
		ctx->output_magbuf[i] = value;
	}

	buf_used_nsamp_in = buf->nsamp_in_used;

	/* move ready pointer to next buffer */
	EnterCriticalSection(&(ctx->csec_buf));
	ctx->buf_doneptr++;
	if(ctx->buf_doneptr == ctx->buf_count)
		ctx->buf_doneptr = 0;
	ctx->buf_donecnt--;
	if(ctx->buf_donecnt == 0)
		ResetEvent(ctx->h_evt_bufdone);
	LeaveCriticalSection(&(ctx->csec_buf));

	/* execute callback */
	ctx->cb(ctx->cb_param, ctx->output_magbuf, ctx->output_framelen, buf_used_nsamp_in);
}

/* ---------------------------------------------------------------------------------------------- */

enum {
	RXSPECT_EVT_STOP,
	RXSPECT_EVT_BUFREADY,

	RXSPECT_EVT_COUNT
};

static unsigned int __stdcall rxspect_threadproc(rxspect_ctx_t *ctx)
{
	HANDLE evt[RXSPECT_EVT_COUNT];
	DWORD res;

	evt[RXSPECT_EVT_STOP] = ctx->h_evt_stop;
	evt[RXSPECT_EVT_BUFREADY] = ctx->h_evt_bufdone;

	for( ; ; )
	{
		res = WaitForMultipleObjects(RXSPECT_EVT_COUNT, evt, FALSE, INFINITE);

		if(res == RXSPECT_EVT_STOP)
			break;

		if(res == RXSPECT_EVT_BUFREADY)
			rxspect_processbuf(ctx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize spectrum analyzer */
int rxspect_init(rxspect_ctx_t *ctx, unsigned int fs_in, unsigned int ups_req,
				 size_t output_framelen, size_t buf_count,
				 wnd_type_t wndtype, double wndarg, double magref,
				 rxspect_callback_t cb, void *cb_param,
				 TCHAR *errbuf, size_t errbufsize)
{
	unsigned int tid;
	cpxf_t *buf;
	size_t i;
	double fc, wndsumsqr;
	float scalef;

	/* clear error message */
	_tcscpy(errbuf, _T(""));

	/* check configuration */
	if( (fs_in == 0) ||
		(!INRANGE_MINMAX(ups_req, RXLIM_SPECT_UPS_REQ)) ||
		(!INRANGE_MINMAX(output_framelen, RXLIM_SPECT_LENGTH)) ||
		(!ISPOWEROFTWO(output_framelen)) ||
		(!INRANGE_MINMAX(buf_count, RXLIM_SPECT_BUF_COUNT)) ||
		(wndtype < 0) || (wndtype >= WND_COUNT) || (cb == NULL) )
	{
		_sntprintf(errbuf, errbufsize, _T("Bad spectrum analyzer configuration."));
		return 0;
	}

	/* initialize state */
	memset(ctx, 0, sizeof(rxspect_ctx_t));

	ctx->decimf = rxspect_decimf(fs_in, ups_req, output_framelen);
	ctx->output_framelen = output_framelen;
	ctx->input_framelen = output_framelen * ctx->decimf;

	ctx->buf_count = buf_count;
	ctx->cb = cb;
	ctx->cb_param = cb_param;

	/* allocate resources*/
	InitializeCriticalSection(&(ctx->csec_buf));

	ctx->h_evt_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
	ctx->h_evt_bufdone = CreateEvent(NULL, TRUE, FALSE, NULL);

	ctx->buf = malloc(buf_count * sizeof(rxspect_buf_t));
	ctx->output_magbuf = malloc(output_framelen * sizeof(float));
	ctx->wndbuf = malloc(ctx->input_framelen * sizeof(float));
	buf = malloc(buf_count * output_framelen * sizeof(cpxf_t));

	/* check allocation results */
	if( (ctx->h_evt_stop != NULL) && (ctx->h_evt_bufdone != NULL) &&
		(ctx->buf != NULL) && (ctx->output_magbuf != NULL) &&
		(ctx->wndbuf != NULL) && (buf != NULL) )
	{
		if(fft_init(&(ctx->fft_st), ctx->output_framelen, 0))
		{
			/* initialize buffers */
			for(i = 0; i < buf_count; i++) {
				ctx->buf[i].data = buf + i * output_framelen;
				ctx->buf[i].nsamp_in_used = 0;
			}

			/* make window */
			if(ctx->decimf == 1)
			{
				/* use specifed window */
				wnd_buf(ctx->wndbuf, ctx->input_framelen, wndtype, wndarg);
			}
			else
			{
				/* make window-sinc kernel with specifed window */
				fc = 0.5 / output_framelen;
				firdes_windowsinc(ctx->wndbuf, ctx->input_framelen, fc, wndtype, wndarg);
			}

			/* normalize window energy to 1 / magref */
			wndsumsqr = sumsqrf(ctx->wndbuf, ctx->input_framelen);
			scalef = (float)(1.0 / (magref * sqrt(wndsumsqr)));
			for(i = 0; i < ctx->input_framelen; i++)
				ctx->wndbuf[i] *= scalef;

			/* start thread */
			ctx->h_thread = (HANDLE)_beginthreadex(
				NULL, 0, rxspect_threadproc, ctx, 0, &tid);

			if(ctx->h_thread != NULL)
			{
				SetThreadPriority(ctx->h_thread, THREAD_PRIORITY_LOWEST);
				return 1;
			}

			fft_cleanup(&(ctx->fft_st));
		}
	}

	/* cleanup */
	free(buf);
	free(ctx->wndbuf);
	free(ctx->output_magbuf);
	free(ctx->buf);

	CloseHandle(ctx->h_evt_bufdone);
	CloseHandle(ctx->h_evt_stop);

	DeleteCriticalSection(&(ctx->csec_buf));

	/* default error message */
	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize, 
			_T("Can't initialize spectrum analyzer (out of memory?)"));
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* free spectrum analyzer */
void rxspect_cleanup(rxspect_ctx_t *ctx)
{
	/* stop processing thread */
	SetEvent(ctx->h_evt_stop);
	WaitForSingleObject(ctx->h_thread, INFINITE);
	CloseHandle(ctx->h_thread);

	/* cleanup */
	fft_cleanup(&(ctx->fft_st));

	free(ctx->buf[0].data);
	free(ctx->wndbuf);
	free(ctx->output_magbuf);
	free(ctx->buf);

	CloseHandle(ctx->h_evt_bufdone);
	CloseHandle(ctx->h_evt_stop);

	DeleteCriticalSection(&(ctx->csec_buf));
}

/* ---------------------------------------------------------------------------------------------- */

/* write data to spectrum analyzer */
size_t rxspect_writedata(rxspect_ctx_t *ctx, cpxf_t *data, size_t nsamp)
{
	size_t nsampused = 0;
	rxspect_buf_t *buf;
	size_t bufindex;
	size_t wrptr, nsampwr, i;
	cpxf_t temp, *src, *dst;
	float *wp;

	/* try process all input samples */
	while(nsamp != 0)
	{
		/* select current buffer to accumulate data */
		buf = NULL;
		EnterCriticalSection(&(ctx->csec_buf));
		if(ctx->buf_donecnt != ctx->buf_count)
		{
			bufindex = ctx->buf_doneptr + ctx->buf_donecnt;
			if(bufindex >= ctx->buf_count)
				bufindex -= ctx->buf_count;
			buf = &(ctx->buf[bufindex]);
		}
		LeaveCriticalSection(&(ctx->csec_buf));

		/* break if all buffers full */
		if(buf == NULL)
			break;

		/* clean current buffer at start */
		if(ctx->input_ptr == 0) {
			memset(buf->data, 0, ctx->output_framelen * sizeof(cpxf_t));
			buf->nsamp_in_used = 0;
		}

		/* accumulate data to current buffer */
		while( (ctx->input_ptr != ctx->input_framelen) && (nsamp != 0) )
		{
			wrptr = ctx->input_ptr % ctx->output_framelen;
			nsampwr = min(ctx->output_framelen - wrptr, nsamp);

			src = data + nsampused;
			dst = buf->data + wrptr;
			wp = ctx->wndbuf + ctx->input_ptr;
			for(i = 0; i < nsampwr; i++)
			{
				cpxf_mulr(&temp, src++, *(wp++));
				cpxf_add(dst, dst, &temp);
				dst++;
			}

			ctx->cur_buf_nsamp_in_used += nsampwr;

			ctx->input_ptr += nsampwr;
			nsampused += nsampwr;
			nsamp -= nsampwr;
		}

		/* select next buffer when current buffer ready */
		if(ctx->input_ptr == ctx->input_framelen)
		{
			EnterCriticalSection(&(ctx->csec_buf));
			buf->nsamp_in_used = ctx->cur_buf_nsamp_in_used;
			ctx->cur_buf_nsamp_in_used = 0;
			ctx->buf_donecnt++;
			ctx->input_ptr = 0;
			SetEvent(ctx->h_evt_bufdone);
			LeaveCriticalSection(&(ctx->csec_buf));
		}
	}

	/* dropped samples also "used" for current frame */
	ctx->cur_buf_nsamp_in_used += nsamp;

	return nsampused;
}

/* ---------------------------------------------------------------------------------------------- */
