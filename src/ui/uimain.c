/* ---------------------------------------------------------------------------------------------- */

#include "uimain.h"

/* ---------------------------------------------------------------------------------------------- */

static int registerclasses(uicommon_t *uidata)
{
	return (
		rxwnd_registerclass(uidata) &&
		freqwnd_registerclass(uidata) &&
		cmwnd_registerclass(uidata) &&
		cmcfg_registerclass(uidata) &&
		setwnd_registerclass(uidata) &&
		audctrl_registerclass(uidata) &&
		filtcfg_registerclass(uidata) &&
		chanopt_registerclass(uidata) &&
		sqlcfg_registerclass(uidata) &&
		fmcfg_registerclass(uidata) &&
		filewnd_registerclass(uidata) &&
		rtlwnd_registerclass(uidata) &&
		lbr_registerclass(uidata->h_inst) );
}

/* ---------------------------------------------------------------------------------------------- */

static void unregisterclasses(HINSTANCE h_inst)
{
	rxwnd_unregisterclass(h_inst);
	freqwnd_unregisterclass(h_inst);
	cmwnd_unregisterclass(h_inst);
	cmcfg_unregisterclass(h_inst);
	setwnd_unregisterclass(h_inst);
	audctrl_unregisterclass(h_inst);
	filtcfg_unregisterclass(h_inst);
	chanopt_unregisterclass(h_inst);
	sqlcfg_unregisterclass(h_inst);
	fmcfg_unregisterclass(h_inst);
	filewnd_unregisterclass(h_inst);
	rtlwnd_unregisterclass(h_inst);
	lbr_unregisterclass(h_inst);
}

/* ---------------------------------------------------------------------------------------------- */

int uimain(HINSTANCE h_inst, int n_show, ini_data_t *ini, ini_data_t *inich)
{
	uicommon_t uidata;
	callback_list_t cb_list;
	HWND hwnd;
	MSG msg;

	/* initialize common resources */
	if(uicommon_init(&uidata, h_inst))
	{
		/* register all classes */
		if(!registerclasses(&uidata))
		{
			MessageBox(NULL, _T("Can't register window classes."),
				ui_title, MB_ICONEXCLAMATION|MB_OK);
		}
		else
		{
			/* allocate window notification list */
			if(callback_list_init(&cb_list, 16))
			{
				hwnd = rxwnd_create(&uidata, ini, inich, &cb_list, n_show);

				if(hwnd != NULL)
				{
					while(GetMessage(&msg, NULL, 0, 0))
					{
						if( (uidata.accel.hwnd != NULL) && (uidata.accel.haccel != NULL) &&
							TranslateAccelerator(uidata.accel.hwnd, uidata.accel.haccel, &msg) )
						{
							continue;
						}

						if(keynav_msghandle(&(uidata.keynav), &msg))
							continue;

						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}

				callback_list_cleanup(&cb_list);
			}
		}

		/* unregister all registered classes */
		unregisterclasses(h_inst);

		/* free common resources */
		uicommon_free(&uidata);
	}


	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
