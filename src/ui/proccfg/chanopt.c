/* ---------------------------------------------------------------------------------------------- */

#include "chanopt.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *chanopt_classname = _T("rxtest_channel_options");

static ATOM chanopt_classatom;

/* ---------------------------------------------------------------------------------------------- */

static int chanopt_initvals(chanopt_ctx_t *ctx)
{
	int i, firstf;
	rxproc_t *proc;
	int decim_frgran, decim_mcsdf;
	double decim_dff, decim_as;
	double dcrem_alpha;
	int decim_frgran_multi, decim_mcsdf_multi, decim_dff_multi, decim_as_multi;
	int dcrem_alpha_multi;

	decim_frgran = decim_mcsdf = 0;
	decim_dff = decim_as = 0.0;
	decim_frgran_multi = decim_mcsdf_multi = decim_dff_multi = decim_as_multi = 0;

	dcrem_alpha = 0.0;
	dcrem_alpha_multi = 0;

	/* get values from selected channels */
	firstf = 1;
	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			if(firstf)
			{
				decim_frgran = (int)(proc->cfg.decim_frgran);
				decim_mcsdf = (int)(proc->cfg.decim_mcsdf);
				decim_dff = proc->cfg.decim_dff;
				decim_as = proc->cfg.decim_as;

				dcrem_alpha = proc->cfg.output_dcrem_alpha;

				firstf = 0;
			}
			else
			{
				if( decim_frgran != (int)(proc->cfg.decim_frgran) )
					decim_frgran_multi = 1;
				if( decim_mcsdf != (int)(proc->cfg.decim_mcsdf) )
					decim_mcsdf_multi = 1;
				if( fabs(decim_dff - proc->cfg.decim_dff) >= 1e-6 )
					decim_dff_multi = 1;
				if( fabs(decim_as - proc->cfg.decim_as) >= 1e-6 )
					decim_as_multi = 1;

				if( fabs(dcrem_alpha - proc->cfg.output_dcrem_alpha) >= 1e-6 )
					dcrem_alpha_multi = 1;
			}
		}
	}

	/* no channels processed? */
	if(firstf) {
		return 0;
	}

	/* put values to edit boxes */
	if(decim_frgran_multi) {
		Edit_SetText(ctx->hwndEditDecimFrgran, _T("*"));
	} else {
		ui_set_int(ctx->uidata, ctx->hwndEditDecimFrgran, decim_frgran);
	}
	if(decim_mcsdf_multi) {
		Edit_SetText(ctx->hwndEditDecimMcsdf, _T("*"));
	} else {
		ui_set_int(ctx->uidata, ctx->hwndEditDecimMcsdf, decim_mcsdf);
	}
	if(decim_dff_multi) {
		Edit_SetText(ctx->hwndEditDecimDff, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditDecimDff, decim_dff, 0, 6, 1);
	}
	if(decim_as_multi) {
		Edit_SetText(ctx->hwndEditDecimAs, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditDecimAs, decim_as, 0, 6, 1);
	}

	if(dcrem_alpha_multi) {
		Edit_SetText(ctx->hwndEditDcremAlpha, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditDcremAlpha, dcrem_alpha, 0, 6, 1);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int chanopt_apply_decim(chanopt_ctx_t *ctx)
{
	int i, error;
	int decim_frgran, decim_mcsdf;
	double decim_dff, decim_as;
	int decim_frgran_keep, decim_mcsdf_keep, decim_dff_keep, decim_as_keep;
	rxproc_t *proc;
	int decim_params_changed;
	notify_proc_list_t channelList;

	/* get defaults */
	decim_frgran = (int)(ctx->rx->config.proc_decim_frgran);
	decim_mcsdf = (int)(ctx->rx->config.proc_decim_mcsdf);
	decim_dff = ctx->rx->config.proc_decim_dff;
	decim_as = ctx->rx->config.proc_decim_as;

	/* parse decimator frequency response granularity */
	Edit_GetText(ctx->hwndEditDecimFrgran,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		decim_frgran_keep = 1;
	} else {
		if( !ui_get_int_range(ctx->uidata, ctx->hwnd, ctx->hwndEditDecimFrgran, &decim_frgran,
			RXLIM_PROC_DECIM_FRGRAN_MIN, RXLIM_PROC_DECIM_FRGRAN_MAX,
			_T("Decimator frequency response granularity"), _T(" samples")) )
		{
			return 0;
		}
		decim_frgran_keep = 0;
	}

	/* parse decimator minimum compensation stage downsampling factor */
	Edit_GetText(ctx->hwndEditDecimMcsdf,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		decim_mcsdf_keep = 1;
	} else {
		if( !ui_get_int_range(ctx->uidata, ctx->hwnd, ctx->hwndEditDecimMcsdf, &decim_mcsdf,
			RXLIM_PROC_DECIM_MCSDF_MIN, RXLIM_PROC_DECIM_MCSDF_MAX,
			_T("Decimator minimum compensation stage downsampling factor"), _T("")) )
		{
			return 0;
		}
		decim_mcsdf_keep = 0;
	}

	/* parse decimator transition bandwidth */
	Edit_GetText(ctx->hwndEditDecimDff,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		decim_dff_keep = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditDecimDff, &decim_dff,
			RXLIM_PROC_DECIM_DFF_MIN, RXLIM_PROC_DECIM_DFF_MAX,
			_T("Decimator transition bandwidth"), _T(""), 6, 1, 0, NULL) )
		{
			return 0;
		}
		decim_dff_keep = 0;
	}

	/* parse decimator stopband attenuation */
	Edit_GetText(ctx->hwndEditDecimAs,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		decim_as_keep = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditDecimAs, &decim_as,
			RXLIM_PROC_DECIM_AS_MIN, RXLIM_PROC_DECIM_AS_MAX,
			_T("Decimator stopband attenuation"), _T(" dB"), 6, 1, 0, NULL) )
		{
			return 0;
		}
		decim_as_keep = 0;
	}

	/* save defaults if requested */
	if(Button_GetCheck(ctx->hwndBtnDecimSetDefs) & BST_CHECKED)
	{
		ctx->rx->config.proc_decim_frgran = decim_frgran;
		ctx->rx->config.proc_decim_mcsdf = decim_mcsdf;
		ctx->rx->config.proc_decim_dff = decim_dff;
		ctx->rx->config.proc_decim_as = decim_as;

		Button_SetCheck(ctx->hwndBtnDecimSetDefs, BST_UNCHECKED);
	}

	/* apply values */
	error = 0;
	decim_params_changed = 0;
	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			/* Apply decimator parameters */
			if(decim_frgran_keep) decim_frgran = proc->cfg.decim_frgran;
			if(decim_mcsdf_keep) decim_mcsdf = proc->cfg.decim_mcsdf;
			if(decim_dff_keep) decim_dff = proc->cfg.decim_dff;
			if(decim_as_keep) decim_as = proc->cfg.decim_as;

			if( (decim_frgran != (int)(proc->cfg.decim_frgran)) ||
				(decim_mcsdf != (int)(proc->cfg.decim_mcsdf)) ||
				(fabs(decim_dff - proc->cfg.decim_dff) >= 1e-6) ||
				(fabs(decim_as - proc->cfg.decim_as) >= 1e-6) )
			{
				if( !rxproc_set_decim_params(
					proc, decim_frgran, decim_mcsdf, decim_dff, decim_as,
					ctx->uidata->msgbuf, ctx->uidata->msgbuf_size) )
				{
					MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
					error = 1;
				}
				decim_params_changed = 1;
			}
		}
	}

	/* broadcast notifications */
	if(decim_params_changed)
	{
		channelList.count = ctx->chidCount;
		channelList.items = ctx->chidList;
		uievent_send(ctx->event_procchan, EVENT_PROCCHAN_DECIMATION, &channelList);
	}

	return (!error);
}

