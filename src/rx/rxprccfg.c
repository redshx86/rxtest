/* ---------------------------------------------------------------------------------------------- */

#include "rxprccfg.h"

/* ---------------------------------------------------------------------------------------------- */

/* demodulator type names */
TCHAR *rxproc_demod_display_name[] = {
	_T("AM"),	/* RXPROC_DEMOD_AM */
	_T("FM"),	/* RXPROC_DEMOD_FM */
};

/* ---------------------------------------------------------------------------------------------- */

static void rxprocconfig_setdefaults(rxprocconfig_t *cfg, rxconfig_t *defs, TCHAR *name, double fc)
{
	/* channel name */
	_tcscpy(cfg->name, name);

	/* oscillator section */
	cfg->fc = fc;

	/* decimator section */
	cfg->decim_frgran = defs->proc_decim_frgran;
	cfg->decim_mcsdf = defs->proc_decim_mcsdf;
	cfg->decim_dff = defs->proc_decim_dff;
	cfg->decim_as = defs->proc_decim_as;

	/* filter section */
	cfg->filter_fc = defs->proc_filter_fc;
	cfg->filter_df = defs->proc_filter_df;
	cfg->filter_as = defs->proc_filter_as;

	/* demodulator and squelch section */
	cfg->demod_type = RXPROC_DEMOD_FM;

	memcpy(&(cfg->fmdemod), &(defs->proc_fm_def), sizeof(rxproc_fm_config_t));
	memcpy(&(cfg->sql), &(defs->proc_sql_def), sizeof(rxsql_cfg_t));

	/* output section */
	cfg->output_gain = 0.0;
	cfg->output_mode = SNDMIX_INPUT_CHAN_AB;
	cfg->output_dcrem_alpha = defs->proc_dcrem_alpha;
}

/* ---------------------------------------------------------------------------------------------- */

int rxprocconfig_init(rxprocconfig_t *cfg, rxconfig_t *defs, TCHAR *name, double fc,
					  ini_sect_t *loadsect)
{
	cfg->name = malloc(RXLIM_PROC_MAXNAME * sizeof(TCHAR));

	if(cfg->name != NULL)
	{
		rxprocconfig_setdefaults(cfg, defs, name, fc);

		if(loadsect != NULL) {
			rxprocconfig_load(loadsect, cfg);
		}

		return 1;
	}

	free(cfg);
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void rxprocconfig_cleanup(rxprocconfig_t *cfg, ini_sect_t *savesect)
{
	if(savesect != NULL) {
		rxprocconfig_save(savesect, cfg);
	}

	free(cfg->name);
}

/* ---------------------------------------------------------------------------------------------- */

/* load processing channel config */
void rxprocconfig_load(ini_sect_t *sect, rxprocconfig_t *cfg)
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
void rxprocconfig_save(ini_sect_t *sect, rxprocconfig_t *cfg)
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

	rxproc_fm_cfg_save(&(cfg->fmdemod), sect);
	rxsql_cfg_save(&(cfg->sql), sect);

	/* output section  */
	ini_setf(sect, _T("output_gain"), 6, cfg->output_gain);
	ini_seti(sect, _T("output_mode"), cfg->output_mode);
	ini_setf(sect, _T("dcrem_alpha"), 6, cfg->output_dcrem_alpha);
}

/* ---------------------------------------------------------------------------------------------- */
