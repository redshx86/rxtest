/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"
#include "cmcfg.h"
#include "proccfg/chanopt.h"
#include "proccfg/filtcfg.h"
#include "proccfg/sqlcfg.h"
#include "proccfg/fmcfg.h"
#include "ctrl/levelbar.h"
#include "../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

/* selected row list */
typedef struct cmwnd_rowlist {
	int capacity;
	int count;
	int *row;
} cmwnd_rowlist_t;

/* ---------------------------------------------------------------------------------------------- */

/* channel list icon data */
typedef struct cmwnd_cl_imglist_data {
	HIMAGELIST h_img_list;							/* imagelist handle */
	HICON hicon_proc_status[RXPROC_STATUS_COUNT];	/* loaded icon handles */
	int img_proc_status[RXPROC_STATUS_COUNT];		/* imagelist indices */
} cmwnd_cl_imglist_data_t;

/* ---------------------------------------------------------------------------------------------- */

/* channel manager window context */
typedef struct cmwnd_ctx {

	/* parent window handle */
	HWND hwndMain;

	/* current window handle */
	HWND hwnd;

	/* common ui resources */
	uicommon_t *uidata;

	/* notification recepient window list */
	callback_list_t *cb_list;

	/* configuration files */
	ini_data_t *ini, *inich;

	/* receiver state */
	rxstate_t *rx;

	/* row index list for various purposes */
	cmwnd_rowlist_t rowlist;

	/* channel identifier list for notifications */
	int chan_notify_list_capacity;
	notify_proc_list_t chan_notify_list;

	/* controls */
	HWND hwndBtnClear;					/* clear list button */
	HWND hwndBtnConfig;					/* config button */

	/* preset storing controls */
	HWND hwndComboPreset;				/* preset selection combo box */
	HWND hwndBtnLoadPreset;				/* preset load button */
	HWND hwndBtnPresetMenu;				/* preset menu button */
	HMENU hmenuPresetCommands;			/* preset commands menu */

	/* frequency adjustment controls */
	HWND hwndComboFreqStep;				/* frequency adjustment step combo box */
	HWND hwndBtnFreqUp;					/* frequency increment button */
	HWND hwndBtnFreqDn;					/* frequency decrement button */
	HWND hwndBtnFreqSnap;				/* frequency grid snap button */

	/* channel list view data */
	HWND hwndChList;					/* list view handle */
	HWND hwndChListHdr;					/* list view header handle */
	WNDPROC wndprocChList;				/* original window proc */
	cmwnd_cl_imglist_data_t cl_imglist_data; /* channel list icon data */

	/* cell editing data */
	int editRow;						/* cell row index */
	int editCol;						/* cell column index */
	HWND hwndEdit;						/* control handle */
	RECT rcEdit;						/* control bounding rect */
	WNDPROC wndprocEdit;				/* original control window proc */

	/* demodulator type selection menu */
	HMENU hmenuDemodType;				/* menu handle */
	int demodSelectRow;					/* affected row index */

	/* channel list popup menus */
	HMENU hmenuChListPopup;				/* not channel */
	HMENU hmenuChanPopup;				/* channel */

	/* frequency display unit */
	int dispUnitChanFreq;				/* for channel frequency */
	int dispUnitFiltCutoff;				/* for filter cutoff freq */

	/* level meters config */
	double levelMin;					/* minimum level meter pos, dB */
	double levelMax;					/* maximum level meter pos, dB */
	unsigned int levelUpdateInt;		/* level meter update interval */
	int levelScaleToInputFc;			/* level meter mode */

	/* level meter state */
	int levelIsStarted;					/* level meter timer is started */

	/* window frame size */
	int cx_frame;
	int cy_frame;

} cmwnd_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* channel row data */
typedef struct cmwnd_rowdata {

	/* channel data */
	unsigned int chid;			/* channel id */

	/* base control identifier */
	UINT uIdBase;

	/* current status icon */
	rxproc_status_t status_icon_cur;

	/* misc options button */
	HWND hwndBtnOptions;
	RECT rcBtnOptions;

	/* filter options button */
	HWND hwndBtnFilterCfg;
	RECT rcBtnFilterCfg;

	/* demodulator type selection button */
	HWND hwndBtnDemodType;
	RECT rcBtnDemodType;

	/* demodulator options button */
	HWND hwndBtnDemodCfg;
	RECT rcBtnDemodCfg;

	/* squelch config button */
	HWND hwndBtnSqlCfg;
	RECT rcBtnSqlCfg;

	/* output mode selection button */
	HWND hwndBtnOutMode;
	RECT rcBtnOutMode;

	/* level meter */
	HWND hwndLevelBar;
	RECT rcLevelBar;

} cmwnd_rowdata_t;

