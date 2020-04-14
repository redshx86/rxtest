/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

/* channel options window context */
typedef struct chanopt_ctx {

	/* current window handle */
	HWND hwnd;

	/* common ui resources */
	uicommon_t *uidata;
	uievent_t *event_procchan;

	/* receiver state */
	rxstate_t *rx;

	/* affected channel list */
	unsigned int *chidList;
	int chidCount;

	/* sub window handles */
	HWND hwndEditDecimFrgran;
	HWND hwndEditDecimMcsdf;
	HWND hwndEditDecimDff;
	HWND hwndEditDecimAs;
	HWND hwndBtnDecimSetDefs;
	
	HWND hwndEditDcremAlpha;
	HWND hwndBtnDcremSetDefs;

} chanopt_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define CHANOPT_CW					295
#define CHANOPT_CH					250

/* ---------------------------------------------------------------------------------------------- */

#define CHANOPT_ID_DECIM_FRGRAN		1001
#define CHANOPT_ID_DECIM_MCSDF		1002
#define CHANOPT_ID_DECIM_DFF		1003
#define CHANOPT_ID_DECIM_AS			1004
#define CHANOPT_ID_DECIM_SETDEFS	1005

#define CHANOPT_ID_DCREM_ALPHA		1021
#define CHANOPT_ID_DCREM_SETDEFS	1022

#define CHANOPT_ID_APPLY			1041
#define CHANOPT_ID_OK				IDOK
#define CHANOPT_ID_CANCEL			IDCANCEL

/* ---------------------------------------------------------------------------------------------- */

int chanopt_createwindow(uicommon_t *uidata, uievent_t *event_procchan, HWND hwndOwner,
						 rxstate_t *rx, unsigned int *chidList, int chidCount, int x, int y);

int chanopt_registerclass(uicommon_t *uidata);
void chanopt_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
