/* ---------------------------------------------------------------------------------------------- */

#include "rxsql.h"

/* ---------------------------------------------------------------------------------------------- */

/* initialize squelch configuration */
void rxsql_cfg_reset(rxsql_cfg_t *cfg)
{
	cfg->use_carr_filter	= 1;

	cfg->bw					= 500.0;

	cfg->envef_param		= 0.1;

	cfg->op_thres_db		= 5.0;
	cfg->cl_thres_db		= -5.0;

	cfg->op_dly_ms			= 50.0;
	cfg->cl_dly_ms			= 50.0;
}

/* ---------------------------------------------------------------------------------------------- */

/* load squelch configuration */
void rxsql_cfg_load(rxsql_cfg_t *cfg, ini_sect_t *sect)
{
	ini_getb(sect, _T("sql_use_carr_filter"), &(cfg->use_carr_filter));

	ini_getfr(sect, _T("sql_bw"), &(cfg->bw),
		RXSQL_BW_MIN, RXSQL_BW_MAX);

	ini_getfr(sect, _T("sql_envef_param"), &(cfg->envef_param),
		RXSQL_ENVEF_PARAM_MIN, RXSQL_ENVEF_PARAM_MAX);

	ini_getfr(sect, _T("sql_op_thres"), &(cfg->op_thres_db),
		RXSQL_THRES_DB_MIN, RXSQL_THRES_DB_MAX);
	ini_getfr(sect, _T("sql_cl_thres"), &(cfg->cl_thres_db),
		RXSQL_THRES_DB_MIN, RXSQL_THRES_DB_MAX);

	if(cfg->cl_thres_db > cfg->op_thres_db)
		cfg->cl_thres_db = cfg->op_thres_db;

	ini_getfr(sect, _T("sql_op_dly_ms"), &(cfg->op_dly_ms),
		RXSQL_DLY_MS_MIN, RXSQL_DLY_MS_MAX);
	ini_getfr(sect, _T("sql_cl_dly_ms"), &(cfg->cl_dly_ms),
		RXSQL_DLY_MS_MIN, RXSQL_DLY_MS_MAX);
}

/* ---------------------------------------------------------------------------------------------- */

/* save squelch configuration */
void rxsql_cfg_save(rxsql_cfg_t *cfg, ini_sect_t *sect)
{
	ini_setb(sect, _T("sql_use_carr_filter"), cfg->use_carr_filter);

	ini_setf(sect, _T("sql_bw"), 3, cfg->bw);

	ini_setf(sect, _T("sql_envef_param"), 6, cfg->bw);

	ini_setf(sect, _T("sql_op_thres"), 6, cfg->op_thres_db);
	ini_setf(sect, _T("sql_cl_thres"), 6, cfg->cl_thres_db);

	ini_setf(sect, _T("sql_op_dly_ms"), 6, cfg->op_dly_ms);
	ini_setf(sect, _T("sql_cl_dly_ms"), 6, cfg->cl_dly_ms);
}

/* ---------------------------------------------------------------------------------------------- */

/* process data with squelch
 *	data : data to process, complex float,
 *	magbuf : output squelch sense magnitudes, linear,
 *	mutebuf : output buffer with mute flags. */
