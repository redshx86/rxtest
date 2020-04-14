/* ---------------------------------------------------------------------------------------------- */

#include "cmcfg.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *cmcfg_classname = _T("rxtest_chanmgr_config");
static TCHAR *cmcfg_wndname = _T("Channel list settings");

static ATOM cmcfg_classatom;

/* ---------------------------------------------------------------------------------------------- */

void cmcfg_load(HWND hwnd, cmcfg_data_t *cfg)
{
	cmcfg_ctx_t *ctx;

	if( (ctx = GetWndPtr(hwnd, GWLP_USERDATA)) != NULL )
	{
		switch(cfg->dispUnitChanFreq)
		{
		case 0:
			Button_SetCheck(ctx->hwndBtnChanFreqUnitHz, BST_CHECKED);
			break;
		case 1:
			Button_SetCheck(ctx->hwndBtnChanFreqUnitKhz, BST_CHECKED);
			break;
		case 2:
			Button_SetCheck(ctx->hwndBtnChanFreqUnitMhz, BST_CHECKED);
			break;
		}

		switch(cfg->dispUnitFiltCutoff)
		{
		case 0:
			Button_SetCheck(ctx->hwndBtnFilterCutoffUnitHz, BST_CHECKED);
			break;
		case 1:
			Button_SetCheck(ctx->hwndBtnFilterCutoffUnitKhz, BST_CHECKED);
			break;
		case 2:
			Button_SetCheck(ctx->hwndBtnFilterCutoffUnitMhz, BST_CHECKED);
			break;
		}

		ui_set_double(ctx->uidata, ctx->hwndEditLevelMin, cfg->levelMin, 0, 6, 1);
		ui_set_double(ctx->uidata, ctx->hwndEditLevelMax, cfg->levelMax, 0, 6, 1);
		
		ui_set_int(ctx->uidata, ctx->hwndEditLevelUpdateInt, (int)(cfg->levelUpdateInt));

		Button_SetCheck(ctx->hwndBtnLevelScaleToInputBw,
			cfg->levelScaleToInputFc ? BST_CHECKED : BST_UNCHECKED);

	}
}

/* ---------------------------------------------------------------------------------------------- */

