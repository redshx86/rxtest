/* ---------------------------------------------------------------------------------------------- */

#include <windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <locale.h>
#include "util/iniparse.h"
#include "dsp/util/cpuid.h"
#include "ui/uimain.h"

/* ---------------------------------------------------------------------------------------------- */

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR szCmdLine, int nCmdShow)
{
	ini_file_t inif_main, inif_chan;
	int ini_main_loaded, ini_chan_loaded;

	_tsetlocale(LC_COLLATE, _T(""));
	_tsetlocale(LC_CTYPE, _T(""));

	_set_SSE2_enable(0);
	check_cpu_features();

	ini_main_loaded = ini_file_load(&inif_main, _T("rxtest.ini"));
	ini_chan_loaded = ini_file_load(&inif_chan, _T("rxchan.ini"));

	if((!ini_main_loaded) || (!ini_chan_loaded))
	{
		MessageBox(NULL, _T("Can't load configuration files."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
	}
	else
	{
		uimain(hInst, nCmdShow, inif_main.ini, inif_chan.ini);

		ini_file_save(&inif_main);
		ini_file_save(&inif_chan);
	}

	if(ini_main_loaded)
		ini_file_cleanup(&inif_main);
	if(ini_chan_loaded)
		ini_file_cleanup(&inif_chan);

	_CrtDumpMemoryLeaks();
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
