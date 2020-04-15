/* ---------------------------------------------------------------------------------------------- */

#include "sqlcfg.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *sqlcfg_classname = _T("rxtest_rxsql_config");

static ATOM sqlcfg_classatom;

/* ---------------------------------------------------------------------------------------------- */

static int sqlcfg_apply(sqlcfg_ctx_t *ctx)
{
	int use_carrf, use_carrf_keep;
	double bw, ep, ot, ct, od, cd;
	int bwf, epf, otf, ctf, odf, cdf;
	int error, cfgchanged, i;
	rxproc_t *proc;
	notify_proc_list_t notifyList;
	int state;

	use_carrf = use_carrf_keep = 0;
	bw = ep = ot = ct = od = cd = 0.0;
	bwf = epf = otf = ctf = odf = cdf = 0;

	/* get carrier filter flag */
	state = Button_GetCheck(ctx->hwndBtnUseCarrf);
	if(state & BST_CHECKED)
		use_carrf = 1;
	if(state & BST_INDETERMINATE)
		use_carrf_keep = 1;

	/* parse sense bandwidth */
	Edit_GetText(ctx->hwndEditBw, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		bwf = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditBw, &bw,
			RXSQL_BW_MIN, RXSQL_BW_MAX,
			_T("Sense bandwidth"), _T(" Hz"), 3, 0, 0, NULL) )
		{
			return 0;
		}
	}

	/* parse envelope filter parameter */
	Edit_GetText(ctx->hwndEditEfParam, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		epf = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditEfParam, &ep,
			RXSQL_ENVEF_PARAM_MIN, RXSQL_ENVEF_PARAM_MAX,
			_T("Envelope filter parameter"), _T(""), 3, 0, 0, NULL) )
		{
			return 0;
		}
	}

	/* parse open threshold */
	Edit_GetText(ctx->hwndEditOpThres, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		otf = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditOpThres, &ot,
			RXSQL_THRES_DB_MIN, RXSQL_THRES_DB_MAX,
			_T("Open threshold"), _T(" dB"), 6, 1, 0, NULL) )
		{
			return 0;
		}
	}

	/* parse open delay */
	Edit_GetText(ctx->hwndEditOpDly, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		odf = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditOpDly, &od,
			RXSQL_DLY_MS_MIN, RXSQL_DLY_MS_MAX,
			_T("Open delay"), _T(" ms"), 6, 1, 0, NULL) )
		{
			return 0;
		}
	}

	/* parse close threshold */
	Edit_GetText(ctx->hwndEditClThres, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		ctf = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditClThres, &ct,
			RXSQL_THRES_DB_MIN, RXSQL_THRES_DB_MAX,
			_T("Close threshold"), _T(" dB"), 6, 1, 0, NULL) )
		{
			return 0;
		}
	}

	/* parse close delay */
	Edit_GetText(ctx->hwndEditClDly, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(_tcscmp(ctx->uidata->databuf, _T("*")) == 0) {
		cdf = 1;
	} else {
		if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditClDly, &cd,
			RXSQL_DLY_MS_MIN, RXSQL_DLY_MS_MAX,
			_T("Close delay"), _T(" ms"), 6, 1, 0, NULL) )
		{
			return 0;
		}
	}

	/* save default values */
	if(Button_GetCheck(ctx->hwndBtnSaveDef) & BST_CHECKED)
	{
		if(!use_carrf_keep) ctx->procdefcfg->sql.use_carr_filter = use_carrf;
		if(!bwf) ctx->procdefcfg->sql.bw = bw;
		if(!epf) ctx->procdefcfg->sql.envef_param = ep;
		if(!otf) ctx->procdefcfg->sql.op_thres_db = ot;
		if(!ctf) ctx->procdefcfg->sql.cl_thres_db = ct;
		if(!odf) ctx->procdefcfg->sql.op_dly_ms = od;
		if(!cdf) ctx->procdefcfg->sql.cl_dly_ms = cd;

		Button_SetCheck(ctx->hwndBtnSaveDef, BST_UNCHECKED);
	}

	/* process channel list */
	error = 0;
	cfgchanged = 0;
	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			if(use_carrf_keep) use_carrf = proc->cfg.sql.use_carr_filter;
			if(bwf) bw = proc->cfg.sql.bw;
			if(epf) ep = proc->cfg.sql.envef_param;
			if(otf) ot = proc->cfg.sql.op_thres_db;
			if(ctf) ct = proc->cfg.sql.cl_thres_db;
			if(odf) od = proc->cfg.sql.op_dly_ms;
			if(cdf) cd = proc->cfg.sql.cl_dly_ms;

			if( (use_carrf != proc->cfg.sql.use_carr_filter) ||
				(fabs(bw - proc->cfg.sql.bw) >= 1e-3) ||
				(fabs(ep - proc->cfg.sql.envef_param) >= 1e-6) ||
				(fabs(ot - proc->cfg.sql.op_thres_db) >= 1e-6) ||
				(fabs(ct - proc->cfg.sql.cl_thres_db) >= 1e-6) ||
				(fabs(od - proc->cfg.sql.op_dly_ms) >= 1e-6) ||
				(fabs(cd - proc->cfg.sql.cl_dly_ms) >= 1e-6) )
			{
				if(!rxproc_set_sql_params(proc, use_carrf, bw, ep, ot, ct, od, cd,
					ctx->uidata->msgbuf, ctx->uidata->msgbuf_size))
				{
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
		uievent_send(ctx->event_procchan, EVENT_PROCCHAN_SQLCFG, &notifyList);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int sqlcfg_initvals(sqlcfg_ctx_t *ctx)
{
	int use_carrf, use_carrf_multi;
	double bw, ep, ot, ct, od, cd;
	int bwf, epf, otf, ctf, odf, cdf;
	int i, firstf;
	rxproc_t *proc;

	use_carrf = use_carrf_multi = 0;
	bw = ep = ot = ct = od = cd = 0.0;
	bwf = epf = otf = ctf = odf = cdf = 0;

	firstf = 1;
	for(i = 0; i < ctx->chidCount; i++)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[i])) != NULL )
		{
			if(firstf)
			{
				use_carrf = proc->cfg.sql.use_carr_filter;
				bw = proc->cfg.sql.bw;
				ep = proc->cfg.sql.envef_param;
				ot = proc->cfg.sql.op_thres_db;
				ct = proc->cfg.sql.cl_thres_db;
				od = proc->cfg.sql.op_dly_ms;
				cd = proc->cfg.sql.cl_dly_ms;
				firstf = 0;
			}
			else
			{
				if(use_carrf != proc->cfg.sql.use_carr_filter)
					use_carrf_multi = 1;
				if( fabs(bw - proc->cfg.sql.bw) >= 1e-3 )
					bwf = 1;
				if( fabs(ep - proc->cfg.sql.envef_param) >= 1e-6 )
					epf = 1;
				if( fabs(ot - proc->cfg.sql.op_thres_db) >= 1e-6 )
					otf = 1;
				if( fabs(ct - proc->cfg.sql.cl_thres_db) >= 1e-6 )
					ctf = 1;
				if( fabs(od - proc->cfg.sql.op_dly_ms) >= 1e-6 )
					odf = 1;
				if( fabs(cd - proc->cfg.sql.cl_dly_ms) >= 1e-6 )
					cdf = 1;
			}
		}
	}

	if(firstf) {
		return 0;
	}

	if(use_carrf_multi)
	{
		Button_SetStyle(ctx->hwndBtnUseCarrf, BS_AUTO3STATE, TRUE);
		Button_SetCheck(ctx->hwndBtnUseCarrf, BST_INDETERMINATE);
	}
	else
	{
		Button_SetCheck(ctx->hwndBtnUseCarrf, use_carrf ? BST_CHECKED : BST_UNCHECKED);
	}

	if((!use_carrf_multi) && (!use_carrf)) {
		EnableWindow(ctx->hwndEditBw, FALSE);
	}

	if(bwf) {
		Edit_SetText(ctx->hwndEditBw, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditBw, bw, 0, 3, 0);
	}

	if(epf) {
		Edit_SetText(ctx->hwndEditEfParam, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditEfParam, ep, 0, 3, 0);
	}

	if(otf) {
		Edit_SetText(ctx->hwndEditOpThres, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditOpThres, ot, 0, 6, 1);
	}

	if(odf) {
		Edit_SetText(ctx->hwndEditOpDly, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditOpDly, od, 0, 6, 1);
	}

	if(ctf) {
		Edit_SetText(ctx->hwndEditClThres, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditClThres, ct, 0, 6, 1);
	}

	if(cdf) {
		Edit_SetText(ctx->hwndEditClDly, _T("*"));
	} else {
		ui_set_double(ctx->uidata, ctx->hwndEditClDly, cd, 0, 6, 1);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void sqlcfg_change_carrf_use(sqlcfg_ctx_t *ctx)
{
	int state;

	state = Button_GetCheck(ctx->hwndBtnUseCarrf);

	if((state & BST_INDETERMINATE) || (state & BST_CHECKED)) {
		EnableWindow(ctx->hwndEditBw, TRUE);
	} else {
		EnableWindow(ctx->hwndEditBw, FALSE);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int sqlcfg_init(sqlcfg_ctx_t *ctx)
{
	ctx->hwndBtnUseCarrf = ui_crt_btn(ctx->uidata, ctx->hwnd, BS_AUTOCHECKBOX,
		10, 10, 225, 15, _T("Enable carrier sense filter"), CARRSQCFG_ID_USECARRF);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 30, 140, 15, _T("Sense bandwidth"), ID_CTL_STATIC);
	ctx->hwndEditBw = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 30, 60, 15, CARRSQCFG_ID_BW);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		215, 30, 20, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 50, 140, 15, _T("Envelope filter"), ID_CTL_STATIC);
	ctx->hwndEditEfParam = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 50, 60, 15, CARRSQCFG_ID_EFPARAM);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 70, 140, 15, _T("Open threshold"), ID_CTL_STATIC);
	ctx->hwndEditOpThres = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 70, 40, 15, CARRSQCFG_ID_OPNTHRES);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		195, 70, 20, 15, _T("dB"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 90, 140, 15, _T("Open delay"), ID_CTL_STATIC);
	ctx->hwndEditOpDly = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 90, 40, 15, CARRSQCFG_ID_OPNDELAY);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		195, 90, 20, 15, _T("ms"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 110, 140, 15, _T("Close threshold"), ID_CTL_STATIC);
	ctx->hwndEditClThres = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 110, 40, 15, CARRSQCFG_ID_CLSTHRES);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		195, 110, 20, 15, _T("dB"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 130, 140, 15, _T("Close delay"), ID_CTL_STATIC);
	ctx->hwndEditClDly = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 130, 40, 15, CARRSQCFG_ID_CLSDELAY);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		195, 130, 20, 15, _T("ms"), ID_CTL_STATIC);

	ctx->hwndBtnSaveDef = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		10, 150, 140, 15, _T("Use as defaults"), CARRSQCFG_ID_SAVEDEF);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 175, 60, 21, _T("Cancel"), CARRSQCFG_ID_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		105, 175, 60, 21, _T("Apply"), CARRSQCFG_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		170, 175, 60, 21, _T("Ok"), CARRSQCFG_ID_OK);

	if( !sqlcfg_initvals(ctx) )
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void sqlcfg_cleanup(sqlcfg_ctx_t *ctx)
{
	free(ctx->chidList);
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK sqlcfg_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	sqlcfg_ctx_t *ctx;

	switch(uMsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lParam)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!sqlcfg_init(ctx))
			return -1;
		return 0;

	case WM_NCDESTROY:
		if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL ) {
			SetWndPtr(hwnd, GWLP_USERDATA, NULL);
			sqlcfg_cleanup(ctx);
		}
		return 0;

	case WM_COMMAND:
		
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(LOWORD(wParam))
		{
		case CARRSQCFG_ID_USECARRF:
			sqlcfg_change_carrf_use(ctx);
			return 0;

		case CARRSQCFG_ID_APPLY:
			sqlcfg_apply(ctx);
			return 0;

		case CARRSQCFG_ID_OK:
			if(sqlcfg_apply(ctx))
				DestroyWindow(hwnd);
			return 0;

		case CARRSQCFG_ID_CANCEL:
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

static int sqlcfg_windowtitle(sqlcfg_ctx_t *ctx)
{
	rxproc_t *proc;

	if(ctx->chidCount == 1)
	{
		if( (proc = rx_proc_find(ctx->rx, ctx->chidList[0])) == NULL )
			return 0;
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Squelch (%s)"), proc->cfg.name);
	}
	else
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Squelch (%d channels)"), ctx->chidCount);
	}
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int sqlcfg_createwindow(uicommon_t *uidata, uievent_t *event_procchan, HWND hwndOwner,
						rxstate_t *rx, rxprocconfig_t *procdefcfg,
						unsigned int *chidList, int chidCount, int x, int y)
{
	HWND hwnd;
	sqlcfg_ctx_t *ctx;
	int w, h;

	if( (ctx = calloc(1, sizeof(sqlcfg_ctx_t))) == NULL )
	{
		free(chidList);
		return 0;
	}

	ctx->uidata = uidata;
	ctx->event_procchan = event_procchan;

	ctx->rx = rx;
	ctx->procdefcfg = procdefcfg;
	ctx->chidList = chidList;
	ctx->chidCount = chidCount;

	/* window size */
	w =
		GetSystemMetrics(SM_CXFIXEDFRAME) * 2 +
		CARRSQCFG_CW;
	h =
		GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
		GetSystemMetrics(SM_CYCAPTION) +
		CARRSQCFG_CH;

	/* window pos */
	if( x + w > GetSystemMetrics(SM_CXSCREEN) )
		x -= w;
	if( y + h > GetSystemMetrics(SM_CYSCREEN) )
		y -= h;

	/* format window name */
	if(!sqlcfg_windowtitle(ctx))
	{
		free(chidList);
		free(ctx);
		return 0;
	}

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(sqlcfg_classatom), ctx->uidata->msgbuf,
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

int sqlcfg_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcl;

	if(sqlcfg_classatom == 0)
	{
		memset(&wcl, 0, sizeof(wcl));
		wcl.cbSize = sizeof(wcl);
		wcl.lpfnWndProc = sqlcfg_proc;
		wcl.hInstance = uidata->h_inst;
		wcl.hIcon = uidata->hicn_main;
		wcl.hCursor = uidata->hcur_main;
		wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcl.lpszClassName = sqlcfg_classname;
		wcl.hIconSm = uidata->hicn_sm;

		sqlcfg_classatom = RegisterClassEx(&wcl);
	}

	return (sqlcfg_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void sqlcfg_unregisterclass(HINSTANCE h_inst)
{
	if(sqlcfg_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(sqlcfg_classatom), h_inst);
		sqlcfg_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
