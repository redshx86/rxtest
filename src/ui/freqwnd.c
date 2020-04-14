/* ---------------------------------------------------------------------------------------------- */

#include "freqwnd.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *freqwnd_classname = _T("rxtest_freqwnd");
static TCHAR *freqwnd_name = _T("Frequency");

static ATOM freqwnd_atom;

/* ---------------------------------------------------------------------------------------------- */

static double freqwnd_dispsnap_tbl[] =
{
	    1e3,
	  2.5e3,
	3.125e3,
	    5e3,
	 6.25e3,
	   10e3,
	 12.5e3,
	   20e3,
	   25e3,
	   50e3,
	  100e3
};

/* ---------------------------------------------------------------------------------------------- */

static double freqwnd_marksnap_tbl[] =
{
	   10e3,
	   20e3,
	   25e3,
	   50e3,
	  100e3,
	  200e3,
	  250e3,
	  500e3,
	    1e6,
	    2e6,
	  2.5e6,
	    5e6,
	   10e6
};

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_parse_freq(freqwnd_ctx_t *ctx, HWND hwndEdit, TCHAR *title,
							  double *p_out, int *p_displayunit)
{
	double value;
	int displayunit;

	if( !ui_get_double(ctx->uidata, ctx->hwnd, hwndEdit, &value, title, 1, &displayunit) )
		return 0;

	if(!INRANGE_MINMAX(value, RXLIM_FC))
	{
		_stprintf(ctx->uidata->msgbuf, _T("%s is out of limit."), title);
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	*p_out = value;
	*p_displayunit = displayunit;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_loadinputcenterfreq(freqwnd_ctx_t *ctx)
{
	if( ctx->rx->is_started &&
		(ctx->rx->input_mod_desc->fn_set_fc != NULL) )
	{
		ui_set_double(ctx->uidata, ctx->hwndEditFIn, ctx->rx->fc_input,
			ctx->prefixFIn, 3, (ctx->prefixFIn >= 2) ? 3 : 0);

		if(!ctx->isFInSelEnabled)
		{
			EnableWindow(ctx->hwndEditFIn, TRUE);
			EnableWindow(ctx->hwndBtnFInSet, TRUE);
			ctx->isFInSelEnabled = 1;
		}
	}
	else
	{
		if(ctx->rx->is_started) {
		ui_set_double(ctx->uidata, ctx->hwndEditFIn, ctx->rx->fc_input,
			ctx->prefixFIn, 3, (ctx->prefixFIn >= 2) ? 3 : 0);
		} else {
			Edit_SetText(ctx->hwndEditFIn, _T(""));
		}

		if(ctx->isFInSelEnabled)
		{
			EnableWindow(ctx->hwndEditFIn, FALSE);
			EnableWindow(ctx->hwndBtnFInSet, FALSE);
			ctx->isFInSelEnabled = 0;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_loaddispfreqrange(freqwnd_ctx_t *ctx)
{
	ui_set_double(ctx->uidata, ctx->hwndEditDispF0,
		ctx->fcfg->disp_f_0, ctx->prefixFDisp0, 3, 0);

	ui_set_double(ctx->uidata, ctx->hwndEditDispF1,
		ctx->fcfg->disp_f_1, ctx->prefixFDisp1, 3, 0);
	
	ui_set_double(ctx->uidata, ctx->hwndCbFDispLen,
		ctx->fcfg->disp_f_1 - ctx->fcfg->disp_f_0, ctx->prefixFDispLen, 3, 0);
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_rx_state_handler(freqwnd_ctx_t *ctx, unsigned int msg, void *data)
{
	switch(msg)
	{
	case EVENT_RX_STATE_START:
	case EVENT_RX_STATE_STOP:
	case EVENT_RX_STATE_SET_INPUT_FC:
		freqwnd_loadinputcenterfreq(ctx);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_visualcfg_handler(freqwnd_ctx_t *ctx, unsigned int msg, void *data)
{
	switch(msg)
	{
	case EVENT_VISUALCFG_FREQ_RANGE:
		freqwnd_loaddispfreqrange(ctx);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_setinputcenterfreq(freqwnd_ctx_t *ctx, double fc_input)
{
	int res;

	res = rx_set_input_fc(ctx->rx, &fc_input,
		ctx->uidata->msgbuf, ctx->uidata->msgbuf_size);

	if(!res)
	{
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title,
			MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(res > 0)
	{
		uievent_send(ctx->event_rx_state, EVENT_RX_STATE_SET_INPUT_FC, NULL);
	}
	else
	{
		freqwnd_loadinputcenterfreq(ctx);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_setdispfreqrange(freqwnd_ctx_t *ctx, double f_0, double f_1)
{
	int is_updated = 0;

	ctx->fcfg->disp_f_0 = f_0;
	ctx->fcfg->disp_f_1 = f_1;

	ctx->specview->f_0 = f_0;
	ctx->specview->f_1 = f_1;
	if(specview_set_freq_range(ctx->specview) > 0)
		is_updated = 1;

	if(watrview_set_freq_range(ctx->watrview, f_0, f_1) > 0)
		is_updated = 1;

	if(is_updated)
	{
		uievent_send(ctx->event_visualcfg, EVENT_VISUALCFG_FREQ_RANGE, NULL);
	}
	else
	{
		freqwnd_loaddispfreqrange(ctx);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_checkdispfreqrange(freqwnd_ctx_t *ctx, double f_0, double f_1)
{
	if(f_1 <= f_0)
	{
		MessageBox(ctx->hwnd, _T("Entered frequency range is empty."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(f_1 - f_0 < 1e3)
	{
		MessageBox(ctx->hwnd, _T("Entered frequency range is too small."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(f_1 - f_0 > 1e9)
	{
		MessageBox(ctx->hwnd, _T("Entered frequency range is too large."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_cmd_changeinputcenterfreq(freqwnd_ctx_t *ctx)
{
	double fc_input;

	if(!freqwnd_parse_freq(ctx, ctx->hwndEditFIn,
		_T("Input center frequency"), &fc_input, &(ctx->prefixFIn)))
	{
		return 0;
	}

	return freqwnd_setinputcenterfreq(ctx, fc_input);
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_cmd_changedispfreqrange(freqwnd_ctx_t *ctx)
{
	double f_0, f_1;

	if( ( !freqwnd_parse_freq(ctx, ctx->hwndEditDispF0,
			_T("Lower display frequency"), &f_0, &(ctx->prefixFDisp0)) ) ||
		( !freqwnd_parse_freq(ctx, ctx->hwndEditDispF1,
			_T("Upper display frequency"), &f_1, &(ctx->prefixFDisp1)) ) )
	{
		return 0;
	}

	if(!freqwnd_checkdispfreqrange(ctx, f_0, f_1))
		return 0;

	return freqwnd_setdispfreqrange(ctx, f_0, f_1);
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_cmd_changedispfreqlen(freqwnd_ctx_t *ctx)
{
	double f_0, f_1, f_len, f_c;

	f_0 = ctx->fcfg->disp_f_0;
	f_1 = ctx->fcfg->disp_f_1;

	if( !freqwnd_parse_freq(ctx, ctx->hwndCbFDispLen,
		_T("Display frequency range"), &f_len, &(ctx->prefixFDispLen)) )
	{
		return 0;
	}

	if(Button_GetCheck(ctx->hwndRbFDispLenL) & BST_CHECKED)
	{
		f_1 = f_0 + f_len;
	}
	else if(Button_GetCheck(ctx->hwndRbFDispLenR) & BST_CHECKED)
	{
		f_0 = f_1 - f_len;
	}
	else if(Button_GetCheck(ctx->hwndRbFDispLenC) & BST_CHECKED)
	{
		f_c = 0.5 * (f_0 + f_1);
		f_0 = f_c - 0.5 * f_len;
		f_1 = f_c + 0.5 * f_len;
	}

	if(!freqwnd_checkdispfreqrange(ctx, f_0, f_1))
		return 0;

	return freqwnd_setdispfreqrange(ctx, f_0, f_1);
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_updatedispsnap(freqwnd_ctx_t *ctx)
{
	double snap_value;
	int snap_unit;

	ComboBox_GetText(ctx->hwndCbDispSnap,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

	if( (!parse_dbl(ctx->uidata->databuf, 1, &snap_value, &snap_unit)) ||
		(snap_value < 0) )
	{
		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
			ctx->fcfg->f_disp_range_snap, ctx->prefixFDispSnap, 3, 0);
		ComboBox_SetText(ctx->hwndCbDispSnap, ctx->uidata->databuf);
	}
	else
	{
		ctx->fcfg->f_disp_range_snap = snap_value;
		ctx->prefixFDispSnap = snap_unit;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_updatemarksnap(freqwnd_ctx_t *ctx)
{
	double snap_value;
	int snap_unit;

	ComboBox_GetText(ctx->hwndCbMarkSnap,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

	if( (!parse_dbl(ctx->uidata->databuf, 1, &snap_value, &snap_unit)) ||
		(snap_value < 0) )
	{
		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
			ctx->fcfg->f_mark_snap, ctx->prefFMarkSnap, 3, 0);
		ComboBox_SetText(ctx->hwndCbMarkSnap, ctx->uidata->databuf);
	}
	else
	{
		ctx->fcfg->f_mark_snap = snap_value;
		ctx->prefFMarkSnap = snap_unit;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_loadparams(freqwnd_ctx_t *ctx)
{
	ini_sect_t *sect;
	int flenanchor;

	sect = ini_sect_get(ctx->ini, _T("freqwnd"), 0);

	flenanchor = 0;
	ini_geti(sect, _T("anchor"), &flenanchor);
	switch(flenanchor)
	{
	case 0: Button_SetCheck(ctx->hwndRbFDispLenC, BST_CHECKED); break;
	case 1: Button_SetCheck(ctx->hwndRbFDispLenL, BST_CHECKED); break;
	case 2: Button_SetCheck(ctx->hwndRbFDispLenR, BST_CHECKED); break;
	}
	
	ctx->prefixFIn = 2;
	ini_geti(sect, _T("unit_f_in"), &(ctx->prefixFIn));

	ctx->prefixFDisp0 = 2;
	ctx->prefixFDisp1 = 2;
	ctx->prefixFDispLen = 1;
	ini_geti(sect, _T("unit_f_0"), &(ctx->prefixFDisp0));
	ini_geti(sect, _T("unit_f_1"), &(ctx->prefixFDisp1));
	ini_geti(sect, _T("unit_f_len"), &(ctx->prefixFDispLen));

	ctx->prefixFDispSnap = 1;
	ctx->prefFMarkSnap = 1;
	ini_geti(sect, _T("unit_disp_snap"), &(ctx->prefixFDispSnap));
	ini_geti(sect, _T("unit_mark_snap"), &(ctx->prefFMarkSnap));
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_saveparams(freqwnd_ctx_t *ctx)
{
	ini_sect_t *sect;
	int flenanchor;

	sect = ini_sect_get(ctx->ini, _T("freqwnd"), 1);

	if(Button_GetCheck(ctx->hwndRbFDispLenL) & BST_CHECKED)
		flenanchor = 1;
	else if(Button_GetCheck(ctx->hwndRbFDispLenR) & BST_CHECKED)
		flenanchor = 2;
	else
		flenanchor = 0;
	ini_seti(sect, _T("anchor"), flenanchor);

	ini_seti(sect, _T("unit_f_in"), ctx->prefixFIn);

	ini_seti(sect, _T("unit_f_0"), ctx->prefixFDisp0);
	ini_seti(sect, _T("unit_f_1"), ctx->prefixFDisp1);
	ini_seti(sect, _T("unit_f_len"), ctx->prefixFDispLen);

	ini_seti(sect, _T("unit_disp_snap"), ctx->prefixFDispSnap);
	ini_seti(sect, _T("unit_mark_snap"), ctx->prefFMarkSnap);
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_initstepcb(freqwnd_ctx_t *ctx, HWND hwndCb, double *step, int numstep)
{
	int i, pref;

	for(i = 0; i < numstep; i++)
	{
		if( (step[i] + 1e-3) >= 1e6 ) {
			pref = 2;
		} else if( (step[i] + 1e-3) >= 1e3 ) {
			pref = 1;
		} else {
			pref = 0;
		}

		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, step[i], pref, 3, 0);
		ComboBox_AddString(hwndCb, ctx->uidata->databuf);
	}
}

/* ---------------------------------------------------------------------------------------------- */

LRESULT CALLBACK freqwnd_inputfc_edit_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	freqwnd_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_ESCAPE:
			freqwnd_loadinputcenterfreq(ctx);
			return 0;
		case VK_RETURN:
			freqwnd_cmd_changeinputcenterfreq(ctx);
			return 0;
		}
		break;
	}

	return CallWindowProc(ctx->wndprocEditFIn, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

LRESULT CALLBACK freqwnd_dispf0_edit_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	freqwnd_ctx_t *ctx;
	int pos;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_ESCAPE:
			ui_set_double(ctx->uidata, ctx->hwndEditDispF0,
				ctx->fcfg->disp_f_0, ctx->prefixFDisp0, 3, 0);
			Edit_SetSel(ctx->hwndEditDispF0, 0, -1);
			return 0;
		case VK_RETURN:
			pos = (short)Edit_GetSel(ctx->hwndEditDispF0);
			if(freqwnd_cmd_changedispfreqrange(ctx))
				Edit_SetSel(ctx->hwndEditDispF0, pos, pos);
			return 1;
		}
		break;
	}

	return CallWindowProc(ctx->wndprocEditDispF0, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

LRESULT CALLBACK freqwnd_dispf1_edit_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	freqwnd_ctx_t *ctx;
	int pos;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_ESCAPE:
			ui_set_double(ctx->uidata, ctx->hwndEditDispF1,
				ctx->fcfg->disp_f_1, ctx->prefixFDisp1, 3, 0);
			Edit_SetSel(ctx->hwndEditDispF1, 0, -1);
			return 0;
		case VK_RETURN:
			pos = (short)Edit_GetSel(ctx->hwndEditDispF1);
			if(freqwnd_cmd_changedispfreqrange(ctx))
				Edit_SetSel(ctx->hwndEditDispF1, pos, pos);
		}
		break;
	}

	return CallWindowProc(ctx->wndprocEditDispF1, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

static int freqwnd_init(freqwnd_ctx_t *ctx)
{
	int nstep;

	/* create input center frequency controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 185, 15, _T("Input center frequency:"), ID_CTL_STATIC);
	ctx->hwndEditFIn = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_DISABLED|WS_TABSTOP,
		10, 30, 100, 21, FREQWND_ID_F_IN);
	ctx->hwndBtnFInSet = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_DISABLED|WS_TABSTOP,
		115, 30, 45, 21, _T("Set"), FREQWND_ID_F_IN_SET);

	SetWndPtr(ctx->hwndEditFIn, GWLP_USERDATA, ctx);
	
	ctx->wndprocEditFIn = (WNDPROC)SetWndPtr(ctx->hwndEditFIn,
		GWLP_WNDPROC, (void*)freqwnd_inputfc_edit_proc);


	/* create display frequency range controls (end points) */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 65, 185, 15, _T("Display frequency range:"), ID_CTL_STATIC);
	ctx->hwndEditDispF0 = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 85, 100, 21, FREQWND_ID_F_DISP_0);
	ui_crt_static(ctx->uidata, ctx->hwnd, SS_CENTER,
		110, 88, 15, 15, _T("–"), ID_CTL_STATIC);
	ctx->hwndEditDispF1 = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		125, 85, 100, 21, FREQWND_ID_F_DISP_1);
	ctx->hwndBtnFDispSet = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		230, 85, 45, 21, _T("Set"), FREQWND_ID_F_DISP_SET);

	SetWndPtr(ctx->hwndEditDispF0, GWLP_USERDATA, ctx);
	SetWndPtr(ctx->hwndEditDispF1, GWLP_USERDATA, ctx);
	ctx->wndprocEditDispF0 = (WNDPROC)SetWndPtr(ctx->hwndEditDispF0,
		GWLP_WNDPROC, (void*)freqwnd_dispf0_edit_proc);
	ctx->wndprocEditDispF1 = (WNDPROC)SetWndPtr(ctx->hwndEditDispF1,
		GWLP_WNDPROC, (void*)freqwnd_dispf1_edit_proc);

	/* create display frequency range controls (length) */
	ctx->hwndCbFDispLen = ui_crt_combo(ctx->uidata, ctx->hwnd, CBS_DROPDOWN|WS_TABSTOP,
		10, 115, 100, 250, FREQWND_ID_F_DISP_LEN);
	ctx->hwndRbFDispLenL = ui_crt_btn(ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP,
		125, 119, 35, 15, _T("L"), FREQWND_ID_F_DISP_LEN_L);
	ctx->hwndRbFDispLenC = ui_crt_btn(ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP,
		160, 119, 35, 15, _T("C"), FREQWND_ID_F_DISP_LEN_C);
	ctx->hwndRbFDispLenR = ui_crt_btn(ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP,
		195, 119, 35, 15, _T("R"), FREQWND_ID_F_DISP_LEN_R);
	ctx->hwndBtnFDispLenSet = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		230, 115, 45, 21, _T("Set"), FREQWND_ID_F_DISP_LEN_SET);

	/* create grid snap controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		295, 65, 155, 15, _T("Main window snap grid:"), ID_CTL_STATIC);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		295, 88, 95, 15, _T("Display range"), ID_CTL_STATIC);
	ctx->hwndCbDispSnap = ui_crt_combo(ctx->uidata, ctx->hwnd, CBS_DROPDOWN|WS_TABSTOP,
		390, 85, 60, 250, FREQWND_ID_DISP_SNAP);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		295, 118, 95, 15, _T("Chan/In mark"), ID_CTL_STATIC);
	ctx->hwndCbMarkSnap = ui_crt_combo(ctx->uidata, ctx->hwnd, CBS_DROPDOWN|WS_TABSTOP,
		390, 115, 60, 250, FREQWND_ID_MARK_SNAP);

	/* initialize freq step selection combo boxes */
	nstep = sizeof(freqwnd_dispsnap_tbl) / sizeof(freqwnd_dispsnap_tbl[0]);
	freqwnd_initstepcb(ctx, ctx->hwndCbDispSnap, freqwnd_dispsnap_tbl, nstep);
	freqwnd_initstepcb(ctx, ctx->hwndCbMarkSnap, freqwnd_dispsnap_tbl, nstep);

	nstep = sizeof(freqwnd_marksnap_tbl) / sizeof(freqwnd_marksnap_tbl[0]);
	freqwnd_initstepcb(ctx, ctx->hwndCbFDispLen, freqwnd_marksnap_tbl, nstep);

	/* load parameters */
	freqwnd_loadparams(ctx);

	/* initialize values */
	freqwnd_loadinputcenterfreq(ctx);

	freqwnd_loaddispfreqrange(ctx);
	ui_set_double(ctx->uidata, ctx->hwndCbDispSnap,
		ctx->fcfg->f_disp_range_snap, ctx->prefixFDispSnap, 3, 0);
	ui_set_double(ctx->uidata, ctx->hwndCbMarkSnap,
		ctx->fcfg->f_mark_snap, ctx->prefFMarkSnap, 3, 0);

	/* subscribe to notifications */
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_RX_STATE, ctx, freqwnd_rx_state_handler);
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_VISUALCFG, ctx, freqwnd_visualcfg_handler);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_destroy(freqwnd_ctx_t *ctx)
{
	/* unregister callback */
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_VISUALCFG, ctx, freqwnd_visualcfg_handler);
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_RX_STATE, ctx, freqwnd_rx_state_handler);

	/* apply changed values */
	freqwnd_updatedispsnap(ctx);
	freqwnd_updatemarksnap(ctx);

	/* save parameters */
	freqwnd_saveparams(ctx);

	/* save window position */
	ui_savepos(ctx->hwnd, ini_sect_get(ctx->ini, _T("freqwnd"), 1),
		UI_POS_OFFSET, ctx->cx_frame, ctx->cy_frame);

	/* send destroying message */
	uievent_send(ctx->event_window_close, EVENT_WNDCLOSE_FREQWND, ctx->hwnd);
}

/* ---------------------------------------------------------------------------------------------- */

static void freqwnd_cleanup(freqwnd_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK freqwnd_wndproc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	freqwnd_ctx_t *ctx;

	switch(umsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lp)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!freqwnd_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL )
			freqwnd_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL )
		{
			SetWndPtr(hwnd, GWLP_USERDATA, NULL);
			freqwnd_cleanup(ctx);
		}
		return 0;

	case WM_COMMAND:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wp))
		{
		case FREQWND_ID_F_IN_SET:
			freqwnd_cmd_changeinputcenterfreq(ctx);
			return 0;
		case FREQWND_ID_F_DISP_SET:
			freqwnd_cmd_changedispfreqrange(ctx);
			return 0;
		case FREQWND_ID_F_DISP_LEN_SET:
			freqwnd_cmd_changedispfreqlen(ctx);
			return 0;
		case FREQWND_ID_DISP_SNAP:
			if(HIWORD(wp) == CBN_KILLFOCUS) {
				freqwnd_updatedispsnap(ctx);
				return 0;
			}
			break;
		case FREQWND_ID_MARK_SNAP:
			if(HIWORD(wp) == CBN_KILLFOCUS) {
				freqwnd_updatemarksnap(ctx);
				return 0;
			}
			break;
		}
		break;

	case WM_ACTIVATE:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wp)
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			keynav_setcurwnd(&(ctx->uidata->keynav), hwnd, 0);
			break;
		case WA_INACTIVE:
			keynav_unsetcurwnd(&(ctx->uidata->keynav), hwnd);
			break;
		}
		return 0;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

HWND freqwnd_create(uicommon_t *uidata, ini_data_t *ini, freqcfg_t *fcfg,
					rxstate_t *rx, specview_ctx_t *specview, watrview_ctx_t *watrview,
					HWND hwndMain)
{
	freqwnd_ctx_t *ctx;
	int win_w, win_h, win_x, win_y;
	int sizestate;
	HWND hwnd;

	if( (ctx = calloc(1, sizeof(freqwnd_ctx_t))) == NULL )
		return NULL;

	ctx->hwndMain = hwndMain;
	ctx->uidata = uidata;
	ctx->ini = ini;

	ctx->fcfg = fcfg;
	ctx->rx = rx;
	ctx->specview = specview;
	ctx->watrview = watrview;

	ctx->event_window_close = uievent_register(&(uidata->event_list), EVENT_NAME_WNDCLOSE);
	ctx->event_rx_state = uievent_register(&(uidata->event_list), EVENT_NAME_RX_STATE);
	ctx->event_visualcfg = uievent_register(&(uidata->event_list), EVENT_NAME_VISUALCFG);

	/* init window pos */
	ui_frame_size(&(ctx->cx_frame), &(ctx->cy_frame),
		UI_FRAME_FIXED|UI_FRAME_CAPTION);

	ui_calcpos(ini_sect_get(ini, _T("freqwnd"), 0), UI_POS_OFFSET,
		ctx->cx_frame, ctx->cy_frame, FREQWND_W, FREQWND_H, FREQWND_W, FREQWND_H,
		&win_x, &win_y, &win_w, &win_h, &sizestate, SW_SHOWNORMAL);

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(freqwnd_atom), freqwnd_name,
		WS_POPUPWINDOW|WS_CAPTION|WS_DLGFRAME|WS_MINIMIZEBOX,
		win_x, win_y, win_w, win_h,
		hwndMain, NULL, uidata->h_inst, ctx);

	if(hwnd == NULL)
	{
		/* do not free anything */
		return NULL;
	}

	ShowWindow(hwnd, sizestate);

	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

int freqwnd_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcx;

	if(freqwnd_atom == 0)
	{
		memset(&wcx, 0, sizeof(wcx));
		wcx.cbSize = sizeof(wcx);
		wcx.lpfnWndProc = freqwnd_wndproc;
		wcx.hInstance = uidata->h_inst;
		wcx.hIcon = uidata->hicn_main;
		wcx.hCursor = uidata->hcur_main;
		wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx.lpszClassName = freqwnd_classname;

		freqwnd_atom = RegisterClassEx(&wcx);
	}

	return (freqwnd_atom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void freqwnd_unregisterclass(HINSTANCE h_inst)
{
	if(freqwnd_atom != 0)
	{
		UnregisterClass(MAKEINTATOM(freqwnd_atom), h_inst);
		freqwnd_atom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
