/* ---------------------------------------------------------------------------------------------- */

#include "rxprocch.h"

/* ---------------------------------------------------------------------------------------------- */

/* update oscillator section */
static void rxproc_osc_update(rxproc_t *proc)
{
	double fshift;

	fshift = -(proc->cfg.fc - proc->fc_input);
	osc_set_freq(&(proc->osc), fshift / proc->fs_input);
}

/* ---------------------------------------------------------------------------------------------- */

/* deinit decimator */
static void rxproc_decim_deinit(rxproc_t *proc)
{
	if(proc->decim_inited)
	{
		if(proc->decimf != 1)
		{
			decim_cleanup(&(proc->decim));
		}
		proc->decim_inited = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* init decimator */
static int rxproc_decim_init(rxproc_t *proc, TCHAR *errbuf, size_t errbufsize)
{
	double decim_fcf;

	_tcscpy(errbuf, _T(""));

	if(!proc->decim_inited)
	{
		if(proc->decimf != 1)
		{
			decim_fcf = 0.5 - 0.5 * proc->cfg.decim_dff;

			if( !decim_init(&(proc->decim), proc->decim_comb_decimf, proc->decim_compf_decimf,
				decim_fcf, proc->cfg.decim_dff, proc->cfg.decim_as, proc->cfg.decim_frgran) )
			{
				_sntprintf(errbuf, errbufsize,
					_T("Can't initialize input decimator for channel '%s' ")
					_T("(comb_decimf = %u, compf_decimf = %u, fcf = %.4f of fs_input, ")
					_T("dff = %.4f of fs_input, as = %.2f dB, frgran = %u samples)."),
					proc->cfg.name, proc->decim_comb_decimf, proc->decim_compf_decimf, decim_fcf,
					proc->cfg.decim_dff, proc->cfg.filter_as, proc->cfg.decim_frgran);
				return 0;
			}
		}

		proc->decim_inited = 1;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* deinit baseband filter */
static void rxproc_filter_deinit(rxproc_t *proc)
{
	if(proc->filter_inited)
	{
		firfilt_cleanup(&(proc->filter));
		proc->filter_inited = 0;
		proc->filter_tuned = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* reinit/tune baseband filter */
static int rxproc_filter_update(rxproc_t *proc, TCHAR *errbuf, size_t errbufsize)
{
	size_t len;
	double fcf, dff;
	float *h;

	_tcscpy(errbuf, _T(""));

	/* check transition bandwidth */
	dff = proc->cfg.filter_df / proc->fs_base;
	if(dff > 0.45)
		dff = 0.45;

	/* check cutoff frequency */
	fcf = proc->cfg.filter_fc / proc->fs_base;
	if(fcf > 0.5 - 0.5 * dff)
		fcf = 0.5 - 0.5 * dff;

	/* reinit filter */
	if(!proc->filter_inited)
	{
		len = firdes_kaiser_length(proc->cfg.filter_as, dff);

		if( !firfilt_init(&(proc->filter), len, 1, 1) ) {
			_sntprintf(errbuf, errbufsize,
				_T("Can't initialize baseband filter for channel '%s' ")
				_T("(as = %.2f db, df = %.2f Hz (%.4f of fs_input), length = %Iu)."),
				proc->cfg.name, proc->cfg.filter_as, proc->cfg.filter_df, dff, len);
			return 0;
		}

		proc->filter_inited = 1;
	}

	/* tune filter */
	if(!proc->filter_tuned)
	{
		if( (h = malloc( proc->filter.length * sizeof(float) )) == NULL )
			return 0;
		firdes_windowsinc(h, proc->filter.length, fcf,
			WND_KAISER, firdes_kaiser_beta(proc->cfg.filter_as));
		firfilt_set_coefs(&(proc->filter), h);
		free(h);

		proc->filter_tuned = 1;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* execute demodulator */
static void rxproc_demod_exec(rxproc_t *proc, float *dst, cpxf_t *src, size_t nsamp)
{
	switch(proc->cfg.demod_type)
	{
	case RXPROC_DEMOD_AM:
		demod_am_process(dst, src, nsamp);
		break;
	case RXPROC_DEMOD_FM:
		rxproc_fm_process(&(proc->demod.fm), dst, src, nsamp);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* deinit demodulator */
static void rxproc_demod_deinit(rxproc_t *proc)
{
	if(proc->demod_inited)
	{
		/*switch(proc->cfg.demod_type)
		{
		case RXPROC_DEMOD_FM:
			break;
		case RXPROC_DEMOD_AM:
			break;
		}*/

		proc->demod_inited = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize demodulator */
static int rxproc_demod_init(rxproc_t *proc, TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	_tcscpy(errbuf, _T(""));

	if(!proc->demod_inited)
	{
		switch(proc->cfg.demod_type)
		{
		case RXPROC_DEMOD_FM:
			status = rxproc_fm_init(&(proc->demod.fm), &(proc->cfg.fmdemod), proc->fs_base);
			if(!status) {
				_sntprintf(errbuf, errbufsize,
					_T("Can't initialize FM demodulator for channel '%s' ")
					_T("(fs_base = %u Hz, df = %.2f Hz)."),
					proc->cfg.name, proc->fs_base, proc->cfg.fmdemod.df);
			}
			break;
		}

		if(!status)
			return 0;

		proc->demod_inited = 1;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* deinitialize current squelch */
static void rxproc_sql_deinit(rxproc_t *proc)
{
	if(proc->sql_inited)
	{
		levelmtr_free(&(proc->sql_sense_meter));
		rxsql_cleanup(&(proc->sql));
		proc->sql_inited = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize squelch of selected type */
static int rxproc_sql_init(rxproc_t *proc, rxconfig_t *cfg, TCHAR *errbuf, size_t errbufsize)
{
	_tcscpy(errbuf, _T(""));

	if(proc->sql_inited)
	{
		_sntprintf(errbuf, errbufsize,
			_T("Squelch for channel '%s' is already initialized."),
			proc->cfg.name);
		return 0;
	}

	if(!rxsql_init(&(proc->sql), &(proc->cfg.sql),
		proc->fs_input, proc->fs_base, proc->cfg.name, errbuf, errbufsize))
	{
		return 0;
	}

	if(!levelmtr_init_fs(&(proc->sql_sense_meter), 
		1, cfg->proc_level_num_points, proc->fs_base, 
		0.001, cfg->proc_level_update_int))
	{
		rxsql_cleanup(&(proc->sql));
		_sntprintf(errbuf, errbufsize,
			_T("Can't initialize squelch sense level meter for channel '%s'."),
			proc->cfg.name);
		return 0;
	}

	proc->sql_inited = 1;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int rxproc_levelmtr_init(rxproc_t *proc, rxconfig_t *cfg, TCHAR *errbuf, size_t errbufsize)
{
	if( (!INRANGE_MINMAX(cfg->proc_level_num_points, RXLIM_PROC_LEVEL_NUM_POINTS)) ||
		(!INRANGE_MINMAX(cfg->proc_level_update_int, RXLIM_PROC_LEVEL_UPDATE_INT)) )
	{
		_sntprintf(errbuf, errbufsize, _T("Bad level meter configuration."));
		return 0;
	}

	if(proc->levelmtr_inited)
	{
		_sntprintf(errbuf, errbufsize,
			_T("Level meter for channel '%s' is already initialized."),
			proc->cfg.name);
		return 0;
	}

	if(!levelmtr_init_fs(&(proc->levelmtr), 1,
		cfg->proc_level_num_points, proc->fs_base, 0.001,
		cfg->proc_level_update_int))
	{
		_sntprintf(errbuf, errbufsize,
			_T("Can't initialize level meter for channel '%s' ")
			_T("(num_points: %Iu, fs: %u, update_int: %.2f)."),
			proc->cfg.name,
			cfg->proc_level_num_points, proc->fs_base, cfg->proc_level_update_int);
		return 0;
	}

	proc->levelmtr_inited = 1;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void rxproc_levelmtr_deinit(rxproc_t *proc)
{
	if(proc->levelmtr_inited)
	{
		levelmtr_free(&(proc->levelmtr));
		proc->levelmtr_inited = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* set channel center frequency */
/*     fc: channel center frequency, Hz */
int rxproc_set_fc(rxproc_t *proc, double fc)
{
	/* check parameters */
	if(!INRANGE_MINMAX(fc, RXLIM_FC))
		return 0;

	/* if steam is not started, save parameters and exit */
	if(!proc->is_started) {
		proc->cfg.fc = fc;
		return 1;
	}

	/* need to update oscilator? */
	if(fabs(fc - proc->cfg.fc) >= 1e-3)
	{
		EnterCriticalSection(&(proc->cs_proc));
		proc->cfg.fc = fc;
		rxproc_osc_update(proc);
		LeaveCriticalSection(&(proc->cs_proc));
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set input center frequency */
/*     fc_input: input center frequency, Hz */
int rxproc_set_fc_input(rxproc_t *proc, double fc_input)
{
	/* check parameters */
	if(!INRANGE_MINMAX(fc_input, RXLIM_FC))
		return 0;

	/* if steam is not started, save parameters and exit */
	if(!proc->is_started) {
		proc->fc_input = fc_input;
		return 1;
	}

	/* need to update oscillator? */
	if(fabs(fc_input - proc->fc_input) >= 1e-3)
	{
		EnterCriticalSection(&(proc->cs_proc));
		proc->fc_input = fc_input;
		rxproc_osc_update(proc);
		LeaveCriticalSection(&(proc->cs_proc));
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set demodulator type */
/*     demod_type: new demodulator type */
int rxproc_set_demod_type(rxproc_t *proc, rxproc_demodtype_t demod_type,
						  TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	_tcscpy(errbuf, _T(""));

	/* check demodulator type */
	if((demod_type < 0) || (demod_type >= RXPROC_DEMOD_COUNT)) {
		_sntprintf(errbuf, errbufsize,
			_T("Bad demodulator type for channel '%s'."), proc->cfg.name);
		return 0;
	}

	/* if steam is not started, save parameters and exit */
	if(!proc->is_started) {
		proc->cfg.demod_type = demod_type;
		return 1;
	}

	/* need to update demodulator? */
	if((!proc->demod_inited) || (demod_type != proc->cfg.demod_type))
	{
		EnterCriticalSection(&(proc->cs_proc));

		/* deinit previous demodulator */
		rxproc_demod_deinit(proc);

		/* init new demodulator */
		proc->cfg.demod_type = demod_type;
		if(!rxproc_demod_init(proc, errbuf, errbufsize))
			status = 0;

		LeaveCriticalSection(&(proc->cs_proc));
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* set decimator parameters */
/*     frgran: frequency response granularity, samples
 *     mcsdf: minimum compensation stage decimation factor
 *     dff: transition bandwidth, fractional
 *     as: stopband attenuation, dB */
int rxproc_set_decim_params(rxproc_t *proc, unsigned int frgran, unsigned int mcsdf,
							double dff, double as, TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	_tcscpy(errbuf, _T(""));

	/* check parameters */
	if( (!INRANGE_MINMAX(frgran, RXLIM_PROC_DECIM_FRGRAN)) ||
		(!INRANGE_MINMAX(mcsdf, RXLIM_PROC_DECIM_MCSDF)) ||
		(!INRANGE_MINMAX(dff, RXLIM_PROC_DECIM_DFF)) ||
		(!INRANGE_MINMAX(as, RXLIM_PROC_DECIM_AS)) )
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad decimator configuration for channel '%s'."), proc->cfg.name);
		return 0;
	}

	/* if steam is not started, only save params and exit */
	if(!proc->is_started) {
		proc->cfg.decim_frgran = frgran;
		proc->cfg.decim_mcsdf = mcsdf;
		proc->cfg.decim_dff = dff;
		proc->cfg.decim_as = as;
		return 1;
	}

	/* need to update decimator? */
	if( (!proc->decim_inited) ||
		(frgran != proc->cfg.decim_frgran) ||
		(mcsdf != proc->cfg.decim_mcsdf) ||
		(fabs(dff - proc->cfg.decim_dff) >= 1e-6) ||
		(fabs(as - proc->cfg.decim_as) >= 1e-6) )
	{
		EnterCriticalSection(&(proc->cs_proc));

		/* deinit decimator */
		rxproc_decim_deinit(proc);

		/* set new parameters */
		proc->cfg.decim_frgran = frgran;
		proc->cfg.decim_mcsdf = mcsdf;
		proc->cfg.decim_dff = dff;
		proc->cfg.decim_as = as;

		/* reinit decimator */
		status = rxproc_decim_init(proc, errbuf, errbufsize);

		LeaveCriticalSection(&(proc->cs_proc));
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* set baseband filter parameters */
/*     fc : cutoff frequency, Hz
 *     df : transition bandwidth, Hz,
 *     as : stopband attenuation, dB */
int rxproc_set_filter_params(rxproc_t *proc, double fc, double df, double as,
							 TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	_tcscpy(errbuf, _T(""));

	/* check parameters */
	if( (!INRANGE_MINMAX(fc, RXLIM_PROC_FILTER_FC)) ||
		(!INRANGE_MINMAX(df, RXLIM_PROC_FILTER_DF)) ||
		(!INRANGE_MINMAX(as, RXLIM_PROC_FILTER_AS)) )
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad baseband filter configuration for channel '%s'"), proc->cfg.name);
		return 0;
	}

	/* if steam is not started, only save params and exit */
	if(!proc->is_started)
	{
		proc->cfg.filter_fc = fc;
		proc->cfg.filter_df = df;
		proc->cfg.filter_as = as;
		return 1;
	}

	/* need to change parameters? */
	if( (!proc->filter_inited) || (!proc->filter_tuned) ||
		(fabs(fc - proc->cfg.filter_fc) >= 1e-3) ||
		(fabs(df - proc->cfg.filter_df) >= 1e-3) ||
		(fabs(as - proc->cfg.filter_as) >= 1e-6)  )
	{
		EnterCriticalSection(&(proc->cs_proc));

		/* check for filter reinit */
		if( (fabs(df - proc->cfg.filter_df) >= 1e-3) ||
			(fabs(as - proc->cfg.filter_as) >= 1e-6)  )
		{
			rxproc_filter_deinit(proc);
			proc->cfg.filter_df = df;
			proc->cfg.filter_as = as;
		}

		/* check for filter retune */
		if(fabs(fc - proc->cfg.filter_fc) >= 1e-3)
		{
			proc->filter_tuned = 0;
			proc->cfg.filter_fc = fc;
		}

		status = rxproc_filter_update(proc, errbuf, errbufsize);

		LeaveCriticalSection(&(proc->cs_proc));
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* set FM demodulator parameters
 *	df : frequency deviation, Hz. */
int rxproc_set_fm_demod_params(rxproc_t *proc, double df, TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	_tcscpy(errbuf, _T(""));

	/* check parameters */
	if(!INRANGE_MINMAX(df, RXPROC_FM_DF))
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad FM demodulator configuration for channel '%s'"), proc->cfg.name);
		return 0;
	}

	/* if demodulator is not used, only save params and exit */
	if((!proc->is_started) || (proc->cfg.demod_type != RXPROC_DEMOD_FM))
	{
		proc->cfg.fmdemod.df = df;
		return 1;
	}

	/* is configuring actually needed? */
	if(fabs(df - proc->cfg.fmdemod.df) >= 1e-3)
	{
		EnterCriticalSection(&(proc->cs_proc));

		/* configure demodulator */
		proc->cfg.fmdemod.df = df;
		status = rxproc_fm_configure(&(proc->demod.fm), df);

		LeaveCriticalSection(&(proc->cs_proc));

		if(status == 0) {
			_sntprintf(errbuf, errbufsize,
				_T("Can't configure FM demodulator for channel '%s'"), proc->cfg.name);
		}
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

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
						  TCHAR *errbuf, size_t errbufsize)
{
	int status = 1;

	/* check parameters */
	if( (!INRANGE_MINMAX(bw, RXSQL_BW)) ||
		(!INRANGE_MINMAX(envef_param, RXSQL_ENVEF_PARAM)) ||
		(!INRANGE_MINMAX(op_thres, RXSQL_THRES_DB)) ||
		(!INRANGE_MINMAX(cl_thres, RXSQL_THRES_DB)) ||
		(!INRANGE_MINMAX(op_dly, RXSQL_DLY_MS)) ||
		(!INRANGE_MINMAX(cl_dly, RXSQL_DLY_MS)) ||
		(cl_thres > op_thres) )
	{
		_sntprintf(errbuf, errbufsize,
			_T("Bad squelch configuration for channel '%s'."), proc->cfg.name);
		return 0;
	}

	/* if squelch is initialized, only save params and exit */
	if(!proc->sql_inited)
	{
		proc->cfg.sql.use_carr_filter = use_carr_filter;
		proc->cfg.sql.bw = bw;
		proc->cfg.sql.envef_param = envef_param;
		proc->cfg.sql.op_thres_db = op_thres;
		proc->cfg.sql.cl_thres_db = cl_thres;
		proc->cfg.sql.op_dly_ms = op_dly;
		proc->cfg.sql.cl_dly_ms = cl_dly;
		return 1;
	}

	/* reconfigure squelch */
	EnterCriticalSection(&(proc->cs_proc));

	status = rxsql_configure(&(proc->sql), use_carr_filter, bw, envef_param,
		op_thres, cl_thres, op_dly, cl_dly, proc->cfg.name, errbuf, errbufsize);

	LeaveCriticalSection(&(proc->cs_proc));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* set DC offset remover filter alpha coefficient */
int rxproc_set_output_dcrem_alpha(rxproc_t *proc, double output_dcrem_alpha)
{
	/* check parameters */
	if(!INRANGE_MINMAX(output_dcrem_alpha, RXLIM_PROC_DCREM_ALPHA))
		return 0;

	/* if steam is not started, only save parameters and exit */
	if(!proc->is_started) {
		proc->cfg.output_dcrem_alpha = output_dcrem_alpha;
		return 1;
	}

	/* check parameter changed */
	if(fabs(output_dcrem_alpha - proc->cfg.output_dcrem_alpha) >= 1e-6)
	{
		EnterCriticalSection(&(proc->cs_proc));

		proc->cfg.output_dcrem_alpha = output_dcrem_alpha;
		iir_dcrem_init(&(proc->dcrem_state), output_dcrem_alpha);

		LeaveCriticalSection(&(proc->cs_proc));
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set audio output gain */
int rxproc_set_output_gain(rxproc_t *proc, double gain)
{
	/* check parameters */
	if(!INRANGE_MINMAX(gain, RXLIM_AUDIO_GAIN))
		return 0;

	/* if steam is not started, save parameters and exit */
	if(!proc->is_started) {
		proc->cfg.output_gain = gain;
		return 1;
	}

	/* check parameters changed */
	if(fabs(gain - proc->cfg.output_gain) >= 1e-6)
	{
		EnterCriticalSection(&(proc->cs_proc));

		proc->cfg.output_gain = gain;
		sndmix_input_gain(proc->audio_sink->in, proc->cfg.output_gain);

		LeaveCriticalSection(&(proc->cs_proc));
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* change channel name */
int rxproc_set_name(rxproc_t *proc, TCHAR *name)
{
	/* check parameters */
	if(_tcslen(name) == 0)
		return 0;

	/* copy new name to channel configuration */
	_tcsncpy(proc->cfg.name, name, RXLIM_PROC_MAXNAME);
	proc->cfg.name[RXLIM_PROC_MAXNAME - 1] = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* read level meter */
int rxproc_read_level(rxproc_t *proc, unsigned int tick, double *plevel)
{
	int status = 0;

	if(proc->is_started && proc->levelmtr_inited)
		status = levelmtr_read(&(proc->levelmtr), tick, plevel);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* read squelch sense meter */
int rxproc_read_sql_sense(rxproc_t *proc, unsigned int tick, double *psense)
{
	int status = 0;

	if(proc->is_started && proc->sql_inited) {
		status = levelmtr_read(&(proc->sql_sense_meter), tick, psense);
	}
	
	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* check modules for error */
int rxproc_check_error(rxproc_t *proc)
{
	/* check decimator */
	if(!proc->decim_inited)
		return 1;

	/* check filter */
	if(!proc->filter_tuned)
		return 1;

	/* check demodulator */
	if(!proc->demod_inited)
		return 1;

	/* check squelch */
	if( (!proc->sql_inited) ||
		(!rxsql_check(&(proc->sql))) )
	{
		return 1;
	}

	/* check level meter */
	if(!proc->levelmtr_inited)
		return 1;

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* check frequency range */
static int rxproc_check_range(rxproc_t *proc)
{
	double df_up, df_dn;

	df_up = proc->cfg.fc - proc->fc_input + proc->cfg.filter_fc;
	if(df_up > 0.5 * proc->fs_input)
		return 1;

	df_dn = proc->cfg.fc - proc->fc_input - proc->cfg.filter_fc;
	if(df_dn < -0.5 * proc->fs_input)
		return -1;

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

rxproc_status_t rxproc_status_query(rxproc_t *proc)
{
	int res;
	if(!proc->is_started)
		return RXPROC_STOPPED;

	if(rxproc_check_error(proc))
		return RXPROC_ERROR;

	res = rxproc_check_range(proc);
	if(res > 0) return RXPROC_RANGEUP;
	if(res < 0) return RXPROC_RANGEDOWN;

	return (proc->out_act_stat ? RXPROC_SQLOPENED : RXPROC_SQLCLOSED);
}

/* ---------------------------------------------------------------------------------------------- */

/* process data */
static void rxproc_process_data(rxproc_t *proc)
{
	size_t nsamp_in, nsamp_base, i;
	size_t nsamp_est_delay;
	unsigned int est_tick_delay, est_out_tick;

	for( ; ; )
	{
		nsamp_in = buf_read(&(proc->buf_input), proc->work_buf, proc->work_buf_len);
		if(nsamp_in == 0)
			break;

		EnterCriticalSection(&(proc->cs_proc));

		/* check configuration */
		if((!rxproc_check_error(proc)) && (!rxproc_check_range(proc)))
		{
			/* frequency shift */
			if( fabs(proc->cfg.fc - proc->fc_input) >= 1e-3 ) {
				osc_mix(&(proc->osc), proc->work_buf, proc->work_buf, nsamp_in);
			}

			/* decimate input data to baseband frequency */
			if(proc->decimf != 1) {
				nsamp_base = decim_exec(&(proc->decim), proc->work_buf, nsamp_in);
			} else {
				nsamp_base = nsamp_in;
			}

			/* filter baseband data */
			firfilt_exec(&(proc->filter), proc->work_buf, nsamp_base);

			/* process data with squelch (data is delayed!) */
			rxsql_process(&(proc->sql), proc->work_buf,
				proc->sql_sense_buf, proc->sql_mute_buf, nsamp_base);

			/* demodulate baseband data */
			rxproc_demod_exec(proc, proc->audio_buf, proc->work_buf, nsamp_base);

			/* remove DC offset */
			iirfilt_process1(&(proc->dcrem_state),
				proc->audio_buf, proc->audio_buf, nsamp_base);

			for(i = 0; i < nsamp_base; i++)
			{
				if(proc->sql_mute_buf[i])
				{
					/* mute sample */
					proc->audio_buf[i] = 0.0f;

					/* update activity status */
					if(proc->out_act_stat)
					{
						proc->out_act_extctr++;
						if(proc->out_act_extctr >= proc->out_act_extcnt) {
							proc->out_act_stat = 0;
							proc->act_cb(proc->act_ctx, proc, 0);
						}
					}
				}
				else
				{
					/* set activity status */
					if(!proc->out_act_stat)
					{
						proc->out_act_stat = 1;
						proc->act_cb(proc->act_ctx, proc, 1);
					}
					proc->out_act_extctr = 0;
				}
			}

			/* send data to audio output */
			rx_audio_output_writesink(proc->audio_sink,
				proc->audio_buf, nsamp_base, proc->cfg.output_mode, &nsamp_est_delay);
			est_tick_delay = (unsigned int)(1000U * nsamp_est_delay / proc->fs_base);
			est_out_tick = GetTickCount() + est_tick_delay;

			/* send data to level meters */
			levelmtr_write(&(proc->sql_sense_meter), proc->sql_sense_buf, 0, nsamp_base, est_out_tick);
			levelmtr_write(&(proc->levelmtr), proc->work_buf, 1, nsamp_base, est_out_tick);
		}

		LeaveCriticalSection(&(proc->cs_proc));
	}
}

/* ---------------------------------------------------------------------------------------------- */

enum {
	RXPROC_EVT_STOP,
	RXPROC_EVT_DATA,

	RXPROC_EVT_COUNT
};

static unsigned int __stdcall rxproc_threadproc(rxproc_t *proc)
{
	DWORD res;
	HANDLE events[RXPROC_EVT_COUNT];

	events[RXPROC_EVT_STOP] = proc->evt_proc_stop;
	events[RXPROC_EVT_DATA] = proc->evt_input;

	for( ; ; )
	{
		res = WaitForMultipleObjects(RXPROC_EVT_COUNT, events, FALSE, INFINITE);

		if(res == RXPROC_EVT_STOP)
			break;

		if(res == RXPROC_EVT_DATA)
			rxproc_process_data(proc);

	}

	return 0;
}

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
				 rxconfig_t *config)
{
	unsigned int tid;
	size_t buflen, bufwh;

	_tcscpy(errbuf, _T(""));

	/* check channel is not started */
	if(proc->is_started)
	{
		_sntprintf(errbuf, errbufsize,
			_T("Channel '%s' is already started."), proc->cfg.name);
		return 0;
	}

	/* initialize sampling rates */
	proc->fs_input = fs_input;
	proc->fs_base = fs_base;
	proc->decimf = fs_input / fs_base;

	/* initialize working buffer */
	proc->work_buf_len = MSECS2NSAMP(config->proc_workbuf_ms, fs_input);
	proc->work_buf = malloc(proc->work_buf_len * sizeof(cpxf_t));
	proc->sql_sense_buf = malloc(proc->work_buf_len * sizeof(float));
	proc->sql_mute_buf = malloc(proc->work_buf_len * sizeof(char));

	/* initialize audio buffer */
	proc->audio_buf_len = (proc->work_buf_len + proc->decimf - 1) / proc->decimf;
	proc->audio_buf = malloc(proc->audio_buf_len * sizeof(float));

	/* initialize activity status */
	proc->out_act_stat = 0;
	proc->out_act_extctr = 0;
	proc->out_act_extcnt = MSECS2NSAMP(RXPROC_OUT_ACTSTATE_EXT_MS, fs_base);

	proc->audio_sink = audio_sink;

	if( (proc->work_buf != NULL) && (proc->sql_sense_buf != NULL) &&
		(proc->sql_mute_buf != NULL) && (proc->audio_buf != NULL) )
	{
		/* initialize input buffer */
		buflen = MSECS2NSAMP(config->proc_buf_len_ms, fs_input);
		bufwh = MSECS2NSAMP(config->proc_in_whyst_ms, fs_input);

		if(buf_init(&(proc->buf_input), sizeof(cpxf_t), buflen, 0, bufwh))
		{
			/* initialize oscillator section */
			proc->fc_input = fc_input;
			osc_init(&(proc->osc), 1.0);
			rxproc_osc_update(proc);

			/* initialize decimator section */
			proc->decim_inited = 0;

			decim_calc_factors(proc->decimf, proc->cfg.decim_mcsdf,
				&(proc->decim_comb_decimf), &(proc->decim_compf_decimf));

			if(rxproc_decim_init(proc, errbuf, errbufsize))
			{
				/* initialize baseband filter */
				proc->filter_inited = 0;
				proc->filter_tuned = 0;

				if(rxproc_filter_update(proc, errbuf, errbufsize))
				{
					/* initialize demodulator */
					proc->demod_inited = 0;

					if(rxproc_demod_init(proc, errbuf, errbufsize))
					{
						/* initialize squelch */
						proc->sql_inited = 0;

						if(rxproc_sql_init(proc, config, errbuf, errbufsize))
						{
							/* initialize output DC remover */
							iir_dcrem_init(&(proc->dcrem_state),
								proc->cfg.output_dcrem_alpha);

							/* set output gain */
							sndmix_input_gain(proc->audio_sink->in,
								proc->cfg.output_gain);

							/* initialize baseband level meter */
							proc->levelmtr_inited = 0;
							if(rxproc_levelmtr_init(proc, config, errbuf, errbufsize))
							{
								/* start processing thread */
								proc->hthread_proc = (HANDLE)_beginthreadex(
									NULL, 0, rxproc_threadproc, proc, 0, &tid);

								if(proc->hthread_proc != NULL)
								{
									/* mark as started */
									proc->is_started = 1;

									return 1;
								}

								rxproc_levelmtr_deinit(proc);
							}

							rxproc_sql_deinit(proc);
						}

						rxproc_demod_deinit(proc);
					}

					rxproc_filter_deinit(proc);
				}

				rxproc_decim_deinit(proc);
			}

			/* free input buffer */
			buf_cleanup(&(proc->buf_input));
		}
	}

	/* cleanup */
	free(proc->audio_buf);
	free(proc->sql_mute_buf);
	free(proc->sql_sense_buf);
	free(proc->work_buf);

	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize,
			_T("Can't start channel '%s' (out of memory?)"), proc->cfg.name);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* stop processing channel */
void rxproc_stop(rxproc_t *proc)
{
	/* check channel not already stopped */
	if(proc->is_started)
	{
		/* mark as stopped */
		proc->is_started = 0;

		/* stop processing thread */
		SetEvent(proc->evt_proc_stop);
		WaitForSingleObject(proc->hthread_proc, INFINITE);
		CloseHandle(proc->hthread_proc);

		/* deinit sections */
		rxproc_levelmtr_deinit(proc);
		rxproc_sql_deinit(proc);
		rxproc_demod_deinit(proc);
		rxproc_filter_deinit(proc);
		rxproc_decim_deinit(proc);

		/* free input buffer */
		buf_cleanup(&(proc->buf_input));

		/* cleanup */
		free(proc->audio_buf);
		free(proc->sql_mute_buf);
		free(proc->sql_sense_buf);
		free(proc->work_buf);
	}
}

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
					  TCHAR *errbuf, size_t errbufsize)
{
	rxproc_t *proc;

	_tcscpy(errbuf, _T(""));

	/* alloc memory */
	if( (proc = calloc(1, sizeof(rxproc_t))) != NULL )
	{
		proc->rx = rx;

		proc->act_cb = act_cb;
		proc->act_ctx = act_ctx;

		/* initialize channel config */
		if(rxprocconfig_init(&(proc->cfg), config, name, fc, loadsect))
		{
			/* initialize events and critical sections */
			InitializeCriticalSection(&(proc->cs_proc));
			proc->evt_proc_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
			proc->evt_input = CreateEvent(NULL, FALSE, FALSE, NULL);

			if( (proc->evt_proc_stop != NULL) && (proc->evt_input != NULL) )
				return proc;

			/* cleanup */
			CloseHandle(proc->evt_input);
			CloseHandle(proc->evt_proc_stop);
			DeleteCriticalSection(&(proc->cs_proc));

			rxprocconfig_cleanup(&(proc->cfg), NULL);
		}

		free(proc);
	}

	if(_tcscmp(errbuf, _T("")) == 0) {
		_sntprintf(errbuf, errbufsize,
			_T("Can't initialize channel '%s' (out of memory?)"), name);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* free processing channel
 *	savesect : ini section to save configuration to. */
void rxproc_clenaup(rxproc_t *proc, ini_sect_t *savesect)
{
	/* stop processing */
	rxproc_stop(proc);

	/* cleanup */
	CloseHandle(proc->evt_input);
	CloseHandle(proc->evt_proc_stop);
	DeleteCriticalSection(&(proc->cs_proc));

	rxprocconfig_cleanup(&(proc->cfg), savesect);
	free(proc);
}

/* ---------------------------------------------------------------------------------------------- */

/* send data to processing channel
 *	data : data from input device at input sampling frequency, complex float,
 *	nsamp : number of input samples. */
size_t rxproc_write_data(rxproc_t *proc, cpxf_t *data, size_t nsamp)
{
	size_t nsampwr;

	if(proc->is_started)
	{
		nsampwr = buf_write(&(proc->buf_input), data, nsamp);
		SetEvent(proc->evt_input);
		return nsampwr;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
