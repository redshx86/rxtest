/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

/* filter configuration window context */
typedef struct filtcfg_ctx {

	/* current window handle */
	HWND hwnd;

	/* common ui resources */
	uicommon_t *uidata;
	uievent_t *event_procchan;

	/* receiver state */
	rxstate_t *rx;
	rxprocconfig_t *procdefcfg;

	/* affected channel list */
	unsigned int *chidList;
	int chidCount;

	/* sub window handles */
	HWND hwndFcEdit;
	HWND hwndDfEdit;
	HWND hwndAsEdit;

	HWND hwndBtnSaveDef;

} filtcfg_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define FILTCFG_CW			240
#define FILTCFG_CH			126

#define FILTCFG_ID_FC		1001
#define FILTCFG_ID_DF		1002
#define FILTCFG_ID_AS		1003

#define FILTCFG_ID_SAVEDEF	1021

#define FILTCFG_ID_APPLY	1041
#define FILTCFG_ID_OK		IDOK
#define FILTCFG_ID_CANCEL	IDCANCEL

/* ---------------------------------------------------------------------------------------------- */

int filtcfg_createwindow(uicommon_t *uidata, uievent_t *event_procchan, HWND hwndOwner,
						 rxstate_t *rx, rxprocconfig_t *procdefcfg,
						 unsigned int *chidList, int chidCount, int x, int y);

int filtcfg_registerclass(uicommon_t *uidata);
void filtcfg_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