/* ---------------------------------------------------------------------------------------------- */

static int chanopt_apply_dcrem(chanopt_ctx_t *ctx)
{
	int i, error;
	double dcrem_alpha;
	int dcrem_alpha_keep;
	rxproc_t *proc;
	int dcrem_params_changed;
	notify_proc_list_t channelList;

	/* get defaults */
	dcrem_alpha = ctx->rx->config.proc_dcrem_alpha;

	/* parse dc remover alpha parameter */
	Edit_GetText(ctx->hwndEditDcremAlpha,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		dcrem_alpha_keep = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditDcremAlpha, &dcrem_alpha,
			RXLIM_PROC_DCREM_ALPHA_MIN, RXLIM_PROC_DCREM_ALPHA_MAX,
			_T("DC removal filter Alpha coefficient"), _T(""), 6, 1, 0, NULL) )
		{
			return 0;
		}
		dcrem_alpha_keep = 0;
	}

	/* save defaults if requested */
	if(Button_GetCheck(ctx->hwndBtnDcremSetDefs) & BST_CHECKED)
	{
		ctx->rx->config.proc_dcrem_alpha = dcrem_alpha;

		Button_SetCheck(ctx->hwndBtnDcremSetDefs, BST_UNCHECKED);
	}

	/* apply values */
	error = 0;
	dcrem_params_changed = 0;
	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			/* Apply DC remover parameters */
			if(dcrem_alpha_keep) dcrem_alpha = proc->cfg.output_dcrem_alpha;

			if(fabs(dcrem_alpha - proc->cfg.output_dcrem_alpha) >= 1e-6)
			{
				rxproc_set_output_dcrem_alpha(proc, dcrem_alpha);
				dcrem_params_changed = 1;
			}
		}
	}

	/* broadcast notifications */
	if(dcrem_params_changed)
	{
		channelList.count = ctx->chidCount;
		channelList.items = ctx->chidList;
		uievent_send(uievent_register(&(ctx->uidata->event_list), EVENT_NAME_PROCCHAN),
			EVENT_PROCCHAN_OUTPUTCFG, &channelList);
	}

	return (!error);
}

