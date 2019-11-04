/* ---------------------------------------------------------------------------------------------- */

#include "rtlsdr.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *rtlwnd_classname = _T("rxtest_rtl_config");
static TCHAR *rtlwnd_name = _T("RTL-SDR configuration");

static ATOM rtlwnd_classatom;

/* ---------------------------------------------------------------------------------------------- */

/* tuner name list */
static TCHAR *rtlwnd_tunername[] =
{
	_T(""),			/* Unknown */
	_T("E4000"),	/* Elonics E4000 */
	_T("FC0012"),	/* Fitipower FC0012 */
	_T("FC0013"),	/* Fitipower FC0013 */
	_T("FC2580"),	/* FCI FC2580 */
	_T("R820T"),	/* Rafael Micro R820T */
	_T("R828D")		/* Rafael Micro R828D */
};

/* ---------------------------------------------------------------------------------------------- */

/* presets for sampling rate combo box */
unsigned int rtlwnd_samprate[] = 
{
	 300000,
	1200000,
	1600000,
	2400000,
};

/* ---------------------------------------------------------------------------------------------- */

static int rtlwnd_compareint(const int *a, const int *b)
{
	if(*a < *b) return -1;
	if(*a > *b) return 1;
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void rtlwnd_selectdevice(rtlwnd_ctx_t *ctx, int index)
{
	int *pvalue, pos;
	unsigned int num;
	rtlsdr_deviceinfo_t *devinfo;
	rtlsdr_deviceconfig_t *devcfg;
	int offset_tuning;

	/* check for valid device selected */
	if( (index < 0) || (index >= ctx->mod->devicecount) ||
		(!ctx->mod->devicedata[index].is_valid) )
	{
		/* clear tuner type window */
		EnableWindow(ctx->hwndEditTunerType, FALSE);
		Edit_SetText(ctx->hwndEditTunerType, _T(""));

		/* disable and clear direct sampling mode selection */
		EnableWindow(ctx->hwndCbDirectSampMode, FALSE);
		ComboBox_SetCurSel(ctx->hwndCbDirectSampMode, 0);

		/* disable and clear sampling rate selection */
		EnableWindow(ctx->hwndCbSampRate, FALSE);
		ComboBox_SetText(ctx->hwndCbSampRate, _T(""));

		/* disable and clear offset tuning mode checkbox */
		EnableWindow(ctx->hwndBtnOffsetTuning, FALSE);
		Button_SetCheck(ctx->hwndBtnOffsetTuning, BST_UNCHECKED);

		/* disable and clear manual tuner gain mode checkbox */
		EnableWindow(ctx->hwndBtnTunerAgc, FALSE);
		Button_SetCheck(ctx->hwndBtnTunerAgc, BST_UNCHECKED);

		/* disable and clear digital agc mode checkbox */
		EnableWindow(ctx->hwndBtnRtlAgc, FALSE);
		Button_SetCheck(ctx->hwndBtnRtlAgc, BST_UNCHECKED);

		/* disable and reset tuner gain trackbar */
		EnableWindow(ctx->hwndTbTunerGain, FALSE);
		SendMessage(ctx->hwndTbTunerGain, TBM_SETRANGE, FALSE, MAKELPARAM(0, 0));

		/* clear tuner gain value window */
		Static_SetText(ctx->hwndStaticTunerGainOut, _T(""));

		/* disable and clear frequency correction edit box */
		EnableWindow(ctx->hwndEditFreqCorr, FALSE);
		Edit_SetText(ctx->hwndEditFreqCorr, _T(""));

		/* disable and clear buffer count edit box */
		EnableWindow(ctx->hwndEditBufCount, FALSE);
		Edit_SetText(ctx->hwndEditBufCount, _T(""));

		/* disable and clear buffer length edit box */
		EnableWindow(ctx->hwndEditBufLenMs, FALSE);
		Edit_SetText(ctx->hwndEditBufLenMs, _T(""));
	}
	else
	{
		devinfo = &(ctx->mod->devicedata[index].info);
		devcfg = &(ctx->mod->devicedata[index].cfg);

		/* show current device tuner type */
		EnableWindow(ctx->hwndEditTunerType, TRUE);
		Edit_SetText(ctx->hwndEditTunerType, rtlwnd_tunername[devinfo->tuner_type]);

		/* set current direct sampling mode selection */
		EnableWindow(ctx->hwndCbDirectSampMode, (ctx->mod->s != NULL) ? FALSE : TRUE);
		ComboBox_SetCurSel(ctx->hwndCbDirectSampMode, devcfg->direct_sampling);

		/* set current sampling rate */
		EnableWindow(ctx->hwndCbSampRate, (ctx->mod->s != NULL) ? FALSE : TRUE);
		ui_set_int(ctx->uidata, ctx->hwndCbSampRate, (int)(devcfg->fs));

		/* set offset tuning mode checkbox state */
		EnableWindow(ctx->hwndBtnOffsetTuning, devinfo->is_offset_tuning_supported);
		offset_tuning = devinfo->is_offset_tuning_supported && devcfg->offset_tuning_on;
		Button_SetCheck(ctx->hwndBtnOffsetTuning, offset_tuning ? BST_CHECKED : BST_UNCHECKED);

		/* set digital agc mode checkbox state */
		EnableWindow(ctx->hwndBtnRtlAgc, TRUE);
		Button_SetCheck(ctx->hwndBtnRtlAgc, devcfg->rtl_agc_on ? BST_CHECKED : BST_UNCHECKED);

		/* check manual tuner gain setting allowed */
		if(devinfo->num_tuner_gains <= 0)
		{
			/* disable manual tuner gain mode checkbox */
			EnableWindow(ctx->hwndBtnTunerAgc, FALSE);
			Button_SetCheck(ctx->hwndBtnTunerAgc, BST_CHECKED);

			/* disable and reset tuner gain trackbar */
			EnableWindow(ctx->hwndTbTunerGain, FALSE);
			SendMessage(ctx->hwndTbTunerGain, TBM_SETRANGE, FALSE, MAKELPARAM(0, 0));

			/* clear tuner gain window */
			Static_SetText(ctx->hwndStaticTunerGainOut, _T(""));
		}
		else
		{
			/* set manual gain checkbox state */
			EnableWindow(ctx->hwndBtnTunerAgc, TRUE);
			Button_SetCheck(ctx->hwndBtnTunerAgc,
				(!devcfg->tuner_gain_manual) ? BST_CHECKED : BST_UNCHECKED);

			/* setup manual gain trackbar */
			EnableWindow(ctx->hwndTbTunerGain, devcfg->tuner_gain_manual);
			SendMessage(ctx->hwndTbTunerGain, TBM_SETRANGE, TRUE,
				MAKELPARAM(0, devinfo->num_tuner_gains - 1));

			/* find index of current gain value */
			num = devinfo->num_tuner_gains;
			pvalue = _lfind(&(devcfg->tuner_gain_value), devinfo->tuner_gains_buf, 
				&num, sizeof(int), rtlwnd_compareint);

			if(pvalue != NULL)
			{
				pos = (int)(pvalue - devinfo->tuner_gains_buf);
				/* move trackbar slider to actual tuner gain value */
				SendMessage(ctx->hwndTbTunerGain, TBM_SETPOS, TRUE, pos);
				/* show actual tuner gain value in dB */
				_stprintf(ctx->uidata->databuf, _T("%.1f dB"), 
					0.1 * (double)(devinfo->tuner_gains_buf[pos]));
				Static_SetText(ctx->hwndStaticTunerGainOut, ctx->uidata->databuf);
			}
		}

		/* set current frequency correction */
		EnableWindow(ctx->hwndEditFreqCorr, TRUE);
		ui_set_int(ctx->uidata, ctx->hwndEditFreqCorr, devcfg->freqcorr);

		/* set buffer count */
		EnableWindow(ctx->hwndEditBufCount, (ctx->mod->s != NULL) ? FALSE : TRUE);
		ui_set_int(ctx->uidata, ctx->hwndEditBufCount, devcfg->buf_count);

		/* set buffer length */
		EnableWindow(ctx->hwndEditBufLenMs, (ctx->mod->s != NULL) ? FALSE : TRUE);
		ui_set_double(ctx->uidata, ctx->hwndEditBufLenMs, devcfg->buf_len_ms, 0, 6, 1);
	}

	/* save selected device index */
	ctx->selected_index = index;
}

/* ---------------------------------------------------------------------------------------------- */

/* notify window when stream started/stopped */
void rtlwnd_setstarted(rtlwnd_ctx_t *ctx, BOOL started)
{
	/* if configuration window opened */
	if(ctx != NULL)
	{
		/* select current device before stream start */
		if( started && (ctx->selected_index != ctx->mod->deviceindex) )
			rtlwnd_selectdevice(ctx, ctx->mod->deviceindex);

		/* disable changing device and sampling mode/rate when stream started */
		EnableWindow(ctx->hwndCbDevice, !started);
		EnableWindow(ctx->hwndCbDirectSampMode, !started);
		EnableWindow(ctx->hwndCbSampRate, !started);
		EnableWindow(ctx->hwndEditBufCount, !started);
		EnableWindow(ctx->hwndEditBufLenMs, !started);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* read device configuration from dialog controls */
static int rtlwnd_readconf(rtlwnd_ctx_t *ctx, rtlsdr_deviceconfig_t *devcfg,
						   rtlsdr_deviceinfo_t *devinfo)
{
	int pos, fs;

	/* get selected direct sampling mode */
	devcfg->direct_sampling = ComboBox_GetCurSel(ctx->hwndCbDirectSampMode);
	if((devcfg->direct_sampling < 0) || (devcfg->direct_sampling > 2))
		return 0;

	/* get entered sampling rate */
	if( !ui_get_int(ctx->uidata, ctx->hwnd,
		ctx->hwndCbSampRate, &fs, _T("Sampling rate")) )
	{
		return 0;
	}

	/* check entered sampling rate */
	if( ((fs < 225001) || (fs > 300000)) &&
		((fs < 900001) || (fs > 3200000)) )
	{
		MessageBox(ctx->hwnd, _T("Sampling rate must be in range of ")
			_T("225001 to 300000 Hz or 900001 to 3200000 Hz."), 
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	devcfg->fs = (unsigned int)fs;

	/* get offset tuning mode */
	if(!devinfo->is_offset_tuning_supported)
	{
		devcfg->offset_tuning_on = 0;
	}
	else
	{
		devcfg->offset_tuning_on =
			((Button_GetCheck(ctx->hwndBtnOffsetTuning) & BST_CHECKED) != 0) ? 1 : 0;
	}

	/* get digital agc mode */
	devcfg->rtl_agc_on =
		((Button_GetCheck(ctx->hwndBtnRtlAgc) & BST_CHECKED) != 0) ? 1 : 0;

	/* check for manual tuner gain setting allowed */
	if(devinfo->num_tuner_gains <= 0)
	{
		devcfg->tuner_gain_manual = 0;
		devcfg->tuner_gain_value = 0;
	}
	else
	{
		/* get manual tuner gain checkbox state */
		devcfg->tuner_gain_manual = 
			(!((Button_GetCheck(ctx->hwndBtnTunerAgc) & BST_CHECKED) != 0)) ? 1 : 0;

		/* get selected tuner gain value */
		pos = (int)SendMessage(ctx->hwndTbTunerGain, TBM_GETPOS, 0, 0);
		if((pos < 0) || (pos >= devinfo->num_tuner_gains))
			return 0;
		devcfg->tuner_gain_value = devinfo->tuner_gains_buf[pos];
	}

	/* get frequency correction value */
	if( !ui_get_int_range(ctx->uidata, ctx->hwnd, ctx->hwndEditFreqCorr, &(devcfg->freqcorr), 
		RTLSDR_FREQCORR_MIN, RTLSDR_FREQCORR_MAX, _T("Frequency correction"), _T(" ppm")) )
	{
		return 0;
	}

	/* get buffer count */
	if( !ui_get_int_range(ctx->uidata, ctx->hwnd, ctx->hwndEditBufCount, &(devcfg->buf_count), 
		RTLSDR_BUFFER_COUNT_MIN, RTLSDR_BUFFER_COUNT_MAX, _T("Buffer count"), _T("")) )
	{
		return 0;
	}

	/* get buffer length */
	if( !ui_get_double_range(ctx->uidata, ctx->hwnd, ctx->hwndEditBufLenMs, &(devcfg->buf_len_ms), 
		RTLSDR_BUFFER_MS_MIN, RTLSDR_BUFFER_MS_MAX, _T("Buffer length"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* show current manual tuner gain */
static void rtlwnd_showtunergain(rtlwnd_ctx_t *ctx)
{
	rtlsdr_deviceinfo_t *devinfo;
	int pos;

	/* check for valid device selected */
	if( (ctx->selected_index >= 0) && (ctx->selected_index < ctx->mod->devicecount) &&
		ctx->mod->devicedata[ctx->selected_index].is_valid )
	{
		devinfo = &(ctx->mod->devicedata[ctx->selected_index].info);

		/* get tuner gain trackbar position */
		pos = (int)SendMessage(ctx->hwndTbTunerGain, TBM_GETPOS, 0, 0);
		if((pos < 0) || (pos >= devinfo->num_tuner_gains))
			return;

		/* show actual tuner gain in dB */
		_stprintf(ctx->uidata->databuf, _T("%.1f dB"),
			0.1 * (double)(devinfo->tuner_gains_buf[pos]));
		Static_SetText(ctx->hwndStaticTunerGainOut, ctx->uidata->databuf);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle device selection change */
static int rtlwnd_changedeviceselection(rtlwnd_ctx_t *ctx)
{
	int index;
	rtlsdr_deviceconfig_t devcfg;
	rtlsdr_deviceinfo_t *devinfo;

	/* check stream is not started */
	if(ctx->mod->s != NULL)
		return 0;

	/* get index of new selected device */
	index = ComboBox_GetCurSel(ctx->hwndCbDevice) - 1;
	if(index == ctx->selected_index)
		return 0;

	/* check previous device is valid */
	if( (ctx->selected_index >= 0) && (ctx->selected_index < ctx->mod->devicecount) &&
		ctx->mod->devicedata[ctx->selected_index].is_valid )
	{
		/* get entered device configuration */
		devinfo = &(ctx->mod->devicedata[ctx->selected_index].info);
		memcpy(&devcfg, &(ctx->mod->devicedata[ctx->selected_index].cfg),
			sizeof(rtlsdr_deviceconfig_t));
		if(!rtlwnd_readconf(ctx, &devcfg, devinfo))
			return 0;

		/* check device configuration changed */
		if(memcmp(&devcfg, &(ctx->mod->devicedata[ctx->selected_index].cfg),
			sizeof(rtlsdr_deviceconfig_t)) != 0)
		{
			/* confirm configuration save */
			_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Save configuration for device [%d] %s?"),
				ctx->selected_index, ctx->mod->devicedata[ctx->selected_index].info.name);
			if(MessageBox(ctx->hwnd, ctx->uidata->msgbuf,
				ui_title, MB_ICONQUESTION|MB_YESNO) == IDYES)
			{
				/* save device configuration */
				memcpy(&(ctx->mod->devicedata[ctx->selected_index].cfg), &devcfg,
					sizeof(rtlsdr_deviceconfig_t));
			}
		}
	}

	/* select new device */
	rtlwnd_selectdevice(ctx, index);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* save settings from device configuration window */
static int rtlwnd_save(rtlwnd_ctx_t *ctx)
{
	rtlsdr_deviceconfig_t devcfg;
	rtlsdr_deviceinfo_t *devinfo;

	/* check device is selected */
	if((ctx->selected_index < 0) || (ctx->selected_index >= ctx->mod->devicecount))
	{
		ctx->mod->deviceindex = -1;
		return 1;
	}

	/* check selected device valid */
	if(!ctx->mod->devicedata[ctx->selected_index].is_valid)
	{
		ctx->mod->deviceindex = ctx->selected_index;
		return 1;
	}

	/* check stream currently started */
	if(ctx->mod->s != NULL)
	{
		/* check selected device is current opened (should be) */
		if(ctx->selected_index != ctx->mod->s->openeddeviceindex)
			return 0;

		/* get device configuration from window */
		memcpy(&devcfg, ctx->mod->s->devcfg, sizeof(rtlsdr_deviceconfig_t));
		if(!rtlwnd_readconf(ctx, &devcfg, ctx->mod->s->devinfo))
			return 0;

		/* update active device configuration */
		if(!rtlsdr_updateconfig(ctx->mod, ctx->mod->s->devcfg, &devcfg, ctx->mod->s->dev,
			ctx->mod->s->devinfo, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size))
		{
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			return 0;
		}
	}
	else
	{
		/* get device configuration from window */
		devinfo = &(ctx->mod->devicedata[ctx->selected_index].info);
		memcpy(&devcfg, &(ctx->mod->devicedata[ctx->selected_index].cfg),
			sizeof(rtlsdr_deviceconfig_t));
		if(!rtlwnd_readconf(ctx, &devcfg, devinfo))
			return 0;

		/* save device configuration */
		memcpy(&(ctx->mod->devicedata[ctx->selected_index].cfg), &devcfg,
			sizeof(rtlsdr_deviceconfig_t));

		/* save selected device index */
		ctx->mod->deviceindex = ctx->selected_index;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize device selection list */
static void rtlwnd_initdevicelist(rtlwnd_ctx_t *ctx)
{
	int i;
	rtlsdr_deviceinfo_t *devinfo;

	/* add default entry */
	ComboBox_AddString(ctx->hwndCbDevice, _T("Not selected"));

	/* add entries for devices found */
	for(i = 0; i < ctx->mod->devicecount; i++)
	{
		devinfo = &(ctx->mod->devicedata[i].info);
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("[%d] %s"), i, devinfo->name);
		ComboBox_AddString(ctx->hwndCbDevice, ctx->uidata->msgbuf);
	}

	/* select current device */
	if((ctx->mod->deviceindex >= 0) && (ctx->mod->deviceindex < ctx->mod->devicecount)) {
		ComboBox_SetCurSel(ctx->hwndCbDevice, ctx->mod->deviceindex + 1);
		rtlwnd_selectdevice(ctx, ctx->mod->deviceindex);
	} else {
		ComboBox_SetCurSel(ctx->hwndCbDevice, 0);
		rtlwnd_selectdevice(ctx, -1);
	}

	/* disable device changing if stream started */
	EnableWindow(ctx->hwndCbDevice, (ctx->mod->s != NULL) ? FALSE : TRUE);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle window initialization */
static int rtlwnd_init(rtlwnd_ctx_t *ctx)
{
	int i, n;

	/* create device selection controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 235, 15, _T("RTL-SDR device:"), ID_CTL_STATIC);
	ctx->hwndCbDevice = ui_crt_combo(ctx->uidata, ctx->hwnd, WS_TABSTOP|CBS_DROPDOWNLIST,
		10, 30, 235, 200, RTLWND_ID_DEVICE);

	/* create tuner type display controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		250, 10, 60, 15, _T("Tuner:"), ID_CTL_STATIC);
	ctx->hwndEditTunerType = ui_crt_edit(ctx->uidata, ctx->hwnd, ES_READONLY,
		250, 30, 60, 21, RTLWND_ID_TUNERTYPE);

	/* create direct sampling mode controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 60, 170, 15, _T("Direct sampling:"), ID_CTL_STATIC);
	ctx->hwndCbDirectSampMode = ui_crt_combo(ctx->uidata, ctx->hwnd, WS_TABSTOP|CBS_DROPDOWNLIST,
		10, 80, 170, 200, RTLWND_ID_SAMPMODE);

	ComboBox_AddString(ctx->hwndCbDirectSampMode, _T("Off"));
	ComboBox_AddString(ctx->hwndCbDirectSampMode, _T("I-ADC input"));
	ComboBox_AddString(ctx->hwndCbDirectSampMode, _T("Q-ADC input"));

	/* create sampling rate controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		190, 60, 100, 15, _T("Sampling rate:"), ID_CTL_STATIC);
	ctx->hwndCbSampRate = ui_crt_combo(ctx->uidata, ctx->hwnd, WS_TABSTOP|CBS_DROPDOWN,
		190, 80, 100, 200, RTLWND_ID_SAMPRATE);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		295, 83, 25, 15, _T("Hz"), ID_CTL_STATIC);

	n = sizeof(rtlwnd_samprate) / sizeof(rtlwnd_samprate[0]);
	for(i = 0; i < n; i++)
	{
		_stprintf(ctx->uidata->databuf, _T("%u"), rtlwnd_samprate[i]); 
		ComboBox_AddString(ctx->hwndCbSampRate, ctx->uidata->databuf);
	}

	/* create mode checkboxes */
	ctx->hwndBtnOffsetTuning = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		10, 110, 120, 15, _T("Offset tuning"), RTLWND_ID_OFFSETTUNING);
	ctx->hwndBtnTunerAgc = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		10, 135, 120, 15, _T("Tuner AGC"), RTLWND_ID_TUNERAGC);
	ctx->hwndBtnRtlAgc = ui_crt_btn(ctx->uidata, ctx->hwnd, WS_TABSTOP|BS_AUTOCHECKBOX,
		10, 160, 120, 15, _T("RTL2832 AGC"), RTLWND_ID_RTLAGC);

	/* create manual tuner gain controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		130, 110, 80, 15, _T("Tuner gain"), ID_CTL_STATIC);
	ctx->hwndStaticTunerGainOut = ui_crt_static(ctx->uidata, ctx->hwnd, SS_RIGHT,
		230, 110, 80, 15, _T(""), RTLWND_ID_TUNERGAINOUT);
	ctx->hwndTbTunerGain = ui_crt_track(ctx->uidata, ctx->hwnd, WS_TABSTOP|TBS_HORZ|TBS_AUTOTICKS,
		130, 130, 180, 45, RTLWND_ID_TUNERGAIN);

	/* create frequency correction controls */
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 188, 130, 15, _T("Frequency correction:"), ID_CTL_STATIC);
	ctx->hwndEditFreqCorr = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 210, 45, 21, RTLWND_ID_FREQCORR);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		60, 213, 25, 15, _T("ppm"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		160, 183, 85, 15, _T("Buffer count"), ID_CTL_STATIC);
	ctx->hwndEditBufCount = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		245, 180, 45, 21, RTLWND_ID_BUFCOUNT);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		160, 213, 85, 15, _T("Buffer length"), ID_CTL_STATIC);
	ctx->hwndEditBufLenMs = ui_crt_edit(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		245, 210, 45, 21, RTLWND_ID_BUFLENMS);
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		295, 213, 25, 15, _T("ms"), ID_CTL_STATIC);

	/* create confirmation controls */
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		10, 245, 65, 21, _T("Cancel"), RTLWND_ID_CANCEL);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		175, 245, 65, 21, _T("Apply"), RTLWND_ID_APPLY);
	ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		245, 245, 65, 21, _T("Ok"), RTLWND_ID_OK);

	/* initialize device list and select current device */
	ctx->selected_index = -1;
	rtlwnd_initdevicelist(ctx);

	/* set window data pointer */
	ctx->mod->cfgwnd = ctx;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle window destroying */
static void rtlwnd_destroy(rtlwnd_ctx_t *ctx)
{
	/* clear window data pointer */
	ctx->mod->cfgwnd = NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* clean up window data */
static void rtlwnd_cleanup(rtlwnd_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

/* device configuration window msgproc */
static LRESULT CALLBACK rtlwnd_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	rtlwnd_ctx_t *ctx;

	switch(umsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lp)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!rtlwnd_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		rtlwnd_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		rtlwnd_cleanup(ctx);
		return 0;

	case WM_COMMAND:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wp))
		{
		case RTLWND_ID_DEVICE:
			switch(HIWORD(wp))
			{
			case CBN_SELCHANGE:
				rtlwnd_changedeviceselection(ctx);
				return 0;
			}
			break;
		case RTLWND_ID_TUNERAGC:
			EnableWindow(ctx->hwndTbTunerGain,
				(Button_GetCheck(ctx->hwndBtnTunerAgc) & BST_CHECKED) == 0);
			break;
		case RTLWND_ID_APPLY:
			rtlwnd_save(ctx);
			return 0;
		case RTLWND_ID_CANCEL:
			DestroyWindow(hwnd);
			return 0;
		case RTLWND_ID_OK:
			if(rtlwnd_save(ctx))
				DestroyWindow(hwnd);
			return 0;
		}
		break;

	case WM_HSCROLL:
		ctx = (void*)GetWndPtr(hwnd, GWLP_USERDATA);
		if((HWND)lp == ctx->hwndTbTunerGain)
			rtlwnd_showtunergain(ctx);
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

/* create rtl-sdr device configuration window */
HWND rtlwnd_create(uicommon_t *uidata, HWND hwndowner, rtlsdr_ctx_t *mod)
{
	rtlwnd_ctx_t *ctx;
	int win_w, win_h, win_x, win_y;
	RECT ownerwndrect;
	HWND hwnd;

	if( (ctx = calloc(1, sizeof(rtlwnd_ctx_t))) != NULL )
	{
		ctx->uidata = uidata;
		ctx->mod = mod;

		/* calculate window pos */
		win_w = RTLWND_W +
			GetSystemMetrics(SM_CXDLGFRAME) * 2;
		win_h = RTLWND_H +
			GetSystemMetrics(SM_CYDLGFRAME) * 2 +
			GetSystemMetrics(SM_CYCAPTION);

		GetWindowRect(hwndowner, &ownerwndrect);
		win_x = (ownerwndrect.left + ownerwndrect.right - win_w) / 2;
		win_y = (ownerwndrect.top + ownerwndrect.bottom - win_h) / 2;

		/* create window */
		hwnd = CreateWindow(MAKEINTATOM(rtlwnd_classatom), rtlwnd_name,
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

/* register rtl-sdr device configuration window */
int rtlwnd_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcx;

	if(rtlwnd_classatom == 0)
	{
		memset(&wcx, 0, sizeof(wcx));
		wcx.cbSize = sizeof(wcx);
		wcx.lpfnWndProc = rtlwnd_proc;
		wcx.hInstance = uidata->h_inst;
		wcx.hIcon = uidata->hicn_main;
		wcx.hCursor = uidata->hcur_main;
		wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx.lpszClassName = rtlwnd_classname;
		wcx.hIconSm = uidata->hicn_sm;

		rtlwnd_classatom = RegisterClassEx(&wcx);
	}

	return (rtlwnd_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* unregister rtl-sdr device configuration window */
void rtlwnd_unregisterclass(HINSTANCE h_inst)
{
	if(rtlwnd_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(rtlwnd_classatom), h_inst);
		rtlwnd_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
