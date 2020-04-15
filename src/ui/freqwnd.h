/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"
#include "../rx/rxstate.h"
#include "specview.h"
#include "watrview.h"
#include "freqcfg.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct freqwnd_ctx {

	HWND hwnd;
	
	HWND hwndMain;
	uicommon_t *uidata;
	ini_data_t *ini;

	uievent_t *event_window_close;
	uievent_t *event_rx_state;

	freqcfg_t *fcfg;
	rxstate_t *rx;
	specview_ctx_t *specview;
	watrview_ctx_t *watrview;

	/* input center frequency controls */
	int isFInSelEnabled;
	HWND hwndEditFIn;
	HWND hwndBtnFInSet;
	WNDPROC wndprocEditFIn;
	int prefixFIn;

	/* display frequency range controls (end points) */
	HWND hwndEditDispF0;
	HWND hwndEditDispF1;
	HWND hwndBtnFDispSet;
	WNDPROC wndprocEditDispF0;
	WNDPROC wndprocEditDispF1;
	int prefixFDisp0;
	int prefixFDisp1;

	/* display frequency range controls (length) */
	HWND hwndCbFDispLen;
	HWND hwndRbFDispLenL;
	HWND hwndRbFDispLenC;
	HWND hwndRbFDispLenR;
	HWND hwndBtnFDispLenSet;
	int prefixFDispLen;

	/* grid snap controls */
	HWND hwndCbDispSnap;
	HWND hwndCbMarkSnap;
	int prefixFDispSnap;
	int prefFMarkSnap;

	/* window frame size */
	int cx_frame;
	int cy_frame;

} freqwnd_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define FREQWND_W					460
#define FREQWND_H					145

/* ---------------------------------------------------------------------------------------------- */

#define FREQWND_ID_F_IN				1001
#define FREQWND_ID_F_IN_SET			1002

#define FREQWND_ID_F_IN_SNAP		1021

#define FREQWND_ID_F_DISP_0			1041
#define FREQWND_ID_F_DISP_1			1042
#define FREQWND_ID_F_DISP_SET		1043

#define FREQWND_ID_F_DISP_LEN		1061
#define FREQWND_ID_F_DISP_LEN_L		1062
#define FREQWND_ID_F_DISP_LEN_C		1063
#define FREQWND_ID_F_DISP_LEN_R		1064
#define FREQWND_ID_F_DISP_LEN_SET	1065

#define FREQWND_ID_DISP_SNAP		1081
#define FREQWND_ID_MARK_SNAP		1082

/* ---------------------------------------------------------------------------------------------- */

HWND freqwnd_create(uicommon_t *uidata, ini_data_t *ini, freqcfg_t *fcfg,
					rxstate_t *rx, specview_ctx_t *specview, watrview_ctx_t *watrview,
					HWND hwndMain);

int freqwnd_registerclass(uicommon_t *uidata);
void freqwnd_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
