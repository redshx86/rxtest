/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"
#include "cmwnd.h"
#include "freqwnd.h"
#include "sets/setwnd.h"
#include "audctrl.h"
#include "specview.h"
#include "watrview.h"
#include "freqcfg.h"
#include "../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

typedef enum rxwnd_drag_mode {

	RXWND_DRAG_NOTHING,

	RXWND_DRAG_SPECVIEW_SHM,
	RXWND_DRAG_INPUT_FC,
	RXWND_DRAG_SPECVIEW_SHM_AND_INPUT_FC,
	RXWND_DRAG_CHAN_FC,
	RXWND_DRAG_CHAN_BW_EDGE_LOWER,
	RXWND_DRAG_CHAN_BW_EDGE_UPPER,
	RXWND_DRAG_CHAN_SQL_OPEN_THRES,
	RXWND_DRAG_CHAN_SQL_CLOSE_THRES,

	RXWND_DRAG_WATRVIEW_IMG,
	RXWND_SCROLL_WATRVIEW

} rxwnd_drag_mode_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct rxwnd_dragstate {

	rxwnd_drag_mode_t mode;
	unsigned int chid;

	int origin_x;
	int origin_y;
	int current_x;
	int current_y;

	HCURSOR hcur_drag;
	HCURSOR hcur_point;

	double fc_input_org;
	double disp_f0_org;
	int scrbar_pos_org;

	double chan_param_org;
	double chan_param_cur;

} rxwnd_dragstate_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct rxwnd_ctx {

	/* current window handle */
	HWND hwnd;

	/* common ui resources */
	uicommon_t *uidata;

	uievent_t *event_rx_state;
	uievent_t *event_procchan;
	uievent_t *event_visualcfg;

	/* configuration files */
	ini_data_t *ini, *inich;

	/* receiver state */
	rxstate_t *rx;

	/* frequency config */
	freqcfg_t fcfg;

	/* current cursor */
	HCURSOR hcur_set;
	HCURSOR hcur_actual;

	int is_minimized;

	int is_sessionreload_executed;

	/* dragging state */
	rxwnd_dragstate_t dragstate;

	/* spectrum viewers */
	RECT specview_rect;
	RECT watrview_rect;
	specview_ctx_t *specview;
	watrview_ctx_t *watrview;

	/* main window menus */
	HMENU hmenu_main;
	HMENU hmenu_wnd;
	HMENU hmenu_source;
	HMENU hmenu_ctrl;

	/* accelerator table */
	HACCEL haccel;

	/* sub windows */
	HWND hwndFreqWnd;
	HWND hwndChannelMgr;
	HWND hwndSettings;
	HWND hwndAudCtrl;

	/* window frame size */
	int cx_frame;
	int cy_frame;

} rxwnd_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* rx menu */
#define RXWND_CMD_FREQWND			1001
#define RXWND_CMD_CHANNELS			1002
#define RXWND_CMD_AUDCTRL			1003
#define RXWND_CMD_SETTINGS			1004
#define RXWND_CMD_EXIT				1005

/* source selection menu */
#define RXWND_CMD_INPUT_UNSET		1021
#define RXWND_CMD_INPUT_CONFIG		1022
#define RXWND_CMD_INPUT_0			1023
#define RXWND_CMD_INPUT_MAX			1122

/* control menu */
#define RXWND_CMD_START				1141
#define RXWND_CMD_RESTART			1142

/* ---------------------------------------------------------------------------------------------- */

#define RXWND_USERMSG				(WM_USER + 80)

enum {
	RXWND_USERMSG_RXPROC_ACT_CHANGE		= 880,
};

/* ---------------------------------------------------------------------------------------------- */

#define RXWND_IDT_SESSIONRELOAD		101

/* ---------------------------------------------------------------------------------------------- */

#define RXWND_DEFCLTW				640
#define RXWND_DEFCLTH				480

#define RXWND_MINCLTW				500
#define RXWND_MINCLTH				400

/* ---------------------------------------------------------------------------------------------- */

#define RXWND_SPECVIEW_OFF_TOP		0
#define RXWND_SPECVIEW_OFF_LEFT		0
#define RXWND_SPECVIEW_OFF_RIGHT	0
#define RXWND_SPECVIEW_HEIGHT		(SPECVIEW_FRAME_H + 121)

#define RXWND_WATRVIEW_OFF_TOP		(RXWND_SPECVIEW_OFF_TOP + RXWND_SPECVIEW_HEIGHT)
#define RXWND_WATRVIEW_OFF_LEFT		0
#define RXWND_WATRVIEW_OFF_RIGHT	0
#define RXWND_WATRVIEW_OFF_BOTTOM	0

/* ---------------------------------------------------------------------------------------------- */

int rxwnd_registerclass(uicommon_t *uidata);

void rxwnd_unregisterclass(HINSTANCE h_inst);

HWND rxwnd_create(uicommon_t *uidata, ini_data_t *ini, ini_data_t *inich, int n_show);

/* ---------------------------------------------------------------------------------------------- */
