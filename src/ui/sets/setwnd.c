/* ---------------------------------------------------------------------------------------------- */

#include "setwnd.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *setwnd_classname = _T("rxtest_settings");
static TCHAR *setwnd_name = _T("Settings");

static TCHAR *setwnd_pageclassname = _T("rxtest_settings_page");

/* ---------------------------------------------------------------------------------------------- */

static ATOM setwnd_classatom;
static ATOM setwnd_pageclassatom;

/* ---------------------------------------------------------------------------------------------- */

static setwnd_page_entry_t setwnd_page[] =
{
	{ _T("Processing"), _T("Processing and buffering parameters") },
	{ _T("Audio output"), _T("Audio output parameters") },
	{ _T("Visualization"), _T("Spectrum analyzer and visualization") },
	{ _T("Spectrum colors"), _T("Spectrum viewer colors") },
	{ _T("Waterfall colors"), _T("Waterfall viewer colors") }
};

/* ---------------------------------------------------------------------------------------------- */

static void setwnd_data_init(setwnd_ctx_t *ctx)
{
	pageproc_data_init(&(ctx->data_proc), &(ctx->rx->config));
	pageaud_data_init(&(ctx->data_aud), &(ctx->rx->config));
	pagevis_data_init(&(ctx->data_vis), &(ctx->rx->config), ctx->svcfg, ctx->wvcfg);
	pagecrsv_data_init(&(ctx->data_crsv), ctx->svcfg);
	pagecrwv_data_init(&(ctx->data_crwv), ctx->wvcfg);
}

/* ---------------------------------------------------------------------------------------------- */

