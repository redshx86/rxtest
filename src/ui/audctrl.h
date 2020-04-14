/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"
#include "ctrl/levelbar.h"
#include "specscal.h"
#include "../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct audctrl_clip_ind_ctx {

	RECT rc_left;
	RECT rc_right;

	int state_left;
	int state_right;

	COLORREF cr_text;
	COLORREF cr_border;
	
	COLORREF cr_none;
	COLORREF cr_soft;
	COLORREF cr_hard;
	
	HBRUSH hbr_none;
	HBRUSH hbr_soft;
	HBRUSH hbr_hard;

	HPEN hpen_border;

} audctrl_clip_ind_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct audctrl_ctx {

	HWND hwnd;

	uicommon_t *uidata;
	ini_data_t *ini;
	rxstate_t *rx;

	uievent_t *event_rx_state;
	uievent_t *event_window_close;

	int cx_frame;
	int cy_frame;

	HWND hwndGainTrackBar;
	HWND hwndGainDisplay;
	
	HWND hwndLevelLeft;
	HWND hwndLevelRight;

	audctrl_clip_ind_ctx_t clip_ind;

	int track_range;
	int track_pos;
	
	int level_timer_started;
	unsigned int level_prev_tick;

} audctrl_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define AUDCTRL_LEVEL_UPDATE_INT		100

#define AUDCTRL_OUT_GAIN_MIN			-40.0
#define AUDCTRL_OUT_GAIN_MAX			20.0

#define AUDCTRL_OUT_GAIN_STEPS_PER_DB	2.0

#define AUDCTRL_OUT_LEVEL_MIN			-60.0
#define AUDCTRL_OUT_LEVEL_MAX			20.0

/* ---------------------------------------------------------------------------------------------- */

#define AUDCTRL_CX						240
#define AUDCTRL_CY						135

/* ---------------------------------------------------------------------------------------------- */

#define AUDCTRL_IDT_UPDATE_LEVEL		1

#define AUDCTRL_ID_GAINTRACKBAR			1001
#define AUDCTRL_ID_GAINDISPLAY			1002

#define AUDCTRL_ID_LEVEL_LEFT			1003
#define AUDCTRL_ID_LEVEL_RIGHT			1004

/* ---------------------------------------------------------------------------------------------- */

HWND audctrl_create(uicommon_t *uidata, ini_data_t *ini,
					HWND hwnd_parent, rxstate_t *rx);

/* ---------------------------------------------------------------------------------------------- */

int audctrl_registerclass(uicommon_t *uidata);
void audctrl_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
