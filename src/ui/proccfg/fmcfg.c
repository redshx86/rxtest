/* ---------------------------------------------------------------------------------------------- */

#include "fmcfg.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *fmcfg_classname = _T("rxtest_fm_config");

static ATOM fmcfg_classatom;

/* ---------------------------------------------------------------------------------------------- */

static int fmcfg_apply(fmcfg_ctx_t *ctx)
{
	double df;
	int df_keep;
	rxproc_t *proc;
	int is_updated, error;
	int i;
	notify_proc_list_t proclist;
	
	df = 0.0;
	df_keep = 0;

	Edit_GetText(ctx->hwndDfEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0)
	{
		df_keep = 1;
	}
	else
	{
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndDfEdit, &df,
			RXPROC_FM_DF_MIN, RXPROC_FM_DF_MAX,
			_T("Frequency deviation"), _T(" Hz"), 3, 0, 1, NULL) )
		{
			return 0;
		}
	}

	if(Button_GetCheck(ctx->hwndBtnSaveDef) & BST_CHECKED)
	{
		if(!df_keep) ctx->rx->config.proc_fm_def.df = df;

		Button_SetCheck(ctx->hwndBtnSaveDef, BST_UNCHECKED);
	}

	is_updated = 0;
	error = 0;

	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			if(df_keep) df = proc->cfg.fmdemod.df;

			if( fabs(df - proc->cfg.fmdemod.df) >= 1e-3 )
			{
				if( !rxproc_set_fm_demod_params(proc, df,
					ctx->uidata->msgbuf, ctx->uidata->msgbuf_size) )
				{
					MessageBox(ctx->hwnd, ctx->uidata->msgbuf,
						ui_title, MB_ICONEXCLAMATION|MB_OK);
					error = 1;
				}

				is_updated = 1;
			}
		}
	}

	if(is_updated)
	{
		proclist.count = ctx->chidCount;
		proclist.items = ctx->chidList;

		callback_list_call(ctx->cb_list, NOTIFY_PROCCHAN,
			NOTIFY_PROCCHAN_DEMODCFG, &proclist);
	}

	return (!error);
}

/* ---------------------------------------------------------------------------------------------- */

static int fmcfg_initvals(fmcfg_ctx_t *ctx)
{
	rxproc_t *proc;
	int i;
	double df;
	int df_multi;
	int firstf;

	df = 0.0;
	df_multi = 0;
	firstf = 1;

	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			if(firstf)
			{
				df = proc->cfg.fmdemod.df;
				firstf = 0;
			}
			else
			{
				if( fabs(df - proc->cfg.fmdemod.df) > 1e-3 )
					df_multi = 1;
			}
		}
	}

	if(firstf)
		return 0;

	if(df_multi) {
		Edit_SetText(ctx->hwndDfEdit, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndDfEdit, df, 0, 3, 0);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int fmcfg_init(fmcfg_ctx_t *ctx)
{
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 140, 15, _T("Frequency deviation"), ID_CTL_STATIC);
	ctx->hwndDfEdit = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 10, 60, 15, FMCFG_ID_DF);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		215, 10, 20, 15, _T("Hz"), ID_CTL_STATIC);

	ctx->hwndBtnSaveDef = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		10, 30, 140, 15, _T("Use as default"), FMCFG_ID_SAVEDEF);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 55, 60, 21, _T("Cancel"), FMCFG_ID_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		105, 55, 60, 21, _T("Apply"), FMCFG_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		170, 55, 60, 21, _T("Ok"), FMCFG_ID_OK);

	if( !fmcfg_initvals(ctx) )
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void fmcfg_cleanup(fmcfg_ctx_t *ctx)
{
	free(ctx->chidList);
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK fmcfg_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	fmcfg_ctx_t *ctx;

	switch(uMsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lParam)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!fmcfg_init(ctx))
			return -1;
		return 0;

	case WM_NCDESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL ) {
			SetWndPtr(hwnd, GWLP_USERDATA, NULL);
			fmcfg_cleanup(ctx);
		}
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case FMCFG_ID_APPLY:
			fmcfg_apply(GetWndPtr(hwnd, GWLP_USERDATA));
			return 0;

		case FMCFG_ID_OK:
			if(fmcfg_apply(GetWndPtr(hwnd, GWLP_USERDATA)))
				DestroyWindow(hwnd);
			return 0;

		case FMCFG_ID_CANCEL:
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

static int fmcfg_windowtitle(fmcfg_ctx_t *ctx)
{
	rxproc_t *proc;

	if(ctx->chidCount == 1)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[0])) == NULL )
			return 0;
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("FM demodulator (%s)"), proc->cfg.name);
	}
	else
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("FM demodulator (%d channels)"), ctx->chidCount);
	}
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int fmcfg_createwindow(uicommon_t *uidata, callback_list_t *cb_list, HWND hwndOwner,
						 rxstate_t *rx, unsigned int *chidList, int chidCount, int x, int y)
{
	HWND hwnd;
	fmcfg_ctx_t *ctx;
	int w, h;

	if( (ctx = calloc(1, sizeof(fmcfg_ctx_t))) == NULL )
	{
		free(chidList);
		return 0;
	}

	ctx->uidata = uidata;
	ctx->cb_list = cb_list;

	ctx->rx = rx;
	ctx->chidList = chidList;
	ctx->chidCount = chidCount;

	/* window size */
	w =
		GetSystemMetrics(SM_CXFIXEDFRAME) * 2 +
		FMCFG_CW;
	h =
		GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
		GetSystemMetrics(SM_CYCAPTION) +
		FMCFG_CH;

	/* window pos */
	if( x + w > GetSystemMetrics(SM_CXSCREEN) )
		x -= w;
	if( y + h > GetSystemMetrics(SM_CYSCREEN) )
		y -= h;

	/* format window name */
	if(!fmcfg_windowtitle(ctx))
	{
		free(chidList);
		free(ctx);
		return 0;
	}

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(fmcfg_classatom), ctx->uidata->msgbuf,
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

int fmcfg_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcl;

	if(fmcfg_classatom == 0)
	{
		memset(&wcl, 0, sizeof(wcl));
		wcl.cbSize = sizeof(wcl);
		wcl.lpfnWndProc = fmcfg_proc;
		wcl.hInstance = uidata->h_inst;
		wcl.hIcon = uidata->hicn_main;
		wcl.hCursor = uidata->hcur_main;
		wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcl.lpszClassName = fmcfg_classname;
		wcl.hIconSm = uidata->hicn_sm;

		fmcfg_classatom = RegisterClassEx(&wcl);
	}

	return (fmcfg_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void fmcfg_unregisterclass(HINSTANCE h_inst)
{
	if(fmcfg_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(fmcfg_classatom), h_inst);
		fmcfg_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
