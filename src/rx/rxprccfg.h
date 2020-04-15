/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "rxlimits.h"
#include "../audio/sndmix.h"
#include "rx_fm.h"
#include "rxsql.h"

/* ---------------------------------------------------------------------------------------------- */

/* demodulator type */
typedef enum rxproc_demodtype {
	RXPROC_DEMOD_AM,
	RXPROC_DEMOD_FM,
	RXPROC_DEMOD_COUNT
} rxproc_demodtype_t;

/* ---------------------------------------------------------------------------------------------- */

extern TCHAR *rxproc_demod_display_name[];

/* ---------------------------------------------------------------------------------------------- */

typedef struct rxprocconfig {

	/* channel name */
	TCHAR *name;

	/* oscillator section */
	double fc;							/* channel carrier frequency, Hz */

	/* decimator section */
	unsigned int decim_frgran;			/* frequency response granularity */
	unsigned int decim_mcsdf;			/* minimum compensation stage decimation factor*/
	double decim_dff;					/* transtion bandwidth, fractional */
	double decim_as;					/* stopband attenuation, dB */

	/* filter section */
	double filter_fc;					/* cutoff frequency, Hz */
	double filter_df;					/* transition bandwidth, Hz */
	double filter_as;					/* stopband attenuation, dB */

	/* demodulator and squelch section */
	rxproc_demodtype_t demod_type;		/* current demodulator type */
	rxproc_fm_config_t fmdemod;			/* FM demodulator configuration */
	rxsql_cfg_t sql;					/* squelch config */

	/* output section */
	double output_gain;					/* output gain */
	int output_mode;					/* audio output mode */
	double output_dcrem_alpha;			/* DC remover filter parameter */

} rxprocconfig_t;

/* ---------------------------------------------------------------------------------------------- */

int rxprocconfig_init(rxprocconfig_t *cfg);
void rxprocconfig_cleanup(rxprocconfig_t *cfg);

void rxprocconfig_set_defaults(rxprocconfig_t *cfg, const TCHAR *name);

void rxprocconfig_copy(rxprocconfig_t *dst, const rxprocconfig_t *src);

void rxprocconfig_load(rxprocconfig_t *cfg, ini_sect_t *sect);
void rxprocconfig_save(ini_sect_t *sect, const rxprocconfig_t *cfg);

/* ---------------------------------------------------------------------------------------------- */