/* ---------------------------------------------------------------------------------------------- */

static int chanopt_init(chanopt_ctx_t *ctx)
{
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 260, 15, _T("Input decimation parameters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		20, 30, 160, 15, _T("Response granularity"), ID_CTL_STATIC);
	ctx->hwndEditDecimFrgran = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 30, 40, 15, CHANOPT_ID_DECIM_FRGRAN);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		225, 30, 60, 15, _T("samples"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		20, 48, 160, 30, _T("Comb-compensator min. downsampling factor"), ID_CTL_STATIC);
	ctx->hwndEditDecimMcsdf = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 55, 40, 15, CHANOPT_ID_DECIM_MCSDF);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		20, 80, 160, 15, _T("Transition bandwidth"), ID_CTL_STATIC);
	ctx->hwndEditDecimDff = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 80, 40, 15, CHANOPT_ID_DECIM_DFF);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		225, 80, 60, 15, _T("Fs_base"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		20, 100, 160, 15, _T("Alias attenuation"), ID_CTL_STATIC);
	ctx->hwndEditDecimAs = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 100, 40, 15, CHANOPT_ID_DECIM_AS);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		225, 100, 60, 15, _T("dB"), ID_CTL_STATIC);

	ctx->hwndBtnDecimSetDefs = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		20, 120, 160, 15, _T("Use as defaults"), CHANOPT_ID_DECIM_SETDEFS);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 150, 260, 15, _T("Output DC offset removal parameters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		20, 170, 160, 15, _T("Filter Alpha coefficient"), ID_CTL_STATIC);
	ctx->hwndEditDcremAlpha = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 170, 40, 15, CHANOPT_ID_DCREM_ALPHA);

	ctx->hwndBtnDcremSetDefs = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		20, 190, 160, 15, _T("Use as default"), CHANOPT_ID_DCREM_SETDEFS);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 220, 60, 21, _T("Cancel"), CHANOPT_ID_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		160, 220, 60, 21, _T("Apply"), CHANOPT_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		225, 220, 60, 21, _T("Ok"), CHANOPT_ID_OK);

	if(!chanopt_initvals(ctx))
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void chanopt_cleanup(chanopt_ctx_t *ctx)
{
	free(ctx->chidList);
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK chanopt_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	chanopt_ctx_t *ctx;

	switch(uMsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lParam)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!chanopt_init(ctx))
			return -1;
		return 0;

	case WM_NCDESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL ) {
			SetWndPtr(hwnd, GWLP_USERDATA, NULL);
			chanopt_cleanup(ctx);
		}
		return 0;

	case WM_COMMAND:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wParam))
		{
		case CHANOPT_ID_APPLY:
			chanopt_apply_decim(ctx);
			chanopt_apply_dcrem(ctx);
			return 0;

		case CHANOPT_ID_OK:
			if(chanopt_apply_decim(ctx) && chanopt_apply_dcrem(ctx))
				DestroyWindow(hwnd);
			return 0;

		case CHANOPT_ID_CANCEL:
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

static int chanopt_windowtitle(chanopt_ctx_t *ctx)
{
	rxproc_t *proc;

	if(ctx->chidCount == 1)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[0])) == NULL )
			return 0;
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Options (%s)"), proc->cfg.name);
	}
	else
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Options (%d channels)"), ctx->chidCount);
	}
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int chanopt_createwindow(uicommon_t *uidata, uievent_t *event_procchan, HWND hwndOwner,
						 rxstate_t *rx, unsigned int *chidList, int chidCount, int x, int y)
{
	HWND hwnd;
	chanopt_ctx_t *ctx;
	int w, h;

	if( (ctx = calloc(1, sizeof(chanopt_ctx_t))) == NULL )
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
		CHANOPT_CW;
	h =
		GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
		GetSystemMetrics(SM_CYCAPTION) +
		CHANOPT_CH;

	/* window pos */
	if( x + w > GetSystemMetrics(SM_CXSCREEN) )
		x -= w;
	if( y + h > GetSystemMetrics(SM_CYSCREEN) )
		y -= h;

	/* format window name */
	if(!chanopt_windowtitle(ctx))
	{
		free(chidList);
		free(ctx);
		return 0;
	}

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(chanopt_classatom), ctx->uidata->msgbuf,
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

int chanopt_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcl;

	if(chanopt_classatom == 0)
	{
		memset(&wcl, 0, sizeof(wcl));
		wcl.cbSize = sizeof(wcl);
		wcl.lpfnWndProc = chanopt_proc;
		wcl.hInstance = uidata->h_inst;
		wcl.hIcon = uidata->hicn_main;
		wcl.hCursor = uidata->hcur_main;
		wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcl.lpszClassName = chanopt_classname;
		wcl.hIconSm = uidata->hicn_sm;

		chanopt_classatom = RegisterClassEx(&wcl);
	}

	return (chanopt_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void chanopt_unregisterclass(HINSTANCE h_inst)
{
	if(chanopt_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(chanopt_classatom), h_inst);
		chanopt_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
