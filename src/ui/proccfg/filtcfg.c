/* ---------------------------------------------------------------------------------------------- */

#include "filtcfg.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *filtcfg_classname = _T("rxtest_filter_config");

static ATOM filtcfg_classatom;

/* ---------------------------------------------------------------------------------------------- */

static int filtcfg_apply(filtcfg_ctx_t *ctx)
{
	double fc, df, as;
	int fc_keep, as_keep, df_keep;
	int i, error, cfgchanged;
	rxproc_t *proc;
	notify_proc_list_t notifyList;

	fc = df = as = 0.0;
	fc_keep = as_keep = df_keep = 0;

	/* parse cutoff frequency */
	Edit_GetText(ctx->hwndFcEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		fc_keep = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndFcEdit, &fc,
			RXLIM_PROC_FILTER_FC_MIN, RXLIM_PROC_FILTER_FC_MAX,
			_T("Filter cutoff frequency"), _T(" Hz"), 3, 0, 1, NULL) )
		{
			return 0;
		}
	}

	/* parse transition bandwidth */
	Edit_GetText(ctx->hwndDfEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		df_keep = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndDfEdit, &df,
			RXLIM_PROC_FILTER_DF_MIN, RXLIM_PROC_FILTER_DF_MAX,
			_T("Filter transition bandwidth"), _T(" Hz"), 3, 0, 1, NULL) )
		{
			return 0;
		}
	}

	/* parse stopband attenuation */
	Edit_GetText(ctx->hwndAsEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		as_keep = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndAsEdit, &as,
			RXLIM_PROC_FILTER_AS_MIN, RXLIM_PROC_FILTER_AS_MAX,
			_T("Filter stopband attenuation"), _T(" dB"), 6, 1, 0, NULL) )
		{
			return 0;
		}
	}

	/* save default values */
	if(Button_GetCheck(ctx->hwndBtnSaveDef) & BST_CHECKED)
	{
		if(!fc_keep) ctx->rx->config.proc_filter_fc = fc;
		if(!df_keep) ctx->rx->config.proc_filter_df = df;
		if(!as_keep) ctx->rx->config.proc_filter_as = as;

		Button_SetCheck(ctx->hwndBtnSaveDef, BST_UNCHECKED);
	}

	/* process channel list */
	error = 0;
	cfgchanged = 0;
	for(i = 0; i < ctx->chidCount; i++)
	{
		/* find target processing channel */
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			/* keep selected values */
			if(fc_keep) fc = proc->cfg.filter_fc;
			if(df_keep) df = proc->cfg.filter_df;
			if(as_keep) as = proc->cfg.filter_as;

			/* check configuration changed */
			if( (fabs(fc - proc->cfg.filter_fc) >= 1e-3) ||
				(fabs(df - proc->cfg.filter_df) >= 1e-3) ||
				(fabs(as - proc->cfg.filter_as) >= 1e-6) )
			{
				/* apply values */
				if( !rxproc_set_filter_params(proc, fc, df, as, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size) ) {
					MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
					error = 1;
				}
				cfgchanged = 1;
			}
		}
	}

	/* broadcast notification */
	if(cfgchanged)
	{
		notifyList.count = ctx->chidCount;
		notifyList.items = ctx->chidList;
		uievent_send(ctx->event_procchan, EVENT_PROCCHAN_FILTERCFG, &notifyList);
	}

	return (!error);
}

/* ---------------------------------------------------------------------------------------------- */

static int filtcfg_initvals(filtcfg_ctx_t *ctx)
{
	double fc, df, as;
	int fc_multi, df_multi, as_multi;
	int i, firstf;
	rxproc_t *proc;

	fc = df = as = 0.0;
	fc_multi = df_multi = as_multi = 0;

	firstf = 1;
	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			if(firstf) {
				fc = proc->cfg.filter_fc;
				df = proc->cfg.filter_df;
				as = proc->cfg.filter_as;
				firstf = 0;
			} else {
				if( fabs(fc - proc->cfg.filter_fc) >= 1e-3 )
					fc_multi = 1;
				if( fabs(df - proc->cfg.filter_df) >= 1e-3 )
					df_multi = 1;
				if( fabs(as - proc->cfg.filter_as) >= 1e-6 )
					as_multi = 1;
			}
		}
	}

	if(firstf) {
		return 0;
	}

	if(fc_multi) {
		Edit_SetText(ctx->hwndFcEdit, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndFcEdit, fc, 0, 3, 0);
	}

	if(df_multi) {
		Edit_SetText(ctx->hwndDfEdit, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndDfEdit, df, 0, 3, 0);
	}

	if(as_multi) {
		Edit_SetText(ctx->hwndAsEdit, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndAsEdit, as, 0, 6, 1);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int filtcfg_init(filtcfg_ctx_t *ctx)
{
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 140, 15, _T("Filter cutoff frequency"), ID_CTL_STATIC);
	ctx->hwndFcEdit = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 10, 60, 15, FILTCFG_ID_FC);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		215, 10, 20, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 30, 140, 15, _T("Transition bandwidth"), ID_CTL_STATIC);
	ctx->hwndDfEdit = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 30, 60, 15, FILTCFG_ID_DF);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		215, 30, 20, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 50, 140, 15, _T("Stopband attenuation"), ID_CTL_STATIC);
	ctx->hwndAsEdit = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 50, 40, 15, FILTCFG_ID_AS);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		195, 50, 20, 15, _T("dB"), ID_CTL_STATIC);

	ctx->hwndBtnSaveDef = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		10, 70, 140, 15, _T("Use as defaults"), FILTCFG_ID_SAVEDEF);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 95, 60, 21, _T("Cancel"), FILTCFG_ID_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		105, 95, 60, 21, _T("Apply"), FILTCFG_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		170, 95, 60, 21, _T("Ok"), FILTCFG_ID_OK);

	if( !filtcfg_initvals(ctx) )
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void filtcfg_destroy(filtcfg_ctx_t *ctx)
{
}

