/* ---------------------------------------------------------------------------------------------- */

#include "rtlsdr.h"

/* ---------------------------------------------------------------------------------------------- */

/* tuner center frequency adjust range */
static rtlsdr_tuner_fc_range_t rtlsdr_tuning_range[] =
{
	{          0U,             0U,             0U,             0U },	/* Unknown */
	{   52000000U,    2200000000U,    1100000000U,    1250000000U },	/* E4000 */
	{   22000000U,     948600000U,             0U,             0U },	/* FC0012 */
	{   22000000U,    1100000000U,             0U,             0U },	/* FC0013 */
	{  146000000U,     924000000U,     308000001U,     437999999U },	/* FC2580 */
	{   24000000U,    1766000000U,             0U,             0U },	/* R820T */
	{   24000000U,    1766000000U,             0U,             0U }		/* R828D */
};

/* ---------------------------------------------------------------------------------------------- */

static int rtlsdr_compareint(const int *a, const int *b)
{
	if(*a < *b) return -1;
	if(*a > *b) return 1;
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* check/limit tuner center frequency */
static int rtlsdr_checkfc(unsigned int *fcp, int tuner_type)
{
	int status;
	rtlsdr_tuner_fc_range_t *r;
	unsigned int fc_cur;

	r = &(rtlsdr_tuning_range[tuner_type]);;
	fc_cur = *fcp;
	status = 1;

	if(fc_cur < r->fc_start) {
		fc_cur = r->fc_start;
		status = 0;
	}

	if(fc_cur > r->fc_end) {
		fc_cur = r->fc_end;
		status = 0;
	}

	if((fc_cur >= r->fc_gap_start) && (fc_cur <= r->fc_gap_end))
	{
		if(fc_cur < (r->fc_gap_end + r->fc_gap_start) / 2)
			fc_cur = r->fc_gap_start - 1;
		else
			fc_cur = r->fc_gap_end + 1;
		status = 0;
	}

	*fcp = fc_cur;
	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* read information from device */
static int rtlsdr_readinfo(rtlsdr_ctx_t *ctx, rtlsdr_deviceinfo_t *devinfo, void *dev)
{
	/* read tuner type */
	devinfo->tuner_type = ctx->proc->get_tuner_type(dev);
	if( (devinfo->tuner_type <= RTLSDR_TUNER_UNKNOWN) ||
		(devinfo->tuner_type >= RTLSDR_TUNER_TYPE_COUNT) )
	{
		return 0;
	}

	devinfo->is_offset_tuning_supported = 
		(devinfo->tuner_type != RTLSDR_TUNER_R820T) &&
		(devinfo->tuner_type != RTLSDR_TUNER_R828D);

	/* read number of possible tuner gains */
	devinfo->num_tuner_gains = ctx->proc->get_tuner_gains(dev, NULL);
	if(devinfo->num_tuner_gains < 0)
		return 0;

	if(devinfo->num_tuner_gains != 0)
	{
		/* allocate buffer */
		devinfo->tuner_gains_buf = malloc(devinfo->num_tuner_gains * sizeof(int));
		if(devinfo->tuner_gains_buf == NULL)
			return 0;

		/* copy possible tuner gains */
		if(ctx->proc->get_tuner_gains(dev, devinfo->tuner_gains_buf) <= 0)
		{
			free(devinfo->tuner_gains_buf);
			return 0;
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* free device info */
static void rtlsdr_freedeviceinfo(rtlsdr_deviceinfo_t *devinfo)
{
	free(devinfo->tuner_gains_buf);
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize default device configuration */
static void rtlsdr_initconfig(rtlsdr_deviceconfig_t *cfg)
{
	memset(cfg, 0, sizeof(rtlsdr_deviceconfig_t));

	cfg->fs				=  1000000;
	cfg->fc				= 27185000;

	cfg->buf_count		= 16;
	cfg->buf_len_ms		= 30.0;
}

/* ---------------------------------------------------------------------------------------------- */

/* check device config */
static int rtlsdr_checkconfig(rtlsdr_deviceconfig_t *devcfg, rtlsdr_deviceinfo_t *devinfo)
{
	unsigned int num, fc;

	/* check direct sampling mode */
	if((devcfg->direct_sampling < 0) || (devcfg->direct_sampling > 2))
		return 0;

	/* check sampling frequency */
	if( (!INRANGE(devcfg->fs, 225001, 300000)) &&
		(!INRANGE(devcfg->fs, 900001, 3200000)) )
		return 0;

	/* check center frequency */
	fc = devcfg->fc;
	if(!rtlsdr_checkfc(&fc, devinfo->tuner_type))
		return 0;

	/* check offset tuning mode */
	if((devcfg->offset_tuning_on != 0) && (devcfg->offset_tuning_on != 1))
		return 0;
	
	/* check manual tuner gain mode */
	if((devcfg->tuner_gain_manual != 0) && (devcfg->tuner_gain_manual != 1))
		return 0;
	
	/* check digital agc mode */
	if((devcfg->rtl_agc_on != 0) && (devcfg->rtl_agc_on != 1))
		return 0;

	/* check tuner gain value */
	if(devcfg->tuner_gain_manual)
	{
		num = devinfo->num_tuner_gains;
		if( _lfind(&(devcfg->tuner_gain_value), devinfo->tuner_gains_buf,
			&num, sizeof(int), rtlsdr_compareint) == NULL)
		{
			return 0;
		}
	}

	/* check frequency correction */
	if(!INRANGE_MINMAX(devcfg->freqcorr, RTLSDR_FREQCORR))
		return 0;

	/* check buffer count */
	if(!INRANGE_MINMAX(devcfg->buf_count, RTLSDR_BUFFER_COUNT))
		return 0;

	/* check buffer length */
	if(!INRANGE_MINMAX(devcfg->buf_len_ms, RTLSDR_BUFFER_MS))
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* check device config and format error message */
static int rtlsdr_verifyconfig(rtlsdr_deviceconfig_t *devcfg, rtlsdr_deviceinfo_t *devinfo,
							   TCHAR *errbuf, size_t errbufsize)
{
	unsigned int num, fc;

	_tcscpy(errbuf, _T(""));

	/* check direct sampling mode */
	if((devcfg->direct_sampling < 0) || (devcfg->direct_sampling > 2))
	{
		_sntprintf(errbuf, errbufsize, _T("Bad RTL-SDR direct sampling mode (%d).")
			_T("Correct range: 0 to 2."), devcfg->direct_sampling);
		return 0;
	}

	/* check sampling frequency */
	if( (!INRANGE(devcfg->fs, 225001, 300000)) &&
		(!INRANGE(devcfg->fs, 900001, 3200000)) )
	{
		_sntprintf(errbuf, errbufsize, _T("Bad RTL-SDR sampling rate (%u Hz). ")
			_T("Correct range: 225001 to 300000 Hz or 900001 to 3200000 Hz."),
			devcfg->fs);
		return 0;
	}

	/* check center frequency */
	fc = devcfg->fc;
	if(!rtlsdr_checkfc(&fc, devinfo->tuner_type))
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad center frequency for current tuner type: %u Hz."), devcfg->fc);
		return 0;
	}

	/* check offset tuning mode */
	if((devcfg->offset_tuning_on != 0) && (devcfg->offset_tuning_on != 1))
	{
		_sntprintf(errbuf, errbufsize, _T("Bad RTL-SDR offset tuning mode (%d). ")
			_T("Correct value: 0 or 1."), devcfg->offset_tuning_on);
		return 0;
	}

	/* check manual tuner gain mode */
	if((devcfg->tuner_gain_manual != 0) && (devcfg->tuner_gain_manual != 1))
	{
		_sntprintf(errbuf, errbufsize, _T("Bad RTL-SDR manual tuner gain mode (%d). ")
			_T("Correct value: 0 or 1."), devcfg->tuner_gain_manual);
		return 0;
	}

	/* check digital agc mode */
	if((devcfg->rtl_agc_on != 0) && (devcfg->rtl_agc_on != 1))
	{
		_sntprintf(errbuf, errbufsize, _T("Bad RTL2832 AGC mode (%d).")
			_T("Correct value: 0 or 1."), devcfg->rtl_agc_on);
		return 0;
	}

	/* check manual tuner gain value */
	if(devcfg->tuner_gain_manual)
	{
		num = devinfo->num_tuner_gains;
		if( _lfind(&(devcfg->tuner_gain_value), devinfo->tuner_gains_buf,
			&num, sizeof(int), rtlsdr_compareint) == NULL)
		{
			_sntprintf(errbuf, errbufsize,
				_T("Gain value unsupported by tuner (%.1f dB)."),
				0.1 * (double)(devcfg->tuner_gain_value));
			return 0;
		}
	}

	/* check frequency correction value */
	if(!INRANGE_MINMAX(devcfg->freqcorr, RTLSDR_FREQCORR))
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad RTL-SDR frequency correction value (%d ppm). Correct range: %d to %d."),
			devcfg->freqcorr, RTLSDR_FREQCORR_MIN, RTLSDR_FREQCORR_MAX);
		return 0;
	}

	/* check buffer count */
	if(!INRANGE_MINMAX(devcfg->buf_count, RTLSDR_BUFFER_COUNT))
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad RTL-SDR buffer count (%d). Correct range: %d to %d."),
			devcfg->buf_count, RTLSDR_BUFFER_COUNT_MIN, RTLSDR_BUFFER_COUNT_MAX);
		return 0;
	}

	/* check buffer length */
	if(!INRANGE_MINMAX(devcfg->buf_len_ms, RTLSDR_BUFFER_MS))
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad RTL-SDR buffer length (%d ms). Correct range: %d to %d ms."),
			devcfg->buf_len_ms, RTLSDR_BUFFER_MS_MIN, RTLSDR_BUFFER_MS_MAX);
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set device configuration */
static int rtlsdr_setconfig(rtlsdr_ctx_t *ctx, rtlsdr_deviceconfig_t *devcfg, void *dev,
							rtlsdr_deviceinfo_t *devinfo, TCHAR *errbuf, size_t errbufsize)
{
	int res;

	_tcscpy(errbuf, _T(""));

	/* check device configuration */
	if(!rtlsdr_verifyconfig(devcfg, devinfo, errbuf, errbufsize))
		return 0;

	/* set direct sampling mode */
	if(ctx->proc->set_direct_sampling(dev, devcfg->direct_sampling) != 0)
	{
		_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR direct sampling mode (%d)."),
			devcfg->direct_sampling);
		return 0;
	}

	/* set sampling frequency */
	if(ctx->proc->set_sample_rate(dev, devcfg->fs) != 0)
	{
		_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR sampling frequency (%u Hz)."),
			devcfg->fs);
		return 0;
	}

	/* set center frequency */
	if(ctx->proc->set_center_freq(dev, devcfg->fc) != 0)
	{
		_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR center frequency (%u Hz)."),
			devcfg->fc);
		return 0;
	}

	/* set offset tuning mode */
	if(devinfo->is_offset_tuning_supported && (devcfg->direct_sampling == 0))
	{
		if(ctx->proc->set_offset_tuning(dev, devcfg->offset_tuning_on) != 0)
		{
			_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR offset tuning mode (%s)."),
				devcfg->offset_tuning_on ? _T("on") : _T("off"));
			return 0;
		}
	}

	/* set manual tuner gain mode */
	if(ctx->proc->set_tuner_gain_mode(dev, devcfg->tuner_gain_manual) != 0)
	{
		_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR tuner gain mode (%s)."),
			devcfg->tuner_gain_manual ? _T("manual") : _T("auto"));
		return 0;
	}

	/* set manual tuner gain value */
	if(devcfg->tuner_gain_manual)
	{
		if(ctx->proc->set_tuner_gain(dev, devcfg->tuner_gain_value) != 0)
		{
			_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR tuner gain (%.1f dB)."),
				0.1 * (double)(devcfg->tuner_gain_value));
			return 0;
		}
	}

	/* set didigtal agc mode */
	if(ctx->proc->set_agc_mode(dev, devcfg->rtl_agc_on) != 0)
	{
		_sntprintf(errbuf, errbufsize, _T("Can't set RTL2832 AGC mode (%s)."),
			devcfg->rtl_agc_on ? _T("on") : _T("off"));
		return 0;
	}

	/* set frequency correction value */
	res = ctx->proc->set_freq_correction(dev, devcfg->freqcorr);
	if((res != 0) && (res != -2))
	{
		_sntprintf(errbuf, errbufsize, _T("Can't set RTL-SDR frequency correction (%d ppm)."),
			devcfg->freqcorr);
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update device config */
int rtlsdr_updateconfig(rtlsdr_ctx_t *ctx, rtlsdr_deviceconfig_t *devcfgcur,
						rtlsdr_deviceconfig_t *devcfgnew, void *dev,
						rtlsdr_deviceinfo_t *devinfo, TCHAR *errbuf, size_t errbufsize)
{
	int cur_tuner_gain_manual, res;

	_tcscpy(errbuf, _T(""));

	/* check new device configuration */
	if(!rtlsdr_verifyconfig(devcfgnew, devinfo, errbuf, errbufsize))
		return 0;

	/* set new offset tuning mode */
	if( devinfo->is_offset_tuning_supported && (devcfgcur->direct_sampling == 0) &&
		(devcfgnew->offset_tuning_on != devcfgcur->offset_tuning_on) )
	{
		if(ctx->proc->set_offset_tuning(dev, devcfgnew->offset_tuning_on) != 0)
		{
			_sntprintf(errbuf, errbufsize,
				_T("Can't change RTL-SDR offset tuning mode (%s to %s)."),
				devcfgcur->offset_tuning_on ? _T("on") : _T("off"),
				devcfgnew->offset_tuning_on ? _T("on") : _T("off"));
			return 0;
		}
		devcfgcur->offset_tuning_on = devcfgnew->offset_tuning_on;
	}

	/* set new manual tuner gain mode */
	cur_tuner_gain_manual = devcfgcur->tuner_gain_manual;
	if(devcfgnew->tuner_gain_manual != devcfgcur->tuner_gain_manual)
	{
		if(ctx->proc->set_tuner_gain_mode(dev, devcfgnew->tuner_gain_manual) != 0)
		{
			_sntprintf(errbuf, errbufsize,
				_T("Can't change tuner gain mode (%s to %s)."),
				devcfgcur->tuner_gain_manual ? _T("manual") : _T("auto"),
				devcfgnew->tuner_gain_manual ? _T("manual") : _T("auto"));
			return 0;
		}
		devcfgcur->tuner_gain_manual = devcfgnew->tuner_gain_manual;
	}

	/* set new manual tuner gain value */
	if(devcfgnew->tuner_gain_manual)
	{
		if( (!cur_tuner_gain_manual) ||
			(devcfgnew->tuner_gain_value != devcfgcur->tuner_gain_value) )
		{
			if(ctx->proc->set_tuner_gain(dev, devcfgnew->tuner_gain_value) != 0)
			{
				_sntprintf(errbuf, errbufsize,
					_T("Can't change tuner gain (%.1f to %.1f dB)."),
					0.1 * (double)(devcfgcur->tuner_gain_value),
					0.1 * (double)(devcfgnew->tuner_gain_value));
				return 0;
			}
			devcfgcur->tuner_gain_value = devcfgnew->tuner_gain_value;
		}
	}

	/* set new digital agc mode */
	if(devcfgnew->rtl_agc_on != devcfgcur->rtl_agc_on)
	{
		if(ctx->proc->set_agc_mode(dev, devcfgnew->rtl_agc_on) != 0)
		{
			_sntprintf(errbuf, errbufsize,
				_T("Can't change RTL2832 AGC mode (%s to %s)."),
				devcfgcur->rtl_agc_on ? _T("on") : _T("off"),
				devcfgnew->rtl_agc_on ? _T("on") : _T("off"));
			return 0;
		}
		devcfgcur->rtl_agc_on = devcfgnew->rtl_agc_on;
	}

	/* set new frequency correction value */
	if(devcfgnew->freqcorr != devcfgcur->freqcorr)
	{
		res = ctx->proc->set_freq_correction(dev, devcfgnew->freqcorr);
		if((res != 0) && (res != -2))
		{
			_sntprintf(errbuf, errbufsize,
				_T("Can't change RTL-SDR frequency correction (%d to %d ppm)."),
				devcfgcur->freqcorr, devcfgnew->freqcorr);
			return 0;
		}
		devcfgcur->freqcorr = devcfgnew->freqcorr;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* get device information by device index */
static int rtlsdr_getdeviceinfo(rtlsdr_ctx_t *ctx, unsigned int index, rtlsdr_deviceinfo_t *devinfo)
{
	void *dev;
	char *name;

	/* get device name */
	if( (name = ctx->proc->get_device_name(index)) != NULL )
	{
		strncpy(devinfo->name, name, sizeof(devinfo->name));
		devinfo->name[sizeof(devinfo->name) - 1] = 0;

		/* open device */
		if(ctx->proc->open(&dev, index) == 0)
		{
			/* read device information */
			if(rtlsdr_readinfo(ctx, devinfo, dev))
			{
				/* close device */
				ctx->proc->close(dev);
				return 1;
			}
			/* close device */
			ctx->proc->close(dev);
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* data read callback */
static void rtlsdr_read_cb(unsigned char *buf, unsigned int len, rtlsdr_ctx_t *ctx)
{
	cpxf_t *ptr;
	size_t nsamp, nsamp_hdr, nsamp_copy, i;
	rtlsdr_state_t *s = ctx->s;
	input_buf_t *header;
	int buf_event = 0;

	/* get number of samples in buffer */
	nsamp = len / 2;

	EnterCriticalSection(&(s->buf_csec));

	/* process captured samples */
	while(nsamp != 0)
	{
		/* get buffer to copy data to */
		header = s->buf_next_fill;
		if(header == NULL)
			break;

		/* calculate number of samples to write to current buffer */
		nsamp_hdr = header->capacity - header->nsamples;
		nsamp_copy = min(nsamp_hdr, nsamp);

		/* copy data to current buffer */
		ptr = header->data + header->nsamples;
		for(i = 0; i < nsamp_copy; i++)
		{
			ptr->re = ((float)*(buf++) - 127.0f) * 0.008f;
			ptr->im = ((float)*(buf++) - 127.0f) * 0.008f;
			ptr++;
		}
		header->nsamples += nsamp_copy;

		/* check current buffer full */
		if(header->nsamples == header->capacity)
		{
			/* remove header from pending queue */
			s->buf_next_fill = header->next;
			if(s->buf_next_fill == NULL)
				s->buf_last_fill = NULL;

			/* put buffer to done queue */
			header->next = NULL;
			if(s->buf_last_out != NULL) {
				s->buf_last_out->next = header;
				s->buf_last_out = header;
			} else {
				s->buf_last_out = header;
				s->buf_next_out = header;
			}

			/* notify input thread of new buffers ready */
			buf_event = 1;
		}

		/* subtract copied samples from remaining quanity */
		nsamp -= nsamp_copy;
	}

	LeaveCriticalSection(&(s->buf_csec));

	if(buf_event) {
		SetEvent(s->buf_event);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* data read thread proc */
static unsigned int __stdcall rtlsdr_read_threadproc(rtlsdr_ctx_t *ctx)
{
	unsigned int buf_count, buf_nsamp;
	int res;

	/* get number of read buffers */
	buf_count = ctx->s->devcfg->buf_count;

	/* calculate read buffer length in samples */
	buf_nsamp = (unsigned int)CEIL2POT(
		MSECS2NSAMP(ctx->s->devcfg->buf_len_ms, ctx->s->devcfg->fs), 256UL);

	/* reset device endpoint */
	if(ctx->proc->reset_buffer(ctx->s->dev) != 0) {
		ctx->s->error = 1;
		SetEvent(ctx->s->buf_event);
		return 0;
	}

	/* read data */
	ctx->s->read_async_entered = 1;
	res = ctx->proc->read_async(ctx->s->dev, rtlsdr_read_cb, ctx, buf_count, buf_nsamp * 2);
	ctx->s->read_async_entered = 0;

	if(res != 0) {
		ctx->s->error = 1;
		SetEvent(ctx->s->buf_event);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* start data reading */
static int rtlsdr_start(rtlsdr_ctx_t *ctx, TCHAR *errbuf, size_t errbufsize)
{
	_tcscpy(errbuf, _T(""));

	/* check device is opened */
	if(ctx->s == NULL) {
		_sntprintf(errbuf, errbufsize, _T("RTL-SDR device is not opened."));
		return 0;
	}

	/* check data reading is not already started */
	if(ctx->s->is_started) {
		_sntprintf(errbuf, errbufsize, _T("RTL-SDR device already started."));
		return 0;
	}

	/* start data reading thread */
	ctx->s->hthread = (HANDLE)_beginthreadex(NULL, 0,
		rtlsdr_read_threadproc, ctx, 0, &(ctx->s->threadid));
	if(ctx->s->hthread == NULL)
		return 0;

	/* set started flag */
	ctx->s->is_started = 1;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* stop data reading */
static void rtlsdr_stop(rtlsdr_ctx_t *ctx)
{
	/* check device is opened and reading is started */
	if((ctx->s == NULL) || (!ctx->s->is_started))
		return;

	/* cancel reading */
	if(ctx->s->read_async_entered) {
		ctx->proc->cancel_async(ctx->s->dev);
		ctx->s->read_async_entered = 0;
	}

	/* wait until thread terimnated and close thread handle */
	if(WaitForSingleObject(ctx->s->hthread, RTLSDR_THREAD_WAIT_TIME) != WAIT_OBJECT_0)
		_endthreadex(ctx->s->threadid);
	CloseHandle(ctx->s->hthread);

	/* clear state */
	ctx->s->threadid = 0;
	ctx->s->hthread = NULL;
	ctx->s->error = 0;

	/* reset started flags */
	ctx->s->is_started = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* open device */
static int rtlsdr_open(rtlsdr_ctx_t *ctx, HANDLE event, TCHAR *errbuf, size_t errbufsize)
{
	int res;
	rtlsdr_state_t *s;

	_tcscpy(errbuf, _T(""));

	/* check device is selected */
	if((ctx->deviceindex < 0) || (ctx->deviceindex >= ctx->devicecount))
	{
		_sntprintf(errbuf, errbufsize, _T("RTL-SDR device is not selected."));
		return 0;
	}

	/* check device is valid */
	if(!ctx->devicedata[ctx->deviceindex].is_valid)
	{
		_sntprintf(errbuf, errbufsize, _T("RTL-SDR device is not enumerated correctly."));
		return 0;
	}

	/* alloc state memory */
	if( (s = calloc(1, sizeof(rtlsdr_state_t))) != NULL )
	{
		/* alloc resources */
		InitializeCriticalSection(&(s->buf_csec));

		/* initialize structure */
		s->openeddeviceindex = ctx->deviceindex;
		s->devinfo = &(ctx->devicedata[ctx->deviceindex].info);
		s->devcfg = &(ctx->devicedata[ctx->deviceindex].cfg);
		s->buf_event = event;

		/* open device */
		res = ctx->proc->open(&(s->dev), s->openeddeviceindex);

		if(res != 0)
		{
			_sntprintf(errbuf, errbufsize, _T("Can't open RTL-SDR device (%d)."), res);
		}
		else
		{
			/* configure device */
			rtlsdr_checkfc(&(s->devcfg->fc), s->devinfo->tuner_type);
			if(rtlsdr_setconfig(ctx, s->devcfg, s->dev, s->devinfo, errbuf, errbufsize))
			{
				/* set state data pointer */
				ctx->s = s;

				/* notify configuration window */
				rtlwnd_setstarted(ctx->cfgwnd, TRUE);

				return 1;
			}

			/* close device */
			ctx->proc->close(s->dev);
		}

		/* cleanup */
		DeleteCriticalSection(&(s->buf_csec));
		free(s);
	}

	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize, _T("Can't open RTL-SDR device."));
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* close device */
static void rtlsdr_close(rtlsdr_ctx_t *ctx)
{
	/* check device currently opened */
	if(ctx->s == NULL)
		return;

	/* stop data reading */
	rtlsdr_stop(ctx);

	/* close device */
	ctx->proc->close(ctx->s->dev);

	/* cleanup */
	DeleteCriticalSection(&(ctx->s->buf_csec));
	free(ctx->s);

	/* reset state data pointer */
	ctx->s = NULL;

	/* notify configuration window */
	rtlwnd_setstarted(ctx->cfgwnd, FALSE);
}

/* ---------------------------------------------------------------------------------------------- */

/* reset buffer queue */
static void rtlsdr_reset_bufs(rtlsdr_ctx_t *ctx)
{
	rtlsdr_state_t *s = ctx->s;

	if(s != NULL)
	{
		EnterCriticalSection(&(s->buf_csec));
		s->buf_last_fill = NULL;
		s->buf_next_fill = NULL;
		s->buf_last_out = NULL;
		s->buf_next_out = NULL;
		LeaveCriticalSection(&(s->buf_csec));
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* add buffers to filling queue */
static void rtlsdr_add_bufs(rtlsdr_ctx_t *ctx, input_buf_t *bufs, size_t count)
{
	rtlsdr_state_t *s = ctx->s;
	input_buf_t *buf;

	if(s != NULL)
	{
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
}

/* ---------------------------------------------------------------------------------------------- */

/* remove buffers from return queue */
static input_buf_t *rtlsdr_remove_buf(rtlsdr_ctx_t *ctx)
{
	rtlsdr_state_t *s = ctx->s;
	input_buf_t *buf = NULL;

	if(s != NULL)
	{
		EnterCriticalSection(&(s->buf_csec));
		if(s->buf_next_out != NULL) {
			buf = s->buf_next_out;
			s->buf_next_out = buf->next;
			if(s->buf_next_out == NULL)
				s->buf_last_out = NULL;
		}
		LeaveCriticalSection(&(s->buf_csec));
	}

	return buf;
}

/* ---------------------------------------------------------------------------------------------- */

/* check for stream errors */
static int rtlsdr_check_error(rtlsdr_ctx_t *ctx)
{
	/* check device opened */
	if(ctx->s == NULL)
		return 0;

	return ctx->s->error;
}

/* ---------------------------------------------------------------------------------------------- */

/* get sampling rate */
static unsigned int rtlsdr_get_fs(rtlsdr_ctx_t *ctx)
{
	/* check device opened */
	if(ctx->s == NULL)
		return 0;

	return ctx->s->devcfg->fs;
}

/* ---------------------------------------------------------------------------------------------- */

/* get center frequency */
static int rtlsdr_get_fc(rtlsdr_ctx_t *ctx, double *fcp)
{
	/* check device opened */
	if(ctx->s == NULL)
		return 0;

	*fcp = (double)(ctx->s->devcfg->fc);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set center frequency */
static int rtlsdr_set_fc(rtlsdr_ctx_t *ctx, double *fcp)
{
	double fc_setd;
	unsigned int fc_set;

	/* check device opened */
	if(ctx->s == NULL)
		return 0;

	/* convert center frequency to unsigned int */
	fc_setd = *fcp;
	if(fc_setd < 0.0)
		fc_setd = 0.0;
	if(fc_setd > (double)UINT_MAX)
		fc_setd = (double)UINT_MAX;
	fc_set = (unsigned int)(fc_setd + 0.5);

	/* limit center frequency for tuner range */
	rtlsdr_checkfc(&fc_set, ctx->s->devinfo->tuner_type);

	/* check center frequency changed */
	if(fc_set != ctx->s->devcfg->fc)
	{
		/* set new center frequency */
		if(ctx->proc->set_center_freq(ctx->s->dev, fc_set) != 0)
			return 0;
		ctx->s->devcfg->fc = fc_set;
	}

	/* return center frequency set */
	*fcp = (double)fc_set;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* find all devices */
static int rtlsdr_enumdevices(rtlsdr_ctx_t *ctx)
{
	int index;
	rtlsdr_device_data_t *devdata;

	/* get rtl-sdr device count */
	ctx->devicecount = ctx->proc->get_device_count();
	if(ctx->devicecount <= 0)
		return 1;

	/* allocate buffer for device data */
	ctx->devicedata = calloc(ctx->devicecount, sizeof(rtlsdr_device_data_t));
	if(ctx->devicedata == NULL)
		return 0;

	/* initialize device data */
	for(index = 0; index < ctx->devicecount; index++)
	{
		devdata = ctx->devicedata + index;

		/* set device index */
		devdata->index = index;

		/* set default device configuration */
		rtlsdr_initconfig(&(devdata->cfg));

		/* collect device information */
		if(rtlsdr_getdeviceinfo(ctx, index, &(devdata->info)))
			devdata->is_valid = 1;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* free device list */
static void rtlsdr_freedevicelist(rtlsdr_ctx_t *ctx)
{
	int i;

	for(i = 0; i < ctx->devicecount; i++)
		rtlsdr_freedeviceinfo(&(ctx->devicedata[i].info));
	free(ctx->devicedata);
}

/* ---------------------------------------------------------------------------------------------- */

/* load configuration */
static void rtlsdr_config_load(rtlsdr_ctx_t *ctx, ini_data_t *ini)
{
	int i;
	ini_sect_t *sect;
	TCHAR sectname[16];
	rtlsdr_deviceconfig_t devcfg;

	/* load selected device index */
	sect = ini_sect_get(ini, _T("rtlsdr"), 0);
	ini_getir(sect, _T("device"), &(ctx->deviceindex), -1, ctx->devicecount - 1);

	/* load device configurations */
	for(i = 0; i < ctx->devicecount; i++)
	{
		/* get device configuration section */
		_stprintf(sectname, _T("rtlsdr%d"), i);
		sect = ini_sect_get(ini, sectname, 0);

		if(sect != NULL)
		{
			rtlsdr_initconfig(&devcfg);

			ini_geti(sect, _T("direct_sampling"), &(devcfg.direct_sampling));

			ini_getu(sect, _T("fs"), &(devcfg.fs));
			ini_getu(sect, _T("fc"), &(devcfg.fc));

			ini_getb(sect, _T("offset_tuning"), &(devcfg.offset_tuning_on));
			ini_getb(sect, _T("tuner_gain_manual"), &(devcfg.tuner_gain_manual));
			ini_getb(sect, _T("rtl_agc"), &(devcfg.rtl_agc_on));

			ini_geti(sect, _T("tuner_gain_value"), &(devcfg.tuner_gain_value));
			ini_geti(sect, _T("freqcorr"), &(devcfg.freqcorr));

			ini_geti(sect, _T("buf_count"), &(devcfg.buf_count));
			ini_getf(sect, _T("buf_len_ms"), &(devcfg.buf_len_ms));

			/* check and set device configuration */
			if(rtlsdr_checkconfig(&devcfg, &(ctx->devicedata[i].info)))
				memcpy(&(ctx->devicedata[i].cfg), &devcfg, sizeof(rtlsdr_deviceconfig_t));
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save configuration */
static void rtlsdr_config_save(rtlsdr_ctx_t *ctx, ini_data_t *ini)
{
	int i;
	ini_sect_t *sect;
	TCHAR sectname[16];
	rtlsdr_deviceconfig_t *devcfg;

	/* save selected device index */
	sect = ini_sect_get(ini, _T("rtlsdr"), 1);
	ini_seti(sect, _T("device"), ctx->deviceindex);

	/* save device configurations */
	for(i = 0; i < ctx->devicecount; i++)
	{
		devcfg = &(ctx->devicedata[i].cfg);

		_stprintf(sectname, _T("rtlsdr%d"), i);
		sect = ini_sect_get(ini, sectname, 1);

		ini_seti(sect, _T("direct_sampling"), devcfg->direct_sampling);
		ini_setu(sect, _T("fs"), devcfg->fs);
		ini_setu(sect, _T("fc"), devcfg->fc);
		ini_setb(sect, _T("offset_tuning"), devcfg->offset_tuning_on);
		ini_setb(sect, _T("tuner_gain_manual"), devcfg->tuner_gain_manual);
		ini_setb(sect, _T("rtl_agc"), devcfg->rtl_agc_on);
		ini_seti(sect, _T("tuner_gain_value"), devcfg->tuner_gain_value);
		ini_seti(sect, _T("freqcorr"), devcfg->freqcorr);
		ini_seti(sect, _T("buf_count"), devcfg->buf_count);
		ini_setf(sect, _T("buf_len_ms"), 6, devcfg->buf_len_ms);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* show/hide device configuration window */
static void rtlsdr_config_showwnd(rtlsdr_ctx_t *ctx, uicommon_t *uidata, HWND hwndowner)
{
	if(ctx->cfgwnd != NULL) {
		DestroyWindow(ctx->cfgwnd->hwnd);
	} else {
		rtlwnd_create(uidata, hwndowner, ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize module */
static rtlsdr_ctx_t *rtlsdr_init(input_module_desc_t *desc, TCHAR *errbuf, size_t errbufsize)
{
	rtlsdr_moduledata_t *data;
	rtlsdr_ctx_t *ctx;

	_tcscpy(errbuf, _T(""));

	data = desc->moduledata;

	/* allocate memory */
	if( (ctx = calloc(1, sizeof(rtlsdr_ctx_t))) != NULL )
	{
		/* initialize structure */
		ctx->proc = &(data->proc);
		ctx->deviceindex = -1;

		/* enumerate devices */
		if(rtlsdr_enumdevices(ctx))
			return ctx;

		/* cleanup */
		free(ctx);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* cleanup module */
static void rtlsdr_cleanup(rtlsdr_ctx_t *ctx)
{
	/* destroy configuration window if one */
	if(ctx->cfgwnd != NULL)
		DestroyWindow(ctx->cfgwnd->hwnd);

	/* cleanup */
	rtlsdr_freedevicelist(ctx);
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

/* free module description */
static void rtlsdr_cleanup_desc(input_module_desc_t *desc)
{
	rtlsdr_moduledata_t *data;

	data = desc->moduledata;

	/* unload library */
	FreeLibrary(data->hmod);

	/* cleanup */
	free(data);
	free(desc);
}

/* ---------------------------------------------------------------------------------------------- */

/* import functions addresses from dll */
static int rtlsdr_getprocaddresses(HMODULE hmod, rtlsdr_proc_t *proc)
{
	proc->get_device_count = (rtlsdr_get_device_count_t)GetProcAddress(hmod, "rtlsdr_get_device_count");
	proc->get_device_name = (rtlsdr_get_device_name_t)GetProcAddress(hmod, "rtlsdr_get_device_name");
	proc->open = (rtlsdr_open_t)GetProcAddress(hmod, "rtlsdr_open");
	proc->close = (rtlsdr_close_t)GetProcAddress(hmod, "rtlsdr_close");
	proc->set_center_freq = (rtlsdr_set_center_freq_t)GetProcAddress(hmod, "rtlsdr_set_center_freq");
	proc->get_center_freq = (rtlsdr_get_center_freq_t)GetProcAddress(hmod, "rtlsdr_get_center_freq");
	proc->set_freq_correction = (rtlsdr_set_freq_correction_t)GetProcAddress(hmod, "rtlsdr_set_freq_correction");
	proc->get_freq_correction = (rtlsdr_get_freq_correction_t)GetProcAddress(hmod, "rtlsdr_get_freq_correction");
	proc->get_tuner_type = (rtlsdr_get_tuner_type_t)GetProcAddress(hmod, "rtlsdr_get_tuner_type");
	proc->get_tuner_gains = (rtlsdr_get_tuner_gains_t)GetProcAddress(hmod, "rtlsdr_get_tuner_gains");
	proc->set_tuner_gain = (rtlsdr_set_tuner_gain_t)GetProcAddress(hmod, "rtlsdr_set_tuner_gain");
	proc->get_tuner_gain = (rtlsdr_get_tuner_gain_t)GetProcAddress(hmod, "rtlsdr_get_tuner_gain");
	proc->set_tuner_gain_mode = (rtlsdr_set_tuner_gain_mode_t)GetProcAddress(hmod, "rtlsdr_set_tuner_gain_mode");
	proc->set_sample_rate = (rtlsdr_set_sample_rate_t)GetProcAddress(hmod, "rtlsdr_set_sample_rate");
	proc->get_sample_rate = (rtlsdr_get_sample_rate_t)GetProcAddress(hmod, "rtlsdr_get_sample_rate");
	proc->set_agc_mode = (rtlsdr_set_agc_mode_t)GetProcAddress(hmod, "rtlsdr_set_agc_mode");
	proc->set_direct_sampling = (rtlsdr_set_direct_sampling_t)GetProcAddress(hmod, "rtlsdr_set_direct_sampling");
	proc->get_direct_sampling = (rtlsdr_get_direct_sampling_t)GetProcAddress(hmod, "rtlsdr_get_direct_sampling");
	proc->set_offset_tuning = (rtlsdr_set_offset_tuning_t)GetProcAddress(hmod, "rtlsdr_set_offset_tuning");
	proc->get_offset_tuning = (rtlsdr_get_offset_tuning_t)GetProcAddress(hmod, "rtlsdr_get_offset_tuning");
	proc->reset_buffer = (rtlsdr_reset_buffer_t)GetProcAddress(hmod, "rtlsdr_reset_buffer");
	proc->read_async = (rtlsdr_read_async_t)GetProcAddress(hmod, "rtlsdr_read_async");
	proc->cancel_async = (rtlsdr_cancel_async_t)GetProcAddress(hmod, "rtlsdr_cancel_async");

	if( (proc->get_device_count == NULL) ||
		(proc->get_device_name == NULL) ||
		(proc->open == NULL) ||
		(proc->close == NULL) ||
		(proc->set_center_freq == NULL) ||
		(proc->get_center_freq == NULL) ||
		(proc->set_freq_correction == NULL) ||
		(proc->get_freq_correction == NULL) ||
		(proc->get_tuner_type == NULL) ||
		(proc->get_tuner_gains == NULL) ||
		(proc->set_tuner_gain == NULL) ||
		(proc->get_tuner_gain == NULL) ||
		(proc->set_tuner_gain_mode == NULL) ||
		(proc->set_sample_rate == NULL) ||
		(proc->get_sample_rate == NULL) ||
		(proc->set_agc_mode == NULL) ||
		(proc->set_direct_sampling == NULL) ||
		(proc->get_direct_sampling == NULL) ||
		(proc->set_offset_tuning == NULL) ||
		(proc->get_offset_tuning == NULL) ||
		(proc->reset_buffer == NULL) ||
		(proc->read_async == NULL) ||
		(proc->cancel_async == NULL) )
	{
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static HMODULE rtlsdr_loadmodule()
{
	TCHAR *pathbuf;
	HMODULE hmodule = NULL;

	/* allocate path buffer*/
	if( (pathbuf = malloc(MAX_PATH * sizeof(TCHAR))) == NULL )
		return NULL;

	/* check for rtlsdr directory */
	if(!get_local_path(pathbuf, MAX_PATH, RTLSDR_DIRECTORY))
		goto __cleanup_and_exit;
	if(GetFileAttributes(pathbuf) & FILE_ATTRIBUTE_DIRECTORY)
	{
		/* append rtlsdr directory to path variable for associated dll loading */
		if(!append_path_env(pathbuf))
			goto __cleanup_and_exit;

		/* try loading module from rtlsdr directory */
		if(!get_local_path(pathbuf, MAX_PATH, RTLSDR_DIRECTORY _T("\\") RTLSDR_MODULENAME))
			goto __cleanup_and_exit;
		hmodule = LoadLibrary(pathbuf);
	}
	else
	{
		/* try standard module search */
		hmodule = LoadLibrary(RTLSDR_MODULENAME);
	}

	free(pathbuf);
	return hmodule;

__cleanup_and_exit:
	free(pathbuf);
	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize rtl-sdr module description */
input_module_desc_t *rtlsdr_load()
{
	input_module_desc_t *desc;
	rtlsdr_moduledata_t *data;

	/* allocate memory */
	desc = calloc(1, sizeof(input_module_desc_t));
	data = calloc(1, sizeof(rtlsdr_moduledata_t));

	if( (desc != NULL) && (data != NULL) )
	{
		desc->moduledata = data;

		/* load rtl-sdr library */
		data->hmod = rtlsdr_loadmodule();

		if(data->hmod != NULL)
		{
			/* import functions */
			if(rtlsdr_getprocaddresses(data->hmod, &(data->proc)))
			{
				desc->name = _T("rtlsdr");
				desc->display_name = _T("RTL-SDR");

				desc->fn_open = rtlsdr_open;
				desc->fn_close = rtlsdr_close;
				desc->fn_start = rtlsdr_start;
				desc->fn_stop = rtlsdr_stop;

				desc->fn_reset_bufs = rtlsdr_reset_bufs;
				desc->fn_add_bufs = rtlsdr_add_bufs;
				desc->fn_remove_buf = rtlsdr_remove_buf;
				desc->fn_check_err = rtlsdr_check_error;

				desc->fn_get_fs = rtlsdr_get_fs;
				desc->fn_get_fc = rtlsdr_get_fc;
				desc->fn_set_fc = rtlsdr_set_fc;

				desc->fn_init = rtlsdr_init;
				desc->fn_cleanup = rtlsdr_cleanup;
				desc->fn_config_load = rtlsdr_config_load;
				desc->fn_config_save = rtlsdr_config_save;
				desc->fn_config_showdialog = rtlsdr_config_showwnd;

				desc->fn_cleanup_desc = rtlsdr_cleanup_desc;

				return desc;
			}

			FreeLibrary(data->hmod);
		}
	}

	/* cleanup */
	free(data);
	free(desc);

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */
