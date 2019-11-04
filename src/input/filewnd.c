/* ---------------------------------------------------------------------------------------------- */

#include "filein.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *filewnd_classname = _T("rxtest_filein_config");
static TCHAR *filewnd_name = _T("File input configuration");

static ATOM filewnd_classatom;

/* ---------------------------------------------------------------------------------------------- */

static void filewnd_browse(filewnd_ctx_t *ctx)
{
	DWORD drives, i, dir_ok;
	TCHAR *buf, *dirbuf;
	OPENFILENAME ofn;

	buf = malloc(MAX_PATH * sizeof(TCHAR));
	dirbuf = malloc(MAX_PATH * sizeof(TCHAR));

	if( (buf != NULL) && (dirbuf != NULL) )
	{
		/* start in first non-system drive */
		dir_ok = 0;
		if(GetWindowsDirectory(buf, MAX_PATH))
		{
			drives = GetLogicalDrives();
			for(i = 0; i < 26; i++)
			{
				if( drives & (1 << i) )
				{
					_stprintf(dirbuf, _T("%c:\\"), (TCHAR)(i + 'A'));
					if( (GetDriveType(dirbuf) == DRIVE_FIXED) &&
						(_tcsnicmp(dirbuf, buf, 3) != 0) )
					{
						dir_ok = 1;
						break;
					}
				}
			}
		}

		/* use windows fallback directory */
		if(!dir_ok) {
			_tcscpy(dirbuf, buf);
		}

		/* get current filename */
		Edit_GetText(ctx->hwndEditFilename, buf, MAX_PATH);

		/* spawn file selection dialog */
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = ctx->hwnd;
		ofn.hInstance = ctx->uidata->h_inst;
		ofn.lpstrFilter =
			_T("Wave files (*.wav)\0*.wav\0")
			_T("All files (*.*)\0*.*\0");
		ofn.lpstrFile = buf;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = dirbuf;
		ofn.lpstrTitle = _T("Select I/Q wave file");
		ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|
			OFN_ENABLESIZING|OFN_HIDEREADONLY;
		ofn.lpstrDefExt = _T("wav");

		if(GetOpenFileName(&ofn)) {
			Edit_SetText(ctx->hwndEditFilename, buf);
		}
	}

	free(dirbuf);
	free(buf);
}

/* ---------------------------------------------------------------------------------------------- */

static int filewnd_save(filewnd_ctx_t *ctx)
{
	filein_config_t *conf;
	double fc;
	int read_int, invert_iq;

	conf = &(ctx->filein->conf);

	/* gather data from controls */
	if( !ui_get_double(ctx->uidata, ctx->hwnd, ctx->hwndEditFc, &fc,
		_T("Center frequency"), 1, &(conf->filewnd_fc_prefix)) )
	{
		return 0;
	}

	if( (fc < FILEIN_FC_MIN) || (fc > FILEIN_FC_MAX) )
	{
		MessageBox(ctx->hwnd, _T("Entered center frequency is too large."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwnd, ctx->hwndEditReadInt, &read_int,
		FILEIN_READINT_MIN, FILEIN_READINT_MAX, _T("Read interval"), _T(" ms")) )
	{
		return 0;
	}

	invert_iq = (Button_GetCheck(ctx->hwndBtnSwapIq) & BST_CHECKED) != 0;

	/* save config */
	Edit_GetText(ctx->hwndEditFilename, conf->fname, (int)(conf->fname_max_len));
	conf->fc = fc;
	conf->read_interval = read_int;
	conf->invert_iq = invert_iq;

	/* apply changes if stream started */
	if(ctx->filein->s != NULL) {
		ctx->filein->s->invert_iq = invert_iq;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void filewnd_setdata(filewnd_ctx_t *ctx)
{
	filein_config_t *conf;

	conf = &(ctx->filein->conf);

	Edit_SetText(ctx->hwndEditFilename, conf->fname);

	ui_set_double(ctx->uidata, ctx->hwndEditFc, conf->fc,
		conf->filewnd_fc_prefix, 3, (conf->filewnd_fc_prefix >= 2) ? 3 : 0);

	ui_set_int(ctx->uidata, ctx->hwndEditReadInt, (int)(conf->read_interval));

	if(conf->invert_iq) {
		Button_SetCheck(ctx->hwndBtnSwapIq, BST_CHECKED);
	}
}

/* ---------------------------------------------------------------------------------------------- */

void filewnd_setstate(filewnd_ctx_t *ctx, BOOL started)
{
	if(ctx == NULL)
		return;

	EnableWindow(ctx->hwndEditFilename, !started);
	EnableWindow(ctx->hwndBtnBrowse, !started);
	EnableWindow(ctx->hwndEditFc, !started);
	EnableWindow(ctx->hwndEditReadInt, !started);
}

/* ---------------------------------------------------------------------------------------------- */

static int filewnd_init(filewnd_ctx_t *ctx)
{
	ctx->filein->filewnd = ctx;

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 300, 15, _T("Input wave file path:"), ID_CTL_STATIC);
	ctx->hwndEditFilename = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 30, 270, 21, FILEWND_ID_FILENAME);
	ctx->hwndBtnBrowse = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		285, 30, 25, 21, _T("..."), FILEWND_ID_BROWSE);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 60, 130, 15, _T("Center frequency:"), ID_CTL_STATIC);
	ctx->hwndEditFc = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 80, 100, 21, FILEWND_ID_FC);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		115, 83, 25, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		150, 60, 70, 15, _T("Read int.:"), ID_CTL_STATIC);
	ctx->hwndEditReadInt = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		150, 80, 40, 21, FILEWND_ID_READINT);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		195, 83, 25, 15, _T("ms"), ID_CTL_STATIC);

	ctx->hwndBtnSwapIq = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		230, 83, 80, 15, _T("Swap I/Q"), FILEWND_ID_SWAPIQ);

	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 115, 65, 21, _T("Cancel"), FILEWND_ID_CALCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		175, 115, 65, 21, _T("Apply"), FILEWND_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		245, 115, 65, 21, _T("Ok"), FILEWND_ID_OK);


	filewnd_setdata(ctx);

	if(ctx->filein->s != NULL)
		filewnd_setstate(ctx, TRUE);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void filewnd_destroy(filewnd_ctx_t *ctx)
{
	ctx->filein->filewnd = NULL;
}

/* ---------------------------------------------------------------------------------------------- */

static void filewnd_cleanup(filewnd_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK filewnd_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	filewnd_ctx_t *ctx;

	switch(umsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lp)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!filewnd_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		filewnd_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		filewnd_cleanup(ctx);
		return 0;

	case WM_COMMAND:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wp))
		{
		case FILEWND_ID_BROWSE:
			filewnd_browse(ctx);
			break;
		case FILEWND_ID_APPLY:
			filewnd_save(ctx);
			break;
		case FILEWND_ID_CALCEL:
			DestroyWindow(hwnd);
			break;
		case FILEWND_ID_OK:
			if(filewnd_save(ctx))
				DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_ACTIVATE:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
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

HWND filewnd_create(uicommon_t *uidata, HWND hwndowner, filein_ctx_t *filein)
{
	filewnd_ctx_t *ctx;
	int win_w, win_h, win_x, win_y;
	RECT ownerwndrect;
	HWND hwnd;

	if( (ctx = calloc(1, sizeof(filewnd_ctx_t))) != NULL )
	{
		ctx->uidata = uidata;
		ctx->filein = filein;

		/* calculate window pos */
		win_w = FILEWND_W +
			GetSystemMetrics(SM_CXDLGFRAME) * 2;
		win_h = FILEWND_H +
			GetSystemMetrics(SM_CYDLGFRAME) * 2 +
			GetSystemMetrics(SM_CYCAPTION);

		GetWindowRect(hwndowner, &ownerwndrect);
		win_x = (ownerwndrect.left + ownerwndrect.right - win_w) / 2;
		win_y = (ownerwndrect.top + ownerwndrect.bottom - win_h) / 2;

		/* create window */
		hwnd = CreateWindow(MAKEINTATOM(filewnd_classatom), filewnd_name,
			WS_POPUPWINDOW|WS_DLGFRAME|WS_CAPTION, win_x, win_y, win_w, win_h,
			hwndowner, NULL, uidata->h_inst, ctx);

		if(hwnd != NULL)
		{
			ShowWindow(hwnd, SW_SHOWNORMAL);
			return hwnd;
		}

		free(ctx);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

int filewnd_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcx;

	if(filewnd_classatom == 0)
	{
		memset(&wcx, 0, sizeof(wcx));
		wcx.cbSize = sizeof(wcx);
		wcx.lpfnWndProc = filewnd_proc;
		wcx.hInstance = uidata->h_inst;
		wcx.hIcon = uidata->hicn_main;
		wcx.hCursor = uidata->hcur_main;
		wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx.lpszClassName = filewnd_classname;
		wcx.hIconSm = uidata->hicn_sm;

		filewnd_classatom = RegisterClassEx(&wcx);
	}

	return (filewnd_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void filewnd_unregisterclass(HINSTANCE h_inst)
{
	if(filewnd_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(filewnd_classatom), h_inst);
		filewnd_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