static int setwnd_data_apply(setwnd_ctx_t *ctx)
{
	int status = 1;

	pageproc_data_apply(&(ctx->rx->config), &(ctx->data_proc));
	pageaud_data_apply(&(ctx->rx->config), &(ctx->data_aud));

	if(!pagevis_data_apply(ctx->rx, ctx->svcfg, ctx->wvcfg, &(ctx->data_vis),
		ctx->hwnd, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size))
	{
		status = 0;
	}

	pagecrsv_data_apply(ctx->svcfg, &(ctx->data_crsv));
	pagecrwv_data_apply(ctx->wvcfg, &(ctx->data_crwv));

	uievent_send(ctx->event_rx_state, EVENT_RX_STATE_SET_CFG, NULL);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static int setwnd_page_save(setwnd_ctx_t *ctx)
{
	int status = 0;

	if(ctx->i_cur_page == -1)
		return 1;

	switch(ctx->i_cur_page)
	{
	case SETWND_PAGE_PROC:
		status = pageproc_save(&(ctx->data_proc), ctx->hwndPage);
		break;
	case SETWND_PAGE_AUDIO:
		status = pageaud_save(&(ctx->data_aud), ctx->hwndPage);
		break;
	case SETWND_PAGE_VIS:
		status = pagevis_save(&(ctx->data_vis), ctx->hwndPage);
		break;
	case SETWND_PAGE_CRSV:
		status = pagecrsv_save(&(ctx->data_crsv), ctx->hwndPage);
		break;
	case SETWND_PAGE_CRWV:
		status = pagecrwv_save(&(ctx->data_crwv), ctx->hwndPage);
		break;
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static void setwnd_page_close(setwnd_ctx_t *ctx)
{
	if(ctx->i_cur_page != -1)
	{
		DestroyWindow(ctx->hwndPage);
		Static_SetText(ctx->hwndPageTitle, _T(""));

		ctx->hwndPage = NULL;
		ctx->i_cur_page = -1;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int setwnd_page_open(setwnd_ctx_t *ctx, int page)
{
	int status;

	if( (page < 0) || (page >= SETWND_PAGE_COUNT) )
		return 0;

	if(page == ctx->i_cur_page)
		return -1;

	if(ctx->i_cur_page != -1)
	{
		if(!setwnd_page_save(ctx)) {
			ListBox_SetCurSel(ctx->hwndPageList, ctx->i_cur_page);
			return 0;
		}
		setwnd_page_close(ctx);
	}

	ctx->hwndPage = CreateWindow(MAKEINTATOM(setwnd_pageclassatom), NULL,
		WS_CHILD|WS_VISIBLE, 140, 32, 480, 250, ctx->hwnd, (HMENU)SETWND_ID_PAGE,
		ctx->uidata->h_inst, NULL);

	if(ctx->hwndPage != NULL)
	{
		status = 0;

		switch(page)
		{
		case SETWND_PAGE_PROC:
			status = pageproc_create(ctx->hwnd, ctx->hwndPage,
				ctx->uidata, &(ctx->data_proc));
			break;
		case SETWND_PAGE_AUDIO:
			status = pageaud_create(ctx->hwnd, ctx->hwndPage,
				ctx->uidata, &(ctx->data_aud));
			break;
		case SETWND_PAGE_VIS:
			status = pagevis_create(ctx->hwnd, ctx->hwndPage,
				ctx->uidata, &(ctx->data_vis));
			break;
		case SETWND_PAGE_CRSV:
			status = pagecrsv_create(ctx->hwnd, ctx->hwndPage,
				ctx->uidata, &(ctx->data_crsv));
			break;
		case SETWND_PAGE_CRWV:
			status = pagecrwv_create(ctx->hwnd, ctx->hwndPage,
				ctx->uidata, ctx->ini, &(ctx->data_crwv));
			break;
		}

		if(status)
		{
			Static_SetText(ctx->hwndPageTitle, setwnd_page[page].page_title);
			ctx->i_cur_page = page;

			/* update keyboard navigation tree */
			keynav_update(&(ctx->uidata->keynav), ctx->hwnd);

			return 1;
		}

		DestroyWindow(ctx->hwndPage);
		ctx->hwndPage = NULL;
	}

	/* update keyboard navigation tree */
	keynav_update(&(ctx->uidata->keynav), ctx->hwnd);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int setwnd_apply(setwnd_ctx_t *ctx)
{
	if( (ctx->i_cur_page != -1) &&
		(!setwnd_page_save(ctx)) )
	{
		return 0;
	}

	return setwnd_data_apply(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static int setwnd_init(setwnd_ctx_t *ctx)
{
	int i, page_sel;

	/* initialize page data */
	setwnd_data_init(ctx);

	/* create controls */
	ctx->hwndPageList = ui_crt_listbox(ctx->uidata, ctx->hwnd,
		LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_TABSTOP,
		5, 5, 125, 300, SETWND_ID_PAGELIST);
	ctx->hwndPageTitle = ui_crt_staticex(ctx->uidata, ctx->hwnd, SS_CENTER, WS_EX_STATICEDGE,
		140, 5, 480, 17, _T(""), SETWND_ID_PAGETITLE);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		140, 284, 60, 21, _T("Cancel"), SETWND_ID_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		490, 284, 60, 21, _T("Apply"), SETWND_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		560, 284, 60, 21, _T("Ok"), SETWND_ID_OK);

	/* initialize page list */
	for(i = 0; i < SETWND_PAGE_COUNT; i++) {
		ListBox_AddString(ctx->hwndPageList, setwnd_page[i].page_name);
	}

	/* select initial page */
	ctx->i_cur_page = -1;
	page_sel = 0;
	ini_geti(ini_sect_get(ctx->ini, _T("setwnd"), 0), _T("page"), &page_sel);
	if(setwnd_page_open(ctx, page_sel)) {
		ListBox_SetCurSel(ctx->hwndPageList, page_sel);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void setwnd_destroy(setwnd_ctx_t *ctx)
{
	ini_sect_t *sect;

	sect = ini_sect_get(ctx->ini, _T("setwnd"), 1);

	if(ctx->i_cur_page != -1)
	{
		ini_seti(sect, _T("page"), ctx->i_cur_page);
		setwnd_page_close(ctx);
	}

	ui_savepos(ctx->hwnd, sect, UI_POS_OFFSET, ctx->cx_frame, ctx->cy_frame);
	uievent_send(ctx->event_window_close, EVENT_WNDCLOSE_SETTINGS, ctx->hwnd);
}

/* ---------------------------------------------------------------------------------------------- */

static void setwnd_cleanup(setwnd_ctx_t *ctx)
{
	if(ctx != NULL)
	{
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK setwnd_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	setwnd_ctx_t *ctx;

	switch(umsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lp)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!setwnd_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		setwnd_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		setwnd_cleanup(ctx);
		return 0;

	case WM_COMMAND:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wp))
		{
		case SETWND_ID_OK:
			if(setwnd_apply(ctx))
				DestroyWindow(hwnd);
			return 0;
		case SETWND_ID_APPLY:
			setwnd_apply(ctx);
			return 0;
		case SETWND_ID_CANCEL:
			DestroyWindow(hwnd);
			return 0;

		case SETWND_ID_PAGELIST:
			switch(HIWORD(wp))
			{
			case LBN_SELCANCEL:
				setwnd_page_close(ctx);
				return 0;
			case LBN_SELCHANGE:
				setwnd_page_open(ctx, ListBox_GetCurSel(ctx->hwndPageList));
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

HWND setwnd_create(uicommon_t *uidata, HWND hwndMain, ini_data_t *ini,
				   rxstate_t *rx, specview_cfg_t *svcfg, watrview_cfg_t *wvcfg)
{
	setwnd_ctx_t *ctx;
	int win_x, win_y, win_w, win_h;
	int sizestate;
	HWND hwnd;

	if( (ctx = calloc(1, sizeof(setwnd_ctx_t))) == NULL )
		return NULL;

	ctx->uidata = uidata;

	ctx->ini = ini;
	
	ctx->rx = rx;
	ctx->svcfg = svcfg;
	ctx->wvcfg = wvcfg;

	ctx->event_window_close = uievent_register(&(uidata->event_list), EVENT_NAME_WNDCLOSE);
	ctx->event_rx_state = uievent_register(&(uidata->event_list), EVENT_NAME_RX_STATE);

	/* calculate window position */
	ui_frame_size(&(ctx->cx_frame), &(ctx->cy_frame),
		UI_FRAME_FIXED|UI_FRAME_CAPTION);

	ui_calcpos(ini_sect_get(ctx->ini, _T("setwnd"), 0), UI_POS_OFFSET,
		ctx->cx_frame, ctx->cy_frame, SETWND_W, SETWND_H, SETWND_W, SETWND_H,
		&win_x, &win_y, &win_w, &win_h, &sizestate, SW_SHOWNORMAL);

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(setwnd_classatom), setwnd_name,
		WS_POPUPWINDOW|WS_DLGFRAME|WS_CAPTION, win_x, win_y, win_w, win_h,
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

int setwnd_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcx_setwnd, wcx_setpage;

	if(setwnd_classatom == 0)
	{
		memset(&wcx_setwnd, 0, sizeof(wcx_setwnd));
		wcx_setwnd.cbSize = sizeof(wcx_setwnd);
		wcx_setwnd.lpfnWndProc = setwnd_proc;
		wcx_setwnd.hInstance = uidata->h_inst;
		wcx_setwnd.hIcon = uidata->hicn_main;
		wcx_setwnd.hCursor = uidata->hcur_main;
		wcx_setwnd.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx_setwnd.lpszClassName = setwnd_classname;

		setwnd_classatom = RegisterClassEx(&wcx_setwnd);
	}

	if(setwnd_pageclassatom == 0)
	{
		memset(&wcx_setpage, 0, sizeof(wcx_setpage));
		wcx_setpage.cbSize = sizeof(wcx_setpage);
		wcx_setpage.lpfnWndProc = DefWindowProc;
		wcx_setpage.hInstance = uidata->h_inst;
		wcx_setpage.hCursor = uidata->hcur_main;
		wcx_setpage.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx_setpage.lpszClassName = setwnd_pageclassname;

		setwnd_pageclassatom = RegisterClassEx(&wcx_setpage);
	}

	return ( (setwnd_classatom != 0) && (setwnd_pageclassatom != 0) );
}

/* ---------------------------------------------------------------------------------------------- */

void setwnd_unregisterclass(HINSTANCE h_inst)
{
	if(setwnd_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(setwnd_classatom), h_inst);
		setwnd_classatom = 0;
	}

	if(setwnd_pageclassatom != 0)
	{
		UnregisterClass(MAKEINTATOM(setwnd_pageclassatom), h_inst);
		setwnd_pageclassatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
