/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../dsp/dsp.h"
#include "../util/iniparse.h"
#include "../util/macro.h"

/* ---------------------------------------------------------------------------------------------- */

#define RXPROC_FM_DF_MIN			50.0
#define RXPROC_FM_DF_MAX			200000.0

/* ---------------------------------------------------------------------------------------------- */

/* FM demodulator configuration */
typedef struct rxproc_fm_config {

	double df;					/* frequency deviation, Hz */

} rxproc_fm_config_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct rxproc_fm {

	rxproc_fm_config_t *cfg;	/* demodulator configuration pointer */

	unsigned int fs_base;		/* baseband sampling frequency */

	demod_fm_state_t demod;		/* demodulator state */

} rxproc_fm_t;

/* ---------------------------------------------------------------------------------------------- */

void rxproc_fm_cfg_reset(rxproc_fm_config_t *cfg);
void rxproc_fm_cfg_load(rxproc_fm_config_t *cfg, ini_sect_t *sect);
void rxproc_fm_cfg_save(rxproc_fm_config_t *cfg, ini_sect_t *sect);

/* ---------------------------------------------------------------------------------------------- */

int rxproc_fm_configure(rxproc_fm_t *c, double df);

int rxproc_fm_init(rxproc_fm_t *c, rxproc_fm_config_t *cfg, unsigned int fs_base);

/* ---------------------------------------------------------------------------------------------- */

void rxproc_fm_process(rxproc_fm_t *c, float *dst, cpxf_t *src, size_t nsamp);

/* ---------------------------------------------------------------------------------------------- */
