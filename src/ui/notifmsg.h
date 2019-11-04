/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include "../util/callback.h"

/* ---------------------------------------------------------------------------------------------- */

#define NOTIFY_WNDCLOSE					0

enum {
	NOTIFY_WNDCLOSE_FREQWND,
	NOTIFY_WNDCLOSE_CHANNELMGR,
	NOTIFY_WNDCLOSE_SETTINGS,
	NOTIFY_WNDCLOSE_AUDCTRL
};

/* ---------------------------------------------------------------------------------------------- */

#define NOTIFY_RX						1

enum {
	NOTIFY_RX_START,
	NOTIFY_RX_STOP,
	NOTIFY_RX_SET_INPUT_FC,
	NOTIFY_RX_SET_OUT_GAIN
};

/* ---------------------------------------------------------------------------------------------- */

#define NOTIFY_PROCCHAN					2

enum {
	NOTIFY_PROCCHAN_CREATE,
	NOITFY_PROCCHAN_DELETE,
	NOTIFY_PROCCHAN_NAME,
	NOTIFY_PROCCHAN_FREQ,
	NOTIFY_PROCCHAN_DECIMATION,
	NOTIFY_PROCCHAN_FILTERCFG,
	NOTIFY_PROCCHAN_DEMODCFG,
	NOTIFY_PROCCHAN_SQLCFG,
	NOTIFY_PROCCHAN_OUTPUTCFG,
	NOTIFY_PROCCHAN_ACTIVITY,
};

typedef struct notify_proc_list {
	int count;
	unsigned int *items;
} notify_proc_list_t;

/* ---------------------------------------------------------------------------------------------- */

#define NOTIFY_VISUALCFG			3

enum {
	NOTIFY_VISUALCFG_SPECVIEW,
	NOTIFY_VISUALCFG_WATRVIEW,
	NOTIFY_VISUALCFG_FREQ_RANGE
};

/* ---------------------------------------------------------------------------------------------- */

#define NOTIFY_CHANMGR				4
enum {
	NOTIFY_CHANMGR_SETCONFIG
};

/* ---------------------------------------------------------------------------------------------- */
