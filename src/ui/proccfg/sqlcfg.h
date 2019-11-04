/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

/* squelch configuration window context */
typedef struct sqlcfg_ctx {

	/* current window handle */
	HWND hwnd;

	/* common ui resources */
	uicommon_t *uidata;

	/* global callback list */
	callback_list_t *cb_list;

	/* receiver state */
	rxstate_t *rx;

	/* affected channel list */
	unsigned int *chidList;
	int chidCount;

	/* sub window handles */
	HWND hwndBtnUseCarrf;
	HWND hwndEditBw;
	HWND hwndEditEfParam;
	HWND hwndEditOpThres;
	HWND hwndEditOpDly;
	HWND hwndEditClThres;
	HWND hwndEditClDly;

	HWND hwndBtnSaveDef;

} sqlcfg_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define CARRSQCFG_CW			240
#define CARRSQCFG_CH			205

#define CARRSQCFG_ID_USECARRF	1001
#define CARRSQCFG_ID_BW			1002
#define CARRSQCFG_ID_EFPARAM	1003
#define CARRSQCFG_ID_OPNTHRES	1004
#define CARRSQCFG_ID_OPNDELAY	1005
#define CARRSQCFG_ID_CLSTHRES	1006
#define CARRSQCFG_ID_CLSDELAY	1007

#define CARRSQCFG_ID_SAVEDEF	1021

#define CARRSQCFG_ID_APPLY		1041
#define CARRSQCFG_ID_OK			IDOK
#define CARRSQCFG_ID_CANCEL		IDCANCEL

/* ---------------------------------------------------------------------------------------------- */

int sqlcfg_createwindow(uicommon_t *uidata, callback_list_t *cb_list, HWND hwndOwner,
						   rxstate_t *rx, unsigned int *chidList, int chidCount, int x, int y);

int sqlcfg_registerclass(uicommon_t *uidata);
void sqlcfg_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
