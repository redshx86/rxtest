/* ---------------------------------------------------------------------------------------------- */

#include "uicommon.h"
#include "../../res/resource.h"

/* ---------------------------------------------------------------------------------------------- */

TCHAR *ui_title = _T("rxtest");

/* ---------------------------------------------------------------------------------------------- */

/* limit client area size when resizing window */
void ui_sizingcheck(int mode, RECT *r, int framew, int frameh, 
					int mincltw, int minclth, int maxcltw, int maxclth)
{
	int cltw, clth, setw, seth;

	/* calculate client area size */
	cltw = r->right - r->left - framew;
	clth = r->bottom - r->top - frameh;

	/* limit client area width */
	setw = 0;
	if((mincltw > 0) && (cltw < mincltw))
	{
		setw = 1;
		cltw = mincltw;
	}
	if((maxcltw > 0) && (cltw > maxcltw))
	{
		setw = 1;
		cltw = maxcltw;
	}

	/* limit client area height */
	seth = 0;
	if((minclth > 0) && (clth < minclth))
	{
		seth = 1;
		clth = minclth;
	}
	if((maxclth > 0) && (clth > maxclth))
	{
		seth = 1;
		clth = maxclth;
	}

	/* set window width */
	if(setw)
	{
		if((mode == WMSZ_LEFT) || (mode == WMSZ_TOPLEFT) || (mode == WMSZ_BOTTOMLEFT)) {
			r->left = r->right - (cltw + framew);
		} else {
			r->right = r->left + (cltw + framew);
		}
	}

	/* set window height */
	if(seth)
	{
		if((mode == WMSZ_TOP) || (mode == WMSZ_TOPLEFT) || (mode == WMSZ_TOPRIGHT)) {
			r->top = r->bottom - (clth + frameh);
		} else {
			r->bottom = r->top + (clth + frameh);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

void ui_savepos(HWND hwnd, ini_sect_t *sect, unsigned int mode, int cx_frame, int cy_frame)
{
	WINDOWPLACEMENT wp;
	int cx, cy, maximized;

	if(GetWindowPlacement(hwnd, &wp))
	{
		if(mode & UI_POS_OFFSET)
		{
			ini_seti(sect, _T("x"), wp.rcNormalPosition.left);
			ini_seti(sect, _T("y"), wp.rcNormalPosition.top);
		}

		if(mode & UI_POS_SIZE)
		{
			cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left - cx_frame;
			cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top - cy_frame;
			ini_seti(sect, _T("cx"), cx);
			ini_seti(sect, _T("cy"), cy);
		}

		if(mode & UI_POS_STATE)
		{
			maximized = (wp.showCmd == SW_SHOWMAXIMIZED);
			ini_setb(sect, _T("maximized"), maximized);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

void ui_calcpos(ini_sect_t *sect, unsigned int mode, int cx_frame, int cy_frame,
				int cx_def, int cy_def, int cx_min, int cy_min,
				int *pwx, int *pwy, int *pwcx, int *pwcy, int *pstate, int userstate)
{
	int cx_scr, cy_scr, cx, cy, wcx, wcy, wx, wy, state, maximized;

	cx_scr = GetSystemMetrics(SM_CXSCREEN);
	cy_scr = GetSystemMetrics(SM_CYSCREEN);

	cx = cx_def;
	cy = cy_def;

	if(mode & UI_POS_SIZE)
	{
		ini_geti(sect, _T("cx"), &cx);
		if(cx < cx_min)
			cx = cx_min;
		if(cx > cx_scr - cx_frame)
			cx = cx_scr - cx_frame;

		ini_geti(sect, _T("cy"), &cy);
		if(cy < cy_min)
			cy = cy_min;
		if(cy > cy_scr - cy_frame)
			cy = cy_scr - cy_frame;
	}

	wcx = cx + cx_frame;
	wcy = cy + cy_frame;

	*pwcx = wcx;
	*pwcy = wcy;

	wx = (cx_scr - wcx) / 2;
	wy = (cy_scr - wcy) / 2;

	if(mode & UI_POS_OFFSET)
	{
		ini_geti(sect, _T("x"), &wx);
		if(wx < 24 - wcx)
			wx = 24 - wcx;
		if(wx > cx_scr - 24)
			wx = cx_scr - 24;

		ini_geti(sect, _T("y"), &wy);
		if(wy < 24 - wcy)
			wy = 24 - wcy;
		if(wy > cy_scr - 24)
			wy = cy_scr - 24;
	}

	*pwx = wx;
	*pwy = wy;

	state = userstate;

	if((state == SW_SHOWNORMAL) && (mode & UI_POS_STATE))
	{
		maximized = 0;
		ini_getb(sect, _T("maximized"), &maximized);

		if(maximized)
			state = SW_SHOWMAXIMIZED;
	}

	*pstate = state;
}

/* ---------------------------------------------------------------------------------------------- */

void ui_frame_size(int *pcx, int *pcy, unsigned int mode)
{
	int cx = 0, cy = 0;

	if(mode & UI_FRAME_SIZING)
	{
		cx += GetSystemMetrics(SM_CXFRAME) * 2;
		cy += GetSystemMetrics(SM_CYFRAME) * 2;
	}

	if(mode & UI_FRAME_FIXED)
	{
		cx += GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
		cy += GetSystemMetrics(SM_CYFIXEDFRAME) * 2;
	}

	if(mode & UI_FRAME_CAPTION)
	{
		cy += GetSystemMetrics(SM_CYCAPTION);
	}

	if(mode & UI_FRAME_MENU)
	{
		cy += GetSystemMetrics(SM_CYMENU);
	}

	*pcx = cx;
	*pcy = cy;
}

/* ---------------------------------------------------------------------------------------------- */

void ui_window_size(int ccx, int ccy, int *pwcx, int *pwcy, unsigned int mode)
{
	int fcx, fcy;

	ui_frame_size(&fcx, &fcy, mode);
	
	*pwcx = ccx + fcx;
	*pwcy = ccy + fcy;
}

/* ---------------------------------------------------------------------------------------------- */

void ui_setwndrect(HWND hwnd, RECT *prc, unsigned int mode)
{
	MoveWindow(hwnd, prc->left, prc->top,
		prc->right - prc->left, prc->bottom - prc->top, FALSE);

	switch(mode)
	{
	case UI_SWR_ERASEANDREDRAW:
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		break;
	case UI_SWR_REDRAWNOERASE:
		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

void ui_updatewndrect(HWND hwnd, RECT *prc_cur, RECT *prc_new, unsigned int mode)
{
	if(memcmp(prc_new, prc_cur, sizeof(RECT)) == 0)
		return;
	ui_setwndrect(hwnd, prc_new, mode);
	memcpy(prc_cur, prc_new, sizeof(RECT));
}

/* ---------------------------------------------------------------------------------------------- */

void ui_set_accel(uicommon_t *uidata, HWND hwnd, HACCEL haccel)
{
	if((haccel == NULL) && (uidata->accel.hwnd == hwnd))
	{
		uidata->accel.hwnd = NULL;
		uidata->accel.haccel = NULL;
	}

	if((haccel != NULL) && (uidata->accel.hwnd == NULL))
	{
		uidata->accel.hwnd = hwnd;
		uidata->accel.haccel = haccel;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* create button control */
HWND ui_crt_btn(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				int x, int y, int w, int h, TCHAR *title, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindow(_T("BUTTON"), title, WS_CHILD|WS_VISIBLE|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create button control (static edge) */
HWND ui_crt_btnse(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				  int x, int y, int w, int h, TCHAR *title, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindowEx(WS_EX_STATICEDGE,
		_T("BUTTON"), title, WS_CHILD|WS_VISIBLE|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create edit box control */
HWND ui_crt_edit(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				 int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		_T("EDIT"), NULL, WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create edit box control (static edge) */
HWND ui_crt_editse(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				   int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindowEx(WS_EX_STATICEDGE,
		_T("EDIT"), NULL, WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create static control */
HWND ui_crt_static(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				   int x, int y, int w, int h, TCHAR *title, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindow(_T("STATIC"), title, WS_CHILD|WS_VISIBLE|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create static control with extended style */
HWND ui_crt_staticex(uicommon_t *uidata, HWND hwnd_owner, DWORD style, DWORD ex_style,
					 int x, int y, int w, int h, TCHAR *title, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindowEx(ex_style, _T("STATIC"), title, WS_CHILD|WS_VISIBLE|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create combo box control */
HWND ui_crt_combo(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				  int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindow(_T("COMBOBOX"), NULL,
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_AUTOHSCROLL|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create track bar control */
HWND ui_crt_track(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
				  int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindow(TRACKBAR_CLASS, NULL, WS_CHILD|WS_VISIBLE|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create progress bar control */
HWND ui_crt_progress(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
					 int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD|WS_VISIBLE|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create listbox control */
HWND ui_crt_listbox(uicommon_t *uidata, HWND hwnd_owner, DWORD style,
					int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindowEx(WS_EX_STATICEDGE, _T("LISTBOX"), NULL,
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|style,
		x, y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);

	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* create listview control */
HWND ui_crt_listview(uicommon_t *uidata, HWND hwnd_owner, DWORD style, DWORD lv_ex_style,
					 int x, int y, int w, int h, UINT uid)
{
	HWND hwnd;

	hwnd = CreateWindowEx(WS_EX_STATICEDGE, WC_LISTVIEW, NULL,
		WS_CHILD|WS_VISIBLE|style,
		x,y, w, h, hwnd_owner, (HMENU)(LONG_PTR)uid, uidata->h_inst, NULL);
	SetWindowFont(hwnd, uidata->hfnt_main, FALSE);
	ListView_SetExtendedListViewStyle(hwnd, lv_ex_style);

	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

void append_str(TCHAR *buf, TCHAR **pptr, size_t bufsize, TCHAR *str)
{
	TCHAR *ptr;
	size_t bufsize_remain;

	if((pptr != NULL) && (*pptr != NULL)) {
		ptr = *pptr;
	} else {
		ptr = buf;
	}

	bufsize_remain = bufsize - (ptr - buf);
	_tcsncpy(ptr, str, bufsize_remain);

	if(pptr != NULL)
	{
		ptr += _tcslen(ptr);
		*pptr = ptr;
	}
}

/* ---------------------------------------------------------------------------------------------- */

void append_sprintf(TCHAR *buf, TCHAR **pptr, size_t bufsize, TCHAR *fmt, ...)
{
	va_list ap;
	TCHAR *ptr;
	size_t bufsize_remain;

	va_start(ap, fmt);

	if((pptr != NULL) && (*pptr != NULL)) {
		ptr = *pptr;
	} else {
		ptr = buf;
	}

	bufsize_remain = bufsize - (ptr - buf);
	_vsntprintf(ptr, bufsize_remain, fmt, ap);

	if(pptr != NULL)
	{
		ptr += _tcslen(ptr);
		*pptr = ptr;
	}

	va_end(ap);
}

/* ---------------------------------------------------------------------------------------------- */

void ui_msg_int_format(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle)
{
	_sntprintf(msgbuf, msgbufsize, _T("%s is not a valid integer value."), valuetitle);
}

/* ---------------------------------------------------------------------------------------------- */

void ui_msg_int_range(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle, TCHAR *valueuint,
					  int valuemin, int valuemax)
{
	TCHAR *ptr = NULL;

	append_sprintf(msgbuf, &ptr, msgbufsize, _T("%s is out of range"), valuetitle);

	if((valuemin != INT_MIN) || (valuemax != INT_MAX))
	{
		append_str(msgbuf, &ptr, msgbufsize, _T(" ("));

		if(valuemin != INT_MIN)
			append_sprintf(msgbuf, &ptr, msgbufsize, _T("min = %d%s"), valuemin, valueuint);

		if(valuemax != INT_MAX)
		{
			if(valuemin != INT_MIN)
				append_str(msgbuf, &ptr, msgbufsize, _T(", "));

			append_sprintf(msgbuf, &ptr, msgbufsize, _T("max = %d%s"), valuemax, valueuint);
		}

		append_str(msgbuf, &ptr, msgbufsize, _T(")"));
	}

	append_str(msgbuf, &ptr, msgbufsize, _T("."));
}

/* ---------------------------------------------------------------------------------------------- */

void ui_set_int(uicommon_t *uidata, HWND hwndEdit, int value)
{
	_itot(value, uidata->databuf, 10);
	SetWindowText(hwndEdit, uidata->databuf);
}

/* ---------------------------------------------------------------------------------------------- */

int ui_get_int(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, int *pvalue, TCHAR *valueTitle)
{
	int value;

	*pvalue = 0;

	if(GetWindowText(hwndEdit, uidata->databuf, (int)(uidata->databuf_size)) <= 0)
	{
		_sntprintf(uidata->msgbuf, uidata->msgbuf_size, _T("%s is not entered."), valueTitle);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(!parse_int(uidata->databuf, &value))
	{
		ui_msg_int_format(uidata->msgbuf, uidata->msgbuf_size, valueTitle);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	*pvalue = value;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int ui_get_int_range(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, int *pvalue,
					 int valueMin, int valueMax, TCHAR *valueTitle, TCHAR *valueUnit)
{
	int value;

	*pvalue = (valueMin + valueMax) / 2;

	if(!ui_get_int(uidata, hwnd, hwndEdit, &value, valueTitle))
		return 0;

	if((value < valueMin) || (value > valueMax))
	{
		ui_msg_int_range(uidata->msgbuf, uidata->msgbuf_size,
			valueTitle, valueUnit, valueMin, valueMax);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	*pvalue = value;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void ui_msg_double_format(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle)
{
	_sntprintf(msgbuf, msgbufsize, _T("%s is not a valid double value."), valuetitle);
}

/* ---------------------------------------------------------------------------------------------- */

void ui_msg_double_range(TCHAR *msgbuf, size_t msgbufsize, TCHAR *valuetitle, TCHAR *valueunit,
						 int useprefix, int initfrac, int minfrac, double vmin, double vmax)
{
	TCHAR *ptr = NULL;

	append_sprintf(msgbuf, &ptr, msgbufsize, _T("%s is out of range"), valuetitle);

	if((vmin != -DBL_MAX) || (vmax != DBL_MAX))
	{
		append_str(msgbuf, &ptr, msgbufsize, _T(" ("));

		if(vmin != -DBL_MAX)
		{
			append_str(msgbuf, &ptr, msgbufsize, _T("min = "));
			append_dbl(msgbuf, &ptr, msgbufsize, vmin, useprefix, initfrac, minfrac);
			append_sprintf(msgbuf, &ptr, msgbufsize, _T("%s"), valueunit);
		}

		if(vmax != DBL_MAX)
		{
			if(vmin != -DBL_MAX)
				append_str(msgbuf, &ptr, msgbufsize, _T(", "));

			append_str(msgbuf, &ptr, msgbufsize, _T("max = "));
			append_dbl(msgbuf, &ptr, msgbufsize, vmax, useprefix, initfrac, minfrac);
			append_sprintf(msgbuf, &ptr, msgbufsize, _T("%s"), valueunit);
		}

		append_str(msgbuf, &ptr, msgbufsize, _T(")"));
	}

	append_str(msgbuf, &ptr, msgbufsize, _T("."));
}

/* ---------------------------------------------------------------------------------------------- */

void ui_set_double(uicommon_t *uidata, HWND hwndEdit, double value,
				   int prefix, int initfrac, int minfrac)
{
	fmt_dbl(uidata->databuf, uidata->databuf_size, value, prefix, initfrac, minfrac);
	SetWindowText(hwndEdit, uidata->databuf);
}

/* ---------------------------------------------------------------------------------------------- */

int ui_get_double(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, double *pvalue,
				  TCHAR *valueTitle, int allow_prefix, int *p_used_prefix)
{
	double value;

	*pvalue = 0;

	if(GetWindowText(hwndEdit, uidata->databuf, (int)(uidata->databuf_size)) <= 0)
	{
		_sntprintf(uidata->msgbuf, uidata->msgbuf_size, _T("%s is not entered."), valueTitle);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(!parse_dbl(uidata->databuf, allow_prefix, &value, p_used_prefix))
	{
		ui_msg_double_format(uidata->msgbuf, uidata->msgbuf_size, valueTitle);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	*pvalue = value;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int ui_get_double_range(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, double *pvalue,
						double valueMin, double valueMax, TCHAR *valueTitle, TCHAR *valueUnit,
						int num_frac, int min_frac, int allow_prefix, int *p_used_prefix)
{
	double value;
	int usedprefix;

	*pvalue = 0.5 * (valueMin + valueMax);

	if(!ui_get_double(uidata, hwnd, hwndEdit, &value, valueTitle, allow_prefix, &usedprefix))
		return 0;

	if((value < valueMin) || (value > valueMax))
	{
		ui_msg_double_range(uidata->msgbuf, uidata->msgbuf_size,
			valueTitle, valueUnit, usedprefix, num_frac, min_frac, valueMin, valueMax);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	*pvalue = value;

	if(p_used_prefix != NULL)
		*p_used_prefix = usedprefix;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void ui_set_rgb(uicommon_t *uidata, HWND hwndEdit, COLORREF cr)
{
	SetWindowText(hwndEdit, fmt_rgb(uidata->databuf, cr));
}

/* ---------------------------------------------------------------------------------------------- */

int ui_get_rgb(uicommon_t *uidata, HWND hwnd, HWND hwndEdit, COLORREF *pcr,  TCHAR *valueTitle)
{
	COLORREF cr;

	*pcr = 0;

	if(GetWindowText(hwndEdit, uidata->databuf, (int)(uidata->databuf_size)) <= 0)
	{
		_sntprintf(uidata->msgbuf, uidata->msgbuf_size, _T("%s is not entered."), valueTitle);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(!parse_rgb(uidata->databuf, &cr))
	{
		_sntprintf(uidata->msgbuf, uidata->msgbuf_size,
			_T("%s is not a valid color."), valueTitle);
		MessageBox(hwnd, uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	*pcr = cr;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set menu item checked and enabled flags */
void ui_menuitemf(HMENU hmenu, UINT ucmd, int checked, int enabled)
{
	MENUITEMINFO mii;

	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState =
		(checked ? MFS_CHECKED : MFS_UNCHECKED) |
		(enabled ? MFS_ENABLED : MFS_GRAYED);
	SetMenuItemInfo(hmenu, ucmd, FALSE, &mii);
}

/* ---------------------------------------------------------------------------------------------- */

/* create font by name and size */
HFONT ui_crt_font(TCHAR *name, int size)
{
	LOGFONT lf;

	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = size;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_TT_PRECIS;
	_tcscpy(lf.lfFaceName, name);
	return CreateFontIndirect(&lf);
}

/* ---------------------------------------------------------------------------------------------- */

void ui_debug(uicommon_t *uidata, TCHAR *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vsntprintf(uidata->msgbuf, uidata->msgbuf_size, fmt, ap);
	OutputDebugString(uidata->msgbuf);
	va_end(ap);
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize common ui resources */
int uicommon_init(uicommon_t *uidata, HINSTANCE h_inst)
{
	INITCOMMONCONTROLSEX icm;

	memset(uidata, 0, sizeof(uicommon_t));

	keynav_init(&(uidata->keynav), h_inst);

	uidata->msgbuf_size = UI_MSGBUF_SIZE;
	uidata->databuf_size = UI_DATABUF_SIZE;

	uidata->msgbuf = malloc((uidata->msgbuf_size + 1) * sizeof(TCHAR));
	uidata->databuf = malloc((uidata->databuf_size + 1) * sizeof(TCHAR));

	if( (uidata->msgbuf != NULL) && (uidata->databuf != NULL) )
	{
		uidata->msgbuf[uidata->msgbuf_size] = 0;
		uidata->databuf[uidata->databuf_size] = 0;

		icm.dwSize = sizeof(icm);
		icm.dwICC = ICC_LISTVIEW_CLASSES|ICC_BAR_CLASSES;
		
		if( InitCommonControlsEx(&icm) )
		{
			uidata->h_inst = h_inst;

			uidata->hicn_main = LoadImage(h_inst,
				MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 32, 32, 0);
			uidata->hicn_sm = LoadImage(h_inst,
				MAKEINTRESOURCE(IDI_MAINICON_SM), IMAGE_ICON, 16, 16, 0);

			uidata->hcur_main = LoadCursor(NULL, IDC_ARROW);
			uidata->hcur_wait = LoadCursor(NULL, IDC_WAIT);
			uidata->hcur_size = LoadCursor(NULL, IDC_SIZEALL);
			uidata->hcur_sizex = LoadCursor(NULL, IDC_SIZEWE);
			uidata->hcur_sizey = LoadCursor(NULL, IDC_SIZENS);

			uidata->hfnt_main = ui_crt_font(_T("Verdana"), 13);
			uidata->hfnt_mono = ui_crt_font(_T("Courier New"), 14);
			uidata->hfnt_mini = ui_crt_font(_T("Tahoma"), 12);
			uidata->hfnt_viewers = ui_crt_font(_T("Verdana"), 12);

			return 1;
		}
	}

	free(uidata->databuf);
	free(uidata->msgbuf);

	keynav_cleanup(&(uidata->keynav));

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* free common ui resources */
void uicommon_free(uicommon_t *uidata)
{
	DeleteObject(uidata->hicn_main);
	DeleteObject(uidata->hicn_sm);

	DeleteObject(uidata->hfnt_viewers);
	DeleteObject(uidata->hfnt_mini);
	DeleteObject(uidata->hfnt_mono);
	DeleteObject(uidata->hfnt_main);

	free(uidata->databuf);
	free(uidata->msgbuf);

	keynav_cleanup(&(uidata->keynav));
}

/* ---------------------------------------------------------------------------------------------- */
