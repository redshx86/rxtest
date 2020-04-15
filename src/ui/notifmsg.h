/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>

/* ---------------------------------------------------------------------------------------------- */

#define EVENT_NAME_WNDCLOSE			"window_close"

enum {
	EVENT_WNDCLOSE_FREQWND,
	EVENT_WNDCLOSE_CHANNELMGR,
	EVENT_WNDCLOSE_SETTINGS,
	EVENT_WNDCLOSE_AUDCTRL
};

/* ---------------------------------------------------------------------------------------------- */

#define EVENT_NAME_RX_STATE			"rx_state_change"

enum {
	EVENT_RX_STATE_START,
	EVENT_RX_STATE_STOP,
	EVENT_RX_STATE_SET_CFG,
	EVENT_RX_STATE_SET_INPUT_FC,
	EVENT_RX_STATE_SET_OUT_GAIN,
	EVENT_RX_STATE_SET_DISPLAY_F_RANGE
};

/* ---------------------------------------------------------------------------------------------- */

#define EVENT_NAME_PROCCHAN			"proc_state_change"

enum {
	EVENT_PROCCHAN_CREATE,
	EVENT_PROCCHAN_DELETE,
	EVENT_PROCCHAN_NAME,
	EVENT_PROCCHAN_FREQ,
	EVENT_PROCCHAN_DECIMATION,
	EVENT_PROCCHAN_FILTERCFG,
	EVENT_PROCCHAN_DEMODCFG,
	EVENT_PROCCHAN_SQLCFG,
	EVENT_PROCCHAN_OUTPUTCFG,
	EVENT_PROCCHAN_ACTIVITY,
};

typedef struct notify_proc_list {
	int count;
	unsigned int *items;
} notify_proc_list_t;

/* ---------------------------------------------------------------------------------------------- */

#define EVENT_NAME_CHANMGRCFG		"chanmgr_config"

enum {
	EVENT_CHANMGR_SETCONFIG
};

/* ---------------------------------------------------------------------------------------------- */
