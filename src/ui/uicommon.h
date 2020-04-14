/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <crtdbg.h>
#include "../util/strutil.h"
#include "../util/numparse.h"
#include "../util/iniparse.h"
#include "../util/macro.h"
#include "uievent.h"
#include "notifmsg.h"
#include "keynav.h"

/* ---------------------------------------------------------------------------------------------- */

#define UI_MSGBUF_SIZE		4095
#define UI_DATABUF_SIZE		1023

#define ID_CTL_STATIC		(UINT)(-1)

/* ---------------------------------------------------------------------------------------------- */

extern TCHAR *ui_title;

/* ---------------------------------------------------------------------------------------------- */

typedef struct uicommon_accel {
	HWND hwnd;
	HACCEL haccel;
} uicommon_accel_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct uicommon {

	uievent_list_t event_list;

	keynav_ctx_t keynav;
	uicommon_accel_t accel;

	TCHAR *msgbuf;
	size_t msgbuf_size;
	TCHAR *databuf;
	size_t databuf_size;

	HINSTANCE h_inst;

	HICON hicn_main;
	HICON hicn_sm;
	HCURSOR hcur_main;
	HCURSOR hcur_wait;
	HCURSOR hcur_size;
	HCURSOR hcur_sizex;
	HCURSOR hcur_sizey;

	HFONT hfnt_main;
	HFONT hfnt_mono;
	HFONT hfnt_mini;
	HFONT hfnt_viewers;

} uicommon_t;

/* ---------------------------------------------------------------------------------------------- */

/* limit client area size when resizing window */
void ui_sizingcheck(int mode, RECT *r, int framew, int frameh, 
					int mincltw, int minclth, int maxcltw, int maxclth);

#define UI_POS_OFFSET		0x0001
#define UI_POS_SIZE			0x0002
#define UI_POS_STATE		0x0004
#define UI_POS_ALL			(UI_POS_OFFSET|UI_POS_SIZE|UI_POS_STATE)

void ui_savepos(HWND hwnd, ini_sect_t *sect, unsigned int mode, int cx_frame, int cy_frame);

void ui_calcpos(ini_sect_t *sect, unsigned int mode, int cx_frame, int cy_frame,
				int cx_def, int cy_def, int cx_min, int cy_min,
				int *pwx, int *pwy, int *pwcx, int *pwcy, int *pstate, int userstate);

#define UI_FRAME_SIZING		0x0001
#define UI_FRAME_FIXED		0x0002
#define UI_FRAME_CAPTION	0x0004
#define UI_FRAME_MENU		0x0008

void ui_frame_size(int *pcx, int *pcy, unsigned int mode);
void ui_window_size(int ccx, int ccy, int *pwcx, int *pwcy, unsigned int mode);

#define UI_SWR_NOREDRAW			0
#define UI_SWR_ERASEANDREDRAW	1
#define UI_SWR_REDRAWNOERASE	2

void ui_setwndrect(HWND hwnd, RECT *prc, unsigned int mode);

void ui_updatewndrect(HWND hwnd, RECT *prc_cur, RECT *prc_new, unsigned int mode);

/* ---------------------------------------------------------------------------------------------- */

void ui_set_accel(uicommon_t *uidata, HWND hwnd, HACCEL haccel);

/* ---------------------------------------------------------------------------------------------- */

/* create button control */
HWND ui_crt_btn(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				int x, int y, int w, int h, TCHAR *title, UINT uid);

/* create button control (static edge) */
HWND ui_crt_btnse(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				  int x, int y, int w, int h, TCHAR *title, UINT uid);

/* create edit box control */
HWND ui_crt_edit(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				 int x, int y, int w, int h, UINT uid);

/* create edit box control (static edge) */
HWND ui_crt_editse(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				   int x, int y, int w, int h, UINT uid);

/* create static control */
HWND ui_crt_static(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				   int x, int y, int w, int h, TCHAR *title, UINT uid);

/* create static control with extended style */
HWND ui_crt_staticex(uicommon_t *uidata, HWND hwnd_owner, DWORD style, DWORD ex_style,
					 int x, int y, int w, int h, TCHAR *title, UINT uid);

/* create combo box control */
HWND ui_crt_combo(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				  int x, int y, int w, int h, UINT uid);

/* create track bar control */
HWND ui_crt_track(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				  int x, int y, int w, int h, UINT uid);

/* create progress bar control */
HWND ui_crt_progress(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
					 int x, int y, int w, int h, UINT uid);

/* create listbox control */
HWND ui_crt_listbox(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
					int x, int y, int w, int h, UINT uid);

/* create listview control */
HWND ui_crt_listview(uicommon_t *uidata, HWND hwnd_owner, DWORD style, DWORD lv_ex_style,
					 int x, int y, int w, int h, UINT uid);

/* ---------------------------------------------------------------------------------------------- */

void ui_msg_int_format(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle);

void ui_msg_int_range(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle, TCHAR *valueuint,
					  int valuemin, int valuemax);

void ui_set_int(uicommon_t *uidata, HWND hwndEdit, int value);

int ui_get_int(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, int *pvalue, TCHAR *valueTitle);

int ui_get_int_range(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, int *pvalue,
					 int valueMin, int valueMax, TCHAR *valueTitle, TCHAR *valueUnit);

/* ---------------------------------------------------------------------------------------------- */

void ui_msg_double_format(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle);

void ui_msg_double_range(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle, TCHAR *valueunit,
						 int useprefix, int initfrac, int minfrac, double vmin, double vmax);

void ui_set_double(uicommon_t *uidata, HWND hwndEdit, double value,
				   int prefix, int initfrac, int minfrac);

int ui_get_double(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, double *pvalue,
				  TCHAR *valueTitle, int allow_prefix, int *p_used_prefix);

int ui_get_double_range(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, double *pvalue,
						double valueMin, double valueMax, TCHAR *valueTitle, TCHAR *valueUnit,
						int num_frac, int min_frac, int allow_prefix, int *p_used_prefix);

/* ---------------------------------------------------------------------------------------------- */

void ui_set_rgb(uicommon_t *uidata, HWND hwndEdit, COLORREF cr);

int ui_get_rgb(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, COLORREF *pcr,  TCHAR *valueTitle);

/* ---------------------------------------------------------------------------------------------- */

/* set menu item checked and enabled flags */
void ui_menuitemf(HMENU hmenu, UINT ucmd, int checked, int enabled);

/* ---------------------------------------------------------------------------------------------- */

/* create font by name and size */
HFONT ui_crt_font(TCHAR *name, int size);

/* ---------------------------------------------------------------------------------------------- */

void ui_debug(uicommon_t *uidata, TCHAR *fmt, ...);

/* ---------------------------------------------------------------------------------------------- */

__inline
void *GetWndPtr(HWND hwnd, int code)
{
	return (void*)(LONG_PTR)GetWindowLongPtr(hwnd, code);
}

#pragma warning(push)
#pragma warning(disable: 4244)

__inline
void *SetWndPtr(HWND hwnd, int code, void *ptr)
{
	return (void*)(LONG_PTR)SetWindowLongPtr(hwnd, code, (LONG_PTR)ptr);
}

#pragma warning(pop)

/* ---------------------------------------------------------------------------------------------- */

/* initialize common ui resources */
int uicommon_init(uicommon_t *uidata, HINSTANCE h_inst);

/* free common ui resources */
void uicommon_free(uicommon_t *uidata);

/* ---------------------------------------------------------------------------------------------- */
