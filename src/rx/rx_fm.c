/* ---------------------------------------------------------------------------------------------- */

#include "rx_fm.h"

/* ---------------------------------------------------------------------------------------------- */

void rxproc_fm_cfg_reset(rxproc_fm_config_t *cfg)
{
	cfg->df = 12500.0;
}

/* ---------------------------------------------------------------------------------------------- */

void rxproc_fm_cfg_load(rxproc_fm_config_t *cfg, ini_sect_t *sect)
{
	ini_getfr(sect, _T("fm_df"), &(cfg->df), RXPROC_FM_DF_MIN, RXPROC_FM_DF_MAX);
}

/* ---------------------------------------------------------------------------------------------- */

void rxproc_fm_cfg_save(rxproc_fm_config_t *cfg, ini_sect_t *sect)
{
	ini_setf(sect, _T("fm_df"), 6, cfg->df);
}

/* ---------------------------------------------------------------------------------------------- */

int rxproc_fm_configure(rxproc_fm_t *c, double df)
{
	double dff;

	if(!INRANGE_MINMAX(df, RXPROC_FM_DF))
		return 0;

	dff = df / (double)(c->fs_base);
	demod_fm_tune(&(c->demod), dff);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int rxproc_fm_init(rxproc_fm_t *c, rxproc_fm_config_t *cfg, unsigned int fs_base)
{
	c->cfg = cfg;
	c->fs_base = fs_base;

	demod_fm_init(&(c->demod));

	if(!rxproc_fm_configure(c, cfg->df))
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void rxproc_fm_process(rxproc_fm_t *c, float *dst, cpxf_t *src, size_t nsamp)
{
	demod_fm_process(&(c->demod), dst, src, nsamp);
}

/* ---------------------------------------------------------------------------------------------- */