/* ---------------------------------------------------------------------------------------------- */

#define CMWND_CLCOL_CXMARGIN		6

/* channel list columns indices */
typedef enum cmwnd_clcol {

	CMWND_CLCOL_NAME,
	CMWND_CLCOL_FREQ,
	CMWND_CLCOL_FILTER,
	CMWND_CLCOL_DEMOD,
	CMWND_CLCOL_SQUELCH,
	CMWND_CLCOL_AUDIO,
	CMWND_CLCOL_LEVEL,

	CMWND_CLCOL_COUNT

} cmwnd_clcol_t;

/* ---------------------------------------------------------------------------------------------- */

/* channel list column widths */
typedef struct cmwnd_clcol_cx {
	int cx_def;
	int cx_min;
	int cx_max;
	int cx_add;
	TCHAR *name;
} cmwnd_clcol_cx_t;

/* ---------------------------------------------------------------------------------------------- */

/* control identifiers */
#define CMWND_ID_CHLIST				1001
#define CMWND_ID_CHLIST_EDIT		1002

#define CMWND_ID_CLEARBTN			1021
#define CMWND_ID_CONFIGBTN			1022

#define CMWND_ID_PRESET_LIST		1041
#define CMWND_ID_PRESET_LOAD		1042
#define CMWND_ID_PRESET_MENU		1043
#define CMWND_ID_PRESET_APPEND		1044
#define CMWND_ID_PRESET_SAVE		1045
#define CMWND_ID_PRESET_DELETE		1046

#define CMWND_ID_FREQSTEPSEL		1061
#define CMWND_ID_FREQSTEPUP			1062
#define CMWND_ID_FREQSTEPDOWN		1063
#define CMWND_ID_FREQSTEPSNAP		1064

#define CMWND_ID_INSERTCHAN			1081
#define CMWND_ID_MOVEUP				1082
#define CMWND_ID_MOVEDOWN			1083
#define CMWND_ID_DELETECHANS		1084
#define CMWND_ID_CHANOPT			1085
#define CMWND_ID_FILTERCFG			1086
#define CMWND_ID_DEMODCFG			1087
#define CMWND_ID_SQLCFG				1088

#define CMWND_ID_OUTGAIN_3			1101
#define CMWND_ID_OUTGAIN_0			1102
#define CMWND_ID_OUTGAIN__3			1103
#define CMWND_ID_OUTGAIN__6			1104
#define CMWND_ID_OUTGAIN__9			1105
#define CMWND_ID_OUTGAIN__12		1106
#define CMWND_ID_OUTGAIN__15		1107

#define CMWND_ID_OUTCHANNONE		1161
#define CMWND_ID_OUTCHANA			1162
#define CMWND_ID_OUTCHANB			1163
#define CMWND_ID_OUTCHANAB			1164

#define CMWND_ID_DEMODTYPE_0		1241
#define CMWND_ID_DEMODTYPE_MAX		1304

#define CMWND_ID_ROWCTL_0			1321
#define CMWND_ID_ROWCTL_MAX			65535

/* row control id offsets */
enum {
	CMWND_ID_ROWCTLS_OPTIONS,
	CMWND_ID_ROWCTLS_FILTCFG,
	CMWND_ID_ROWCTLS_DEMODTYPE,
	CMWND_ID_ROWCTLS_DEMODCFG,
	CMWND_ID_ROWCTLS_SQLCFG,
	CMWND_ID_ROWCTLS_OUTMODE,
	CMWND_ID_ROWCTLS_LEVELBAR,

	CMWND_ROWCTLS_NUM_IDS
};

/* ---------------------------------------------------------------------------------------------- */

#define CMWND_IDT_UPDATELEVEL		1

/* ---------------------------------------------------------------------------------------------- */

#define CMWND_DEFCLTW				500
#define CMWND_DEFCLTH				200

#define CMWND_MINCLTW				490
#define CMWND_MAXCLTW				600

#define CMWND_MINCLTH				80

#define CMWND_CL_OFS_BTM			24
#define CMWND_BTM_CTLS_OFS			21

/* ---------------------------------------------------------------------------------------------- */

/* create channel manager window */
HWND cmwnd_create(uicommon_t *uidata, ini_data_t *ini, ini_data_t *inich,
				  rxstate_t *rx, HWND hwndMain, callback_list_t *cb_list);

/* register channel manager window class */
int cmwnd_registerclass(uicommon_t *uidata);

/* unregister channel manager window class */
void cmwnd_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