/* ---------------------------------------------------------------------------------------------- */

static void filtcfg_cleanup(filtcfg_ctx_t *ctx)
{
	free(ctx->chidList);
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK filtcfg_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	filtcfg_ctx_t *ctx;

	switch(uMsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lParam)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!filtcfg_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL )
			filtcfg_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL ) {
			SetWndPtr(hwnd, GWLP_USERDATA, NULL);
			filtcfg_cleanup(ctx);
		}
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case FILTCFG_ID_APPLY:
			filtcfg_apply(GetWndPtr(hwnd, GWLP_USERDATA));
			return 0;

		case FILTCFG_ID_OK:
			if(filtcfg_apply(GetWndPtr(hwnd, GWLP_USERDATA)))
				DestroyWindow(hwnd);
			return 0;

		case FILTCFG_ID_CANCEL:
			DestroyWindow(hwnd);
			return 0;
		}
		break;

	case WM_ACTIVATE:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wParam)
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			keynav_setcurwnd(&(ctx->uidata->keynav), hwnd, 1);
			break;
		case WA_INACTIVE:
			keynav_unsetcurwnd(&(ctx->uidata->keynav), hwnd);
			break;
		}
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* ---------------------------------------------------------------------------------------------- */

static int filtcfg_windowtitle(filtcfg_ctx_t *ctx)
{
	rxproc_t *proc;

	if(ctx->chidCount == 1)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[0])) == NULL )
			return 0;
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Baseband filter ('%s')"), proc->cfg.name);
	}
	else
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Baseband filter (%d channels)"), ctx->chidCount);
	}
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int filtcfg_createwindow(uicommon_t *uidata, uievent_t *event_procchan, HWND hwndOwner,
						 rxstate_t *rx, unsigned int *chidList, int chidCount, int x, int y)
{
	HWND hwnd;
	filtcfg_ctx_t *ctx;
	int w, h;

	if( (ctx = calloc(1, sizeof(filtcfg_ctx_t))) == NULL )
	{
		free(chidList);
		return 0;
	}

	ctx->uidata = uidata;
	ctx->event_procchan = event_procchan;

	ctx->rx = rx;
	ctx->chidList = chidList;
	ctx->chidCount = chidCount;

	/* window size */
	w =
		GetSystemMetrics(SM_CXFIXEDFRAME) * 2 +
		FILTCFG_CW;
	h =
		GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
		GetSystemMetrics(SM_CYCAPTION) +
		FILTCFG_CH;

	/* window pos */
	if( x + w > GetSystemMetrics(SM_CXSCREEN) )
		x -= w;
	if( y + h > GetSystemMetrics(SM_CYSCREEN) )
		y -= h;

	/* format window name */
	if(!filtcfg_windowtitle(ctx))
	{
		free(chidList);
		free(ctx);
		return 0;
	}

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(filtcfg_classatom), ctx->uidata->msgbuf,
		WS_POPUPWINDOW|WS_DLGFRAME|WS_CAPTION, x, y, w, h,
		hwndOwner, NULL, uidata->h_inst, ctx);

	if(hwnd == NULL)
	{
		/* do not free anything */
		return 0;
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int filtcfg_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcl;

	if(filtcfg_classatom == 0)
	{
		memset(&wcl, 0, sizeof(wcl));
		wcl.cbSize = sizeof(wcl);
		wcl.lpfnWndProc = filtcfg_proc;
		wcl.hInstance = uidata->h_inst;
		wcl.hIcon = uidata->hicn_main;
		wcl.hCursor = uidata->hcur_main;
		wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcl.lpszClassName = filtcfg_classname;
		wcl.hIconSm = uidata->hicn_sm;

		filtcfg_classatom = RegisterClassEx(&wcl);
	}

	return (filtcfg_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void filtcfg_unregisterclass(HINSTANCE h_inst)
{
	if(filtcfg_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(filtcfg_classatom), h_inst);
		filtcfg_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
