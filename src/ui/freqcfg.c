/* ---------------------------------------------------------------------------------------------- */

#include "freqcfg.h"

/* ---------------------------------------------------------------------------------------------- */

static void freqcfg_load(freqcfg_t *fcfg, ini_data_t *ini)
{
	ini_sect_t *sect;
	double disp_f_0, disp_f_1;

	sect = ini_sect_get(ini, _T("freqcfg"), 0);

	disp_f_0 = fcfg->disp_f_0;
	disp_f_1 = fcfg->disp_f_1;

	if( ini_getfr(sect, _T("disp_f_0"), &disp_f_0, RXLIM_FC_MIN, RXLIM_FC_MAX) &&
		ini_getfr(sect, _T("disp_f_1"), &disp_f_1, RXLIM_FC_MIN, RXLIM_FC_MAX) &&
		(disp_f_0 < disp_f_1) )
	{
		fcfg->disp_f_0 = disp_f_0;
		fcfg->disp_f_1 = disp_f_1;
	}

	ini_getfr(sect, _T("f_disp_range_snap"), &(fcfg->f_disp_range_snap), 0.0, 1e6);
	ini_getfr(sect, _T("f_mark_snap"), &(fcfg->f_mark_snap), 0.0, 1e6);
}

/* ---------------------------------------------------------------------------------------------- */

void freqcfg_init(freqcfg_t *fcfg, ini_data_t *ini)
{
	fcfg->disp_f_0				= 26.9e6;
	fcfg->disp_f_1				= 27.5e6;

	fcfg->f_disp_range_snap		=   25e3;
	fcfg->f_mark_snap			= 6.25e3;

	freqcfg_load(fcfg, ini);
}

/* ---------------------------------------------------------------------------------------------- */

void freqcfg_save(freqcfg_t *fcfg, ini_data_t *ini)
{
	ini_sect_t *sect;

	sect = ini_sect_get(ini, _T("freqcfg"), 1);

	ini_setf(sect, _T("disp_f_0"), 3, fcfg->disp_f_0);
	ini_setf(sect, _T("disp_f_1"), 3, fcfg->disp_f_1);

	ini_setf(sect, _T("f_disp_range_snap"), 3, fcfg->f_disp_range_snap);
	ini_setf(sect, _T("f_mark_snap"), 3, fcfg->f_mark_snap);
}

/* ---------------------------------------------------------------------------------------------- */
