/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

/* filter configuration window context */
typedef struct fmcfg_ctx {

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
	HWND hwndDfEdit;

	HWND hwndBtnSaveDef;

} fmcfg_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define FMCFG_CW			240
#define FMCFG_CH			86

#define FMCFG_ID_DF			1001

#define FMCFG_ID_SAVEDEF	1021

#define FMCFG_ID_APPLY		1041
#define FMCFG_ID_OK			IDOK
#define FMCFG_ID_CANCEL		IDCANCEL

/* ---------------------------------------------------------------------------------------------- */

int fmcfg_createwindow(uicommon_t *uidata, uievent_t *event_procchan, HWND hwndOwner,
					   rxstate_t *rx, rxprocconfig_t *procdefcfg,
					   unsigned int *chidList, int chidCount, int x, int y);

int fmcfg_registerclass(uicommon_t *uidata);
void fmcfg_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
