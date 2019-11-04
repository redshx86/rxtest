/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../util/iniparse.h"
#include "../rx/rxlimits.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct freqcfg {

	double disp_f_0;
	double disp_f_1;

	double f_disp_range_snap;
	double f_mark_snap;

} freqcfg_t;

/* ---------------------------------------------------------------------------------------------- */

void freqcfg_init(freqcfg_t *fcfg, ini_data_t *ini);
void freqcfg_save(freqcfg_t *fcfg, ini_data_t *ini);

/* ---------------------------------------------------------------------------------------------- */