static int cmcfg_apply(cmcfg_ctx_t *ctx)
{
	cmcfg_data_t cfg;

	memset(&cfg, 0, sizeof(cfg));

	if(Button_GetCheck(ctx->hwndBtnChanFreqUnitKhz) & BST_CHECKED)
		cfg.dispUnitChanFreq = 1;
	if(Button_GetCheck(ctx->hwndBtnChanFreqUnitMhz) & BST_CHECKED)
		cfg.dispUnitChanFreq = 2;

	if(Button_GetCheck(ctx->hwndBtnFilterCutoffUnitKhz) & BST_CHECKED)
		cfg.dispUnitFiltCutoff = 1;
	if(Button_GetCheck(ctx->hwndBtnFilterCutoffUnitMhz) & BST_CHECKED)
		cfg.dispUnitFiltCutoff = 2;

	if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditLevelMin,
		&(cfg.levelMin), CMCFG_LEVEL_ABS_MIN, CMCFG_LEVEL_ABS_MAX, 
		_T("Lower level meter range limit"), _T(" dB"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditLevelMax,
		&(cfg.levelMax), CMCFG_LEVEL_ABS_MIN, CMCFG_LEVEL_ABS_MAX, 
		_T("Upper level meter range limit"), _T(" dB"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if(cfg.levelMax <= cfg.levelMin)
	{
		MessageBox(ctx->hwnd, _T("Bad level meter range."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwnd, ctx->hwndEditLevelUpdateInt,
		&(cfg.levelUpdateInt), CMCFG_LEVEL_UPDATE_INT_MIN, CMCFG_LEVEL_UPDATE_INT_MAX,
		_T("Level meter update interval"), _T(" ms")) )
	{
		return 0;
	}

	if(Button_GetCheck(ctx->hwndBtnLevelScaleToInputBw) & BST_CHECKED)
		cfg.levelScaleToInputFc = 1;

	uievent_send(ctx->event_chanmgrcfg, EVENT_CHANMGR_SETCONFIG, &cfg);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int cmcfg_init(cmcfg_ctx_t *ctx)
{
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 150, 15, _T("Channel frequency unit:"), ID_CTL_STATIC);

	ctx->hwndBtnChanFreqUnitHz = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP,
		15, 30, 45, 15, _T("Hz"), CMCFG_ID_CHAN_FREQ_UNIT_HZ);
	ctx->hwndBtnChanFreqUnitKhz = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP,
		65, 30, 45, 15, _T("kHz"), CMCFG_ID_CHAN_FREQ_UNIT_KHZ);
	ctx->hwndBtnChanFreqUnitMhz = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP,
		115, 30, 45, 15, _T("MHz"), CMCFG_ID_CHAN_FREQ_UNIT_MHZ);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 50, 150, 15, _T("Filter frequency unit:"), ID_CTL_STATIC);

	ctx->hwndBtnFilterCutoffUnitHz = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP,
		15, 70, 45, 15, _T("Hz"), CMCFG_ID_FILT_CUTOFF_UNIT_HZ);
	ctx->hwndBtnFilterCutoffUnitKhz = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP,
		65, 70, 45, 15, _T("kHz"), CMCFG_ID_FILT_CUTOFF_UNIT_KHZ);
	ctx->hwndBtnFilterCutoffUnitMhz = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTORADIOBUTTON|WS_TABSTOP,
		115, 70, 45, 15, _T("MHz"), CMCFG_ID_FILT_CUTOFF_UNIT_MHZ);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		175, 10, 150, 15, _T("Level meter range:"), ID_CTL_STATIC);

	ctx->hwndEditLevelMin = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 30, 45, 15, CMCFG_ID_LEVEL_MIN);
	ui_crt_static(ctx->uidata, ctx->hwnd, SS_CENTER,
		225, 30, 20, 15, _T("–"), ID_CTL_STATIC);
	ctx->hwndEditLevelMax = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		245, 30, 45, 15, CMCFG_ID_LEVEL_MAX);
	ui_crt_static(ctx->uidata, ctx->hwnd, SS_CENTER,
		295, 30, 30, 15, _T("dB"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		175, 50, 150, 15, _T("Level update interval:"), ID_CTL_STATIC);
	ctx->hwndEditLevelUpdateInt = ui_crt_editse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		180, 70, 45, 15, CMCFG_ID_LEVEL_UPDATE_INT);
	ui_crt_static(ctx->uidata, ctx->hwnd, SS_CENTER,
		230, 70, 30, 15, _T("ms"), ID_CTL_STATIC);

	ctx->hwndBtnLevelScaleToInputBw = ui_crt_btn(
		ctx->uidata, ctx->hwnd, BS_AUTOCHECKBOX|WS_TABSTOP,
		10, 100, 270, 15, _T("Scale level meter to input spectral density"),
		CMCFG_ID_LEVEL_SCALE_TO_INPUT);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 130, 65, 21, _T("Cancel"), CMWND_ID_BTN_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		185, 130, 65, 21, _T("Apply"), CMWND_ID_BTN_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		255, 130, 65, 21, _T("Ok"), CMWND_ID_BTN_OK);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void cmcfg_destroy(cmcfg_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK cmcfg_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	cmcfg_ctx_t *ctx;

	switch(umsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lp)->lpCreateParams;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		ctx->hwnd = hwnd;
		if(!cmcfg_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		cmcfg_destroy(ctx);
		return 0;

	case WM_COMMAND:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wp)
		{
		case CMWND_ID_BTN_CANCEL:
			DestroyWindow(hwnd);
			return 0;
		case CMWND_ID_BTN_APPLY:
			cmcfg_apply(ctx);
			return 0;
		case CMWND_ID_BTN_OK:
			if(cmcfg_apply(ctx))
				DestroyWindow(hwnd);
			return 0;
		}
		break;

	case WM_ACTIVATE:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wp)
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

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

HWND cmcfg_create(uicommon_t *uidata, HWND hwnd_owner, int x, int y)
{
	cmcfg_ctx_t *ctx;
	HWND hwnd;
	int wcx, wcy;

	if( (ctx = calloc(1, sizeof(cmcfg_ctx_t))) != NULL )
	{
		ctx->uidata = uidata;

		ctx->event_chanmgrcfg = uievent_register(&(uidata->event_list), EVENT_NAME_CHANMGRCFG);

		ui_window_size(CMCFG_W, CMCFG_H, &wcx, &wcy, UI_FRAME_FIXED|UI_FRAME_CAPTION);

		hwnd = CreateWindow(
			MAKEINTATOM(cmcfg_classatom), cmcfg_wndname, 
			WS_DLGFRAME|WS_CAPTION|WS_SYSMENU, x, y, wcx, wcy, hwnd_owner, NULL, uidata->h_inst, ctx);

		if(hwnd != NULL)
		{
			ShowWindow(hwnd, SW_SHOWNORMAL);
			return hwnd;
		}
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

int cmcfg_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcx;

	if(cmcfg_classatom == 0)
	{
		memset(&wcx, 0, sizeof(wcx));
		wcx.cbSize = sizeof(wcx);
		wcx.lpfnWndProc = cmcfg_proc;
		wcx.hInstance = uidata->h_inst;
		wcx.hIcon = uidata->hicn_main;
		wcx.hCursor = uidata->hcur_main;
		wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx.lpszClassName = cmcfg_classname;
		wcx.hIconSm = uidata->hicn_sm;

		cmcfg_classatom = RegisterClassEx(&wcx);
	}

	return (cmcfg_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void cmcfg_unregisterclass(HINSTANCE h_inst)
{
	if(cmcfg_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(cmcfg_classatom), h_inst);
		cmcfg_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
