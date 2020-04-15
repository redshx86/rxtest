/* ---------------------------------------------------------------------------------------------- */

#include "rxprccfg.h"

/* ---------------------------------------------------------------------------------------------- */

/* demodulator type names */
TCHAR *rxproc_demod_display_name[] = {
	_T("AM"),	/* RXPROC_DEMOD_AM */
	_T("FM"),	/* RXPROC_DEMOD_FM */
};

/* ---------------------------------------------------------------------------------------------- */

int rxprocconfig_init(rxprocconfig_t *cfg)
{
	memset(cfg, 0, sizeof(rxprocconfig_t));
	
	cfg->name = malloc(RXLIM_PROC_MAXNAME * sizeof(TCHAR));
	if(cfg->name == NULL)
		return 0;

	_tcscpy(cfg->name, _T(""));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void rxprocconfig_cleanup(rxprocconfig_t *cfg)
{
	free(cfg->name);
}

/* ---------------------------------------------------------------------------------------------- */

void rxprocconfig_set_defaults(rxprocconfig_t *cfg, const TCHAR *name)
{
	_tcscpy(cfg->name, name);

	/* oscillator section */
	cfg->fc								= 0.0;

	/* decimator section */
	cfg->decim_frgran					= 2501;
	cfg->decim_mcsdf					= 4;
	cfg->decim_dff						= 0.1;
	cfg->decim_as						= 60.0;

	/* filter section */
	cfg->filter_fc						= 10000.0;
	cfg->filter_df						= 2500.0;
	cfg->filter_as						= 60.0;

	/* demodulator and squelch section */
	cfg->demod_type						= RXPROC_DEMOD_FM;
	rxproc_fm_cfg_set_defaults(&(cfg->fmdemod));
	rxsql_cfg_set_defaults(&(cfg->sql));

	/* output section */
	cfg->output_gain					= 0.0;
	cfg->output_mode					= SNDMIX_INPUT_CHAN_AB;
	cfg->output_dcrem_alpha				= 0.025;
}

/* ---------------------------------------------------------------------------------------------- */

void rxprocconfig_copy(rxprocconfig_t *dst, const rxprocconfig_t *src)
{
	_tcscpy(dst->name, src->name);

	/* oscillator section */
	dst->fc								= src->fc;

	/* decimator section */
	dst->decim_frgran					= src->decim_frgran;
	dst->decim_mcsdf					= src->decim_mcsdf;
	dst->decim_dff						= src->decim_dff;
	dst->decim_as						= src->decim_as;

	/* filter section */
	dst->filter_fc						= src->filter_fc;
	dst->filter_df						= src->filter_df;
	dst->filter_as						= src->filter_as;

	/* demodulator and squelch section */
	dst->demod_type						= src->demod_type;
	memcpy(&(dst->fmdemod), &(src->fmdemod), sizeof(src->fmdemod));
	memcpy(&(dst->sql), &(src->sql), sizeof(src->sql));

	/* output section */
	dst->output_gain					= src->output_gain;
	dst->output_mode					= src->output_mode;
	dst->output_dcrem_alpha				= src->output_dcrem_alpha;
}

/* ---------------------------------------------------------------------------------------------- */

/* load processing channel config */
void rxprocconfig_load(rxprocconfig_t *cfg, ini_sect_t *sect)
{
	/* channel name */
	ini_copys(sect, _T("name"), cfg->name, RXLIM_PROC_MAXNAME);

	/* oscillator section */
	ini_getfr(sect, _T("fc"), &(cfg->fc), RXLIM_FC_MIN, RXLIM_FC_MAX);

	/* decimator section */
	ini_getur(sect, _T("decim_frgran"), &(cfg->decim_frgran),
		RXLIM_PROC_DECIM_FRGRAN_MIN, RXLIM_PROC_DECIM_FRGRAN_MAX);
	ini_getur(sect, _T("decim_mcsdf"), &(cfg->decim_mcsdf),
		RXLIM_PROC_DECIM_MCSDF_MIN, RXLIM_PROC_DECIM_MCSDF_MAX);
	ini_getfr(sect, _T("decim_dff"), &(cfg->decim_dff),
		RXLIM_PROC_DECIM_DFF_MIN, RXLIM_PROC_DECIM_DFF_MAX);
	ini_getfr(sect, _T("decim_as"), &(cfg->decim_as),
		RXLIM_PROC_DECIM_AS_MIN, RXLIM_PROC_DECIM_AS_MAX);

	/* filter section */
	ini_getfr(sect, _T("filt_fc"), &(cfg->filter_fc),
		RXLIM_PROC_FILTER_FC_MIN, RXLIM_PROC_FILTER_FC_MAX);
	ini_getfr(sect, _T("filt_df"), &(cfg->filter_df),
		RXLIM_PROC_FILTER_DF_MIN, RXLIM_PROC_FILTER_DF_MAX);
	ini_getfr(sect, _T("filt_as"), &(cfg->filter_as),
		RXLIM_PROC_FILTER_AS_MIN, RXLIM_PROC_FILTER_AS_MAX);

	/* demodulator section */
	ini_gete(sect, _T("demod_type"), &(cfg->demod_type), RXPROC_DEMOD_COUNT);

	rxproc_fm_cfg_load(&(cfg->fmdemod), sect);
	rxsql_cfg_load(&(cfg->sql), sect);

	/* output section */
	ini_getfr(sect, _T("output_gain"), &(cfg->output_gain),
		RXLIM_AUDIO_GAIN_MIN, RXLIM_AUDIO_GAIN_MAX);
	ini_getir(sect, _T("output_mode"), &(cfg->output_mode),
		SNDMIX_INPUT_CHAN_NONE, SNDMIX_INPUT_CHAN_AB);
	ini_getfr(sect, _T("dcrem_alpha"), &(cfg->output_dcrem_alpha),
		RXLIM_PROC_DCREM_ALPHA_MIN, RXLIM_PROC_DCREM_ALPHA_MAX);
}

/* ---------------------------------------------------------------------------------------------- */

/* save processing channel config */
void rxprocconfig_save(ini_sect_t *sect, const rxprocconfig_t *cfg)
{
	/* channel name */
	ini_set(sect, _T("name"), cfg->name);

	/* oscillator section */
	ini_setf(sect, _T("fc"), 3, cfg->fc);

	/* decimator section */
	ini_setu(sect, _T("decim_frgran"), cfg->decim_frgran);
	ini_setu(sect, _T("decim_mcsdf"), cfg->decim_mcsdf);
	ini_setf(sect, _T("decim_dff"), 6, cfg->decim_dff);
	ini_setf(sect, _T("decim_as"), 6, cfg->decim_as);

	/* filter section */
	ini_setf(sect, _T("filt_fc"), 3, cfg->filter_fc);
	ini_setf(sect, _T("filt_df"), 3, cfg->filter_df);
	ini_setf(sect, _T("filt_as"), 6, cfg->filter_as);

	/* demodulator section */
	ini_setu(sect, _T("demod_type"), cfg->demod_type);

	rxproc_fm_cfg_save(sect, &(cfg->fmdemod));
	rxsql_cfg_save(sect, &(cfg->sql));

	/* output section  */
	ini_setf(sect, _T("output_gain"), 6, cfg->output_gain);
	ini_seti(sect, _T("output_mode"), cfg->output_mode);
	ini_setf(sect, _T("dcrem_alpha"), 6, cfg->output_dcrem_alpha);
}

/* ---------------------------------------------------------------------------------------------- */
