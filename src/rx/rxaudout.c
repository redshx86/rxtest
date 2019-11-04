/* ---------------------------------------------------------------------------------------------- */

#include "rxaudout.h"

/* ---------------------------------------------------------------------------------------------- */

/* open output device */
int rx_audio_output_open(rx_audio_output_t *c,
						 unsigned int fs_base, size_t base_framelen, rxconfig_t *params,
						 TCHAR *errbuf, size_t errbufsize)
{
	size_t output_framelen, output_hdrnsamp, output_hdrlen;

	_tcscpy(errbuf, _T(""));

	/* check parameters */
	if( (fs_base == 0) || (base_framelen == 0) ||
		(!INRANGE_MINMAX(params->proc_buf_len_ms, RXLIM_PROC_BUF_LEN_MS)) ||
		(!INRANGE_MINMAX(params->proc_out_rhyst_ms, RXLIM_PROC_BUF_LEN_MS)) ||
		(params->proc_out_rhyst_ms > params->proc_buf_len_ms) ||
		(!INRANGE_MINMAX(params->output_static_gain, RXLIM_OUTPUT_STATIC_GAIN)) ||
		(!INRANGE_MINMAX(params->output_resamp_df, RXLIM_OUTPUT_RESAMP_DF)) ||
		(!INRANGE_MINMAX(params->output_resamp_as, RXLIM_OUTPUT_RESAMP_AS)) ||
		(!INRANGE_MINMAX(params->output_lim_thres, RXLIM_OUTPUT_LIM_THRES)) ||
		(!INRANGE_MINMAX(params->output_lim_range, RXLIM_OUTPUT_LIM_RANGE)) ||
		(!INRANGE_MINMAX(params->output_hdr_cnt, RXLIM_OUTPUT_HDR_CNT)) ||
		(!INRANGE_MINMAX(params->output_hdr_ms, RXLIM_OUTPUT_HDR_MS)) ||
		(!INRANGE(params->output_device_id, -1, RXLIM_OUTPUT_MAX_NUM_DEVS - 1)) ||
		(!INRANGE_MINMAX(params->output_fs, RXLIM_OUTPUT_FS)) ||
		((params->output_bps != 8) && (params->output_bps != 16)) ||
		(!INRANGE_MINMAX(params->output_levelmtr_num_points, RXLIM_OUTPUT_LEVELMTR_NUM_POINTS)) ||
		(!INRANGE_MINMAX(params->output_levelmtr_tick_interval, RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL)) ||
		(!INRANGE_MINMAX(params->output_gain, RXLIM_AUDIO_GAIN)) )
	{
		_sntprintf(errbuf, errbufsize, _T("Bad audio output configuration."));
		return 0;
	}

	/* initialize structure */
	InitializeCriticalSection(&(c->cs_proc));

	c->fs_base = fs_base;
	c->base_framelen = base_framelen;

	c->output_fs = params->output_fs;
	c->output_bps = params->output_bps;
	output_framelen = (size_t)(((unsigned __int64)base_framelen * params->output_fs + fs_base - 1) / fs_base);

	c->mix_inbuf_len = MSECS2NSAMP(params->proc_buf_len_ms, fs_base);
	c->mix_inbuf_rhyst = MSECS2NSAMP(params->proc_out_rhyst_ms, fs_base);

	/* initialize baseband mixing buffer */
	c->mix_buf_len = base_framelen;
	c->mix_buf = malloc(c->mix_buf_len * 2 * sizeof(float));

	/* initialize output buffer */
	c->out_buf_len = output_framelen;
	c->out_buf = malloc(c->out_buf_len * 2 * sizeof(float));
	c->out_lim_buf = malloc(c->out_buf_len * 2 * sizeof(float));

	/* initialize output raw buffer */
	c->raw_blocksize = (size_t)(c->output_bps / 8) * 2;
	c->raw_buf_len = c->out_buf_len;
	c->raw_buf = malloc(c->raw_buf_len * c->raw_blocksize);

	if( (c->mix_buf != NULL) && (c->out_buf != NULL) &&
		(c->out_lim_buf != NULL) && (c->raw_buf != NULL) )
	{
		/* initialize baseband audio mixer */
		if(!sndmix_init(&(c->st_mix), params->output_static_gain, params->output_gain))
		{
			_sntprintf(errbuf, errbufsize,
				_T("Can't initialize audio output mixer (static gain = %.2f dB)."),
				params->output_static_gain);
		}
		else
		{
			/* initialize resampler */
			if(!resamp_init(&(c->st_resamp), 2, fs_base, params->output_fs,
				params->output_resamp_as, params->output_resamp_df))
			{
				_sntprintf(errbuf, errbufsize,
					_T("Can't initialize audio output resampler ")
					_T("(fs_base = %u Hz, fs_out = %u Hz, as = %.2f dB, df = %u Hz)."),
					fs_base, params->output_fs, params->output_resamp_as,
					params->output_resamp_df);
			}
			else
			{
				/* initialize limiter (no cleanup neeeded) */
				if(!rangelim_init(&(c->st_lim),
					params->output_lim_range, params->output_lim_thres))
				{
					_sntprintf(errbuf, errbufsize,
						_T("Can't initialize audio output limiter ")
						_T("(thres = %.2f, range = %.2f)."),
						params->output_lim_thres, params->output_lim_range);
				}
				else
				{
					/* initialize level meter */
					if(!levelmtr_init_fs(&(c->st_levelmtr), 2, params->output_levelmtr_num_points, 
						params->output_fs, 0.001, params->output_levelmtr_tick_interval))
					{
						_sntprintf(errbuf, errbufsize,
							_T("Can't initialize audio level meter ")
							_T("(num_points = %Iu, fs = %u Hz, interval = %.2f ms)."),
							params->output_levelmtr_num_points, params->output_fs,
							params->output_levelmtr_tick_interval);
					}
					else
					{
						/* calculate audio buffer size */
						output_hdrnsamp = MSECS2NSAMP(params->output_hdr_ms, params->output_fs);
						output_hdrlen = CEIL2POT(output_hdrnsamp, 256UL);

						/* open device */
						if(!sndout_open(&(c->st_outdev), params->output_device_id, 2,
							params->output_fs, params->output_bps,
							params->output_hdr_cnt, output_hdrlen))
						{
							_sntprintf(errbuf, errbufsize,
								_T("Can't open audio output device ")
								_T("(Id = %d, fs = %u Hz, res = %u bps, ")
								_T("num_bufs = %Iu, buf_len = %Iu samples)."),
								params->output_device_id, params->output_fs, params->output_bps,
								params->output_hdr_cnt, output_hdrlen);
						}
						else
						{
							return 1;
						}

						levelmtr_free(&(c->st_levelmtr));
					}
				}

				resamp_cleanup(&(c->st_resamp));
			}

			sndmix_cleanup(&(c->st_mix));
		}
	}

	free(c->raw_buf);
	free(c->out_lim_buf);
	free(c->out_buf);
	free(c->mix_buf);

	DeleteCriticalSection(&(c->cs_proc));

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* close output device */
void rx_audio_output_close(rx_audio_output_t *c)
{
	rx_audio_output_sink_t *sink, *sink_next;

	/* delete sinks */
	for(sink = c->sink_head; sink != NULL; sink = sink_next)
	{
		sink_next = sink->sink_next;
		rx_audio_output_deletesink(sink);
	}

	/* free components */
	sndout_close(&(c->st_outdev));
	levelmtr_free(&(c->st_levelmtr));
	resamp_cleanup(&(c->st_resamp));
	sndmix_cleanup(&(c->st_mix));

	/* cleanup */
	free(c->raw_buf);
	free(c->out_lim_buf);
	free(c->out_buf);
	free(c->mix_buf);

	DeleteCriticalSection(&(c->cs_proc));
}

/* ---------------------------------------------------------------------------------------------- */

/* copy data from mixer buffers to output device input */
void rx_audio_output_processdata(rx_audio_output_t *c, size_t nsamp_total)
{
	size_t ns_process, ns_out;
	size_t est_nsamp_delay;
	unsigned int est_ms_delay, est_out_tick;

	EnterCriticalSection(&(c->cs_proc));

	/* process specified number of samples */
	while(nsamp_total != 0)
	{
		/* number of samples to process at once */
		ns_process = min(nsamp_total, c->base_framelen);

		/* read data from mixer buffers */
		sndmix_output_data(&(c->st_mix), c->mix_buf, ns_process);

		/* resample data */
		ns_out = resamp_exec(&(c->st_resamp), c->out_buf, c->mix_buf, ns_process);

		/* limit range to [-1, 1] */
		rangelim_process(&(c->st_lim), c->out_lim_buf, c->out_buf, ns_out * 2);

		/* convert data to raw format */
		switch(c->output_bps)
		{
		case 8:
			buf_f2uc(c->raw_buf, c->out_lim_buf, ns_out * 2);
			break;
		case 16:
			buf_f2ss(c->raw_buf, c->out_lim_buf, ns_out * 2);
			break;
		}

		/* send data to level meter */
		est_nsamp_delay = sndout_nsamp_pending(&(c->st_outdev));
		est_ms_delay = (unsigned int)(1000UL * est_nsamp_delay / c->output_fs);
		est_out_tick = GetTickCount() + est_ms_delay;
		levelmtr_write(&(c->st_levelmtr), c->out_buf, 0, ns_out, est_out_tick);

		/* send data to device */
		sndout_write(&(c->st_outdev), c->raw_buf, ns_out);

		nsamp_total -= ns_process;
	}

	LeaveCriticalSection(&(c->cs_proc));
}

/* ---------------------------------------------------------------------------------------------- */

void rx_audio_output_set_output_gain(rx_audio_output_t *c, double gain)
{
	EnterCriticalSection(&(c->cs_proc));
	sndmix_output_gain(&(c->st_mix), gain);
	LeaveCriticalSection(&(c->cs_proc));
}

/* ---------------------------------------------------------------------------------------------- */

int rx_audio_output_read_level(rx_audio_output_t *c, double *mag, double *peak,
							   unsigned int tick_peak_from, unsigned int tick)
{
	return levelmtr_read_peak(&(c->st_levelmtr), tick_peak_from, tick, mag, peak);
}

/* ---------------------------------------------------------------------------------------------- */

/* create data sink */
rx_audio_output_sink_t *rx_audio_output_createsink(rx_audio_output_t *c)
{
	rx_audio_output_sink_t *sink;

	if( (sink = calloc(1, sizeof(rx_audio_output_sink_t))) != NULL )
	{
		sink->c = c;

		if( (sink->in = sndmix_input_create(&(c->st_mix),
			c->mix_inbuf_len, c->mix_inbuf_rhyst, 0)) != NULL )
		{
			DLIST_INSBACK(sink, c->sink_head, c->sink_tail, sink_next, sink_prev);
			return sink;
		}

		free(sink);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* delete data sink */
void rx_audio_output_deletesink(rx_audio_output_sink_t *sink)
{
	rx_audio_output_t *c;

	c = sink->c;

	DLIST_REMOVE(sink, c->sink_head, c->sink_tail, sink_next, sink_prev);

	sndmix_input_destroy(sink->in);
	free(sink);
}

/* ---------------------------------------------------------------------------------------------- */

/* write to data sink and calculate delay estimate */
size_t rx_audio_output_writesink(rx_audio_output_sink_t *sink,
								 float *data, size_t nsamp, sndmix_input_mode_t mode,
								 size_t *pnsamp_est_delay)
{
	rx_audio_output_t *c;
	sndmix_input_t *in;
	size_t nsamp_est_delay, nsamp_raw_pend;

	c = sink->c;
	in = sink->in;

	if(pnsamp_est_delay != NULL)
	{
		EnterCriticalSection(&(c->cs_proc));

		/* audio mixer buffer pending */
		nsamp_est_delay = in->buf.cnt;

		/* resampler delay */
		nsamp_est_delay += resamp_get_delay(&(c->st_resamp));

		/* device output buffer pending */
		nsamp_raw_pend = sndout_nsamp_pending(&(c->st_outdev));
		nsamp_est_delay += (size_t)((unsigned __int64)nsamp_raw_pend *
			c->st_resamp.M / c->st_resamp.L);

		*pnsamp_est_delay = nsamp_est_delay;

		LeaveCriticalSection(&(c->cs_proc));
	}

	return sndmix_input_data(in, data, nsamp, mode);
}

/* ---------------------------------------------------------------------------------------------- */