void rxsql_process(rxsql_state_t *s,
				   cpxf_t *data, float *magbuf, unsigned char *mutebuf, size_t nsamp)
{
	size_t nsamp_process, i;
	float pwr;
	double bwscal, rms;
	rxsql_dlyelem_t de_out, de_in;

	/* calculate bandwidth scale factor */
	bwscal = (double)(s->fs_input) / (double)(s->fs_base);

	while(nsamp != 0)
	{
		/* block length to process */
		nsamp_process = min(nsamp, s->pwrbuf_len);

		/* copy data */
		memcpy(s->pwrbuf, data, nsamp_process * sizeof(cpxf_t));

		/* execute carrier filter */
		if(s->cfgp->use_carr_filter) {
			goertzw_exec(&(s->carrf), s->pwrbuf, nsamp_process);
		}

		for(i = 0; i < nsamp_process; i++)
		{
			/* process power envelope */
			pwr = cpxf_modsqr(&(s->pwrbuf[i]));
			iirfilt_process1(&(s->envef), &pwr, &pwr, 1);
			rms = sqrt(pwr * bwscal);

			s->sense = rms;

			/* process squelch */
			if(!s->is_opn)
			{
				if(rms >= s->op_thres)
				{
					if(s->opn_cnt >= s->op_dly)
						s->is_opn = 1;
					else
						s->opn_cnt++;
				}
				else
				{
					s->opn_cnt = 0;
				}
			}
			else
			{
				if(rms <= s->cl_thres)
				{
					if(s->cls_cnt >= s->cl_dly)
						s->is_opn = 0;
					else
						s->cls_cnt++;
				}
				else
				{
					s->cls_cnt = 0;
				}
			}

			/* delay input data */
			cpxf_copy(&(de_in.sample), &(data[i]));
			de_in.mag = (float)rms;
			delaybuf(&(s->dlybuf), &de_out, &de_in);

			/* return delayed data */
			cpxf_copy(&(data[i]), &(de_out.sample));
			*(magbuf++) = de_out.mag;
			*(mutebuf++) = s->is_opn ? 0 : 1;
		}

		data += nsamp_process;
		nsamp -= nsamp_process;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* (re-)initialize carrier filter */
static int rxsql_carrf_update(rxsql_state_t *s,
							  TCHAR *procname, TCHAR *errbuf, size_t errbufsize)
{
	size_t n, goertz_n;
	float gain, wf[3];

	/* disable carrier filter? */
	if(!s->cfgp->use_carr_filter)
	{
		if(s->carrf_is_inited) {
			goertzw_cleanup(&(s->carrf));
			s->carrf_is_inited = 0;
		}
		return 1;
	}

	n = (size_t)((double)(s->fs_base) / s->cfgp->bw + 0.5);
	goertz_n = max( (n | 1UL), 3 );

	/* check filter reinit needed */
	if((!s->carrf_is_inited) || (s->carrf.comb.n != goertz_n))
	{
		/* free carrier filter state */
		if(s->carrf_is_inited) {
			goertzw_cleanup(&(s->carrf));
			s->carrf_is_inited = 0;
		}

		/* use blackman window */
		gain = (float)(1.0 / sqrt(goertz_n));
		wf[0] =  0.7610f * gain;
		wf[1] = -0.4530f * gain;
		wf[2] = -0.0725f * gain;

		/* initialize carrier filter */
		if(!goertzw_init(&(s->carrf), goertz_n, 0, 3, wf, 1.0 - 1e-6)) {
			_sntprintf(errbuf, errbufsize,
				_T("Can't initialize squelch carrier filter for channel '%s' ")
				_T("(n = %Iu, k = 0)."), procname, goertz_n);
			return 0;
		}

		s->carrf_is_inited = 1;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* (re-)initialize power envelope filter */
static void rxsql_envef_update(rxsql_state_t *s)
{
	float b[2], a[2];

	b[0] = (float)(s->cfgp->envef_param);
	b[1] = 0.0f;
	
	a[0] = 1.0f;
	a[1] = -(float)(1.0 - s->cfgp->envef_param);

	iirfilt_init1(&(s->envef), b, a);

	s->envef_is_inited = 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* (re-)initialize squelch thresholds */
static void rxsql_thresholds_update(rxsql_state_t *s)
{
	size_t op_dly, cl_dly;

	s->op_thres = DB2LINM(s->cfgp->op_thres_db);
	s->cl_thres = DB2LINM(s->cfgp->cl_thres_db);

	op_dly = (size_t)(0.001 * s->cfgp->op_dly_ms * (double)(s->fs_base) + 0.5);
	cl_dly = (size_t)(0.001 * s->cfgp->cl_dly_ms * (double)(s->fs_base) + 0.5);

	if(op_dly != s->op_dly)
	{
		s->op_dly = op_dly;
		s->opn_cnt = 0;
	}

	if(cl_dly != s->cl_dly)
	{
		s->cl_dly = cl_dly;
		s->cls_cnt = 0;
	}

	s->thresholds_are_inited = 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* (re-)initialize delay buffer */
static int rxsql_delaybuf_update(rxsql_state_t *s,
								 TCHAR *procname, TCHAR *errbuf, size_t errbufsize)
{
	size_t goertz_n, dlylen;

	/* calculate required delay buffer length */
	goertz_n = (size_t)((double)(s->fs_base) / s->cfgp->bw + 0.5);
	dlylen = ((s->op_dly +  s->cl_dly) / 2) + (goertz_n / 2) + 16;

	/* check reinit required */
	if((!s->dlybuf_is_inited) || (dlylen != s->dlybuf.length))
	{
		/* free current delay buffer state */
		if(s->dlybuf_is_inited) {
			delaybuf_cleanup(&(s->dlybuf));
			s->dlybuf_is_inited = 0;
		}

		/* reinit delay buffer */
		if(!delaybuf_init(&(s->dlybuf), sizeof(rxsql_dlyelem_t), dlylen)) {
			_sntprintf(errbuf, errbufsize,
				_T("Can't initialize squelch delay buffer for channel '%s' ")
				_T("(delay = %Iu samples)."), procname, dlylen);
			return 0;
		}

		s->dlybuf_is_inited = 1;
	}

	return 1;
}

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
					TCHAR *procname, TCHAR *errbuf, size_t errbufsize)
{
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
			_T("Bad squelch configuration for channel '%s'."), procname);
		return 0;
	}

	/* update carrier filter */
	if( (use_carr_filter != s->carrf_is_inited) ||
		(fabs(bw - s->cfgp->bw) >= 1e-3) )
	{
		s->cfgp->use_carr_filter = use_carr_filter;
		s->cfgp->bw = bw;

		if(!rxsql_carrf_update(s, procname, errbuf, errbufsize))
			return 0;
	}

	/* update power envelope filter */
	if( (!s->envef_is_inited) ||
		(fabs(envef_param - s->cfgp->envef_param) >= 1e-6) )
	{
		s->cfgp->envef_param = envef_param;

		rxsql_envef_update(s);
	}

	/* update thresholds */
	if( (!s->thresholds_are_inited) ||
		(fabs(op_thres - s->cfgp->op_thres_db) >= 1e-6) ||
		(fabs(cl_thres - s->cfgp->cl_thres_db) >= 1e-6) ||
		(fabs(op_dly - s->cfgp->op_dly_ms) >= 1e-6) ||
		(fabs(cl_dly - s->cfgp->cl_dly_ms) >= 1e-6) )
	{
		s->cfgp->op_thres_db = op_thres;
		s->cfgp->cl_thres_db = cl_thres;
		s->cfgp->op_dly_ms = op_dly;
		s->cfgp->cl_dly_ms = cl_dly;

		rxsql_thresholds_update(s);
	}

	/* update delay buffer */
	if(!rxsql_delaybuf_update(s, procname, errbuf, errbufsize))
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* test squelch ready status */
int rxsql_check(rxsql_state_t *s)
{
	if(s->cfgp->use_carr_filter && (!s->carrf_is_inited))
		return 0;
	
	if(!s->envef_is_inited)
		return 0;

	if(!s->thresholds_are_inited)
		return 0;

	if(!s->dlybuf_is_inited)
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize squelch state
 *	cfgp : filter configuration pointer,
 *	fs_input : input device sampling frequency (for psd scaling), Hz,
 *	fs_base : baseband sampling frequency,
 *	procname : processing channel name (for message formatting). */
int rxsql_init(rxsql_state_t *s, rxsql_cfg_t *cfgp,
			   unsigned int fs_input, unsigned int fs_base,
			   TCHAR *procname, TCHAR *errbuf, size_t errbufsize)
{
	if((fs_input == 0) || (fs_base == 0)) {
		_sntprintf(errbuf, errbufsize,
			_T("Can't initialize squelch for channel '%s'. Bad sampling frequency."),
			procname);
		return 0;
	}

	memset(s, 0, sizeof(rxsql_state_t));

	s->cfgp = cfgp;

	s->fs_input = fs_input;
	s->fs_base = fs_base;

	s->pwrbuf_len = RXSQL_BUF_LEN;
	s->pwrbuf = malloc(s->pwrbuf_len * sizeof(cpxf_t));

	if(s->pwrbuf != NULL)
	{
		if( rxsql_configure(s, cfgp->use_carr_filter,
			cfgp->bw, cfgp->envef_param,
			cfgp->op_thres_db, cfgp->cl_thres_db,
			cfgp->op_dly_ms, cfgp->cl_dly_ms,
			procname, errbuf, errbufsize) )
		{
			return 1;
		}
	}

	if(s->dlybuf_is_inited)
		delaybuf_cleanup(&(s->dlybuf));
	if(s->carrf_is_inited)
		goertzw_cleanup(&(s->carrf));
	free(s->pwrbuf);

	if(_tcscmp(errbuf, _T("")) == 0)
	{
		_sntprintf(errbuf, errbufsize,
			_T("Can't initialize squelch for channel '%s' (out of memory?)."),
			procname);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* free squelch state */
void rxsql_cleanup(rxsql_state_t *s)
{
	if(s->dlybuf_is_inited)
		delaybuf_cleanup(&(s->dlybuf));
	if(s->carrf_is_inited)
		goertzw_cleanup(&(s->carrf));
	free(s->pwrbuf);
}

/* ---------------------------------------------------------------------------------------------- */
