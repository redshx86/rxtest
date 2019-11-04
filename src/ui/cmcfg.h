/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"

/* ---------------------------------------------------------------------------------------------- */

#define CMCFG_LEVEL_ABS_MIN				-180.0
#define CMCFG_LEVEL_ABS_MAX				180.0

#define CMCFG_LEVEL_UPDATE_INT_MIN		10
#define CMCFG_LEVEL_UPDATE_INT_MAX		1000

/* ---------------------------------------------------------------------------------------------- */

typedef struct cmcfg_data {

	int dispUnitChanFreq;
	int dispUnitFiltCutoff;

	double levelMin;
	double levelMax;
	int levelUpdateInt;
	int levelScaleToInputFc;

} cmcfg_data_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct cmcfg_ctx {

	HWND hwnd;

	uicommon_t *uidata;
	callback_list_t *cb_list;

	HWND hwndBtnChanFreqUnitHz;
	HWND hwndBtnChanFreqUnitKhz;
	HWND hwndBtnChanFreqUnitMhz;

	HWND hwndBtnFilterCutoffUnitHz;
	HWND hwndBtnFilterCutoffUnitKhz;
	HWND hwndBtnFilterCutoffUnitMhz;

	HWND hwndEditLevelMin;
	HWND hwndEditLevelMax;
	HWND hwndEditLevelUpdateInt;
	HWND hwndBtnLevelScaleToInputBw;


} cmcfg_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define CMCFG_W								330
#define CMCFG_H								160

/* ---------------------------------------------------------------------------------------------- */

#define CMCFG_ID_CHAN_FREQ_UNIT_HZ			1001
#define CMCFG_ID_CHAN_FREQ_UNIT_KHZ			1002
#define CMCFG_ID_CHAN_FREQ_UNIT_MHZ			1003

#define CMCFG_ID_FILT_CUTOFF_UNIT_HZ		1021
#define CMCFG_ID_FILT_CUTOFF_UNIT_KHZ		1022
#define CMCFG_ID_FILT_CUTOFF_UNIT_MHZ		1023

#define CMCFG_ID_LEVEL_MIN					1041
#define CMCFG_ID_LEVEL_MAX					1042
#define CMCFG_ID_LEVEL_UPDATE_INT			1043
#define CMCFG_ID_LEVEL_SCALE_TO_INPUT		1044

#define CMWND_ID_BTN_CANCEL					IDCANCEL
#define CMWND_ID_BTN_APPLY					1061
#define CMWND_ID_BTN_OK						IDOK

/* ---------------------------------------------------------------------------------------------- */

HWND cmcfg_create(uicommon_t *uidata, callback_list_t *cb_list, HWND hwnd_owner, int x, int y);

void cmcfg_load(HWND hwnd, cmcfg_data_t *cfg);

int cmcfg_registerclass(uicommon_t *uidata);
void cmcfg_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
