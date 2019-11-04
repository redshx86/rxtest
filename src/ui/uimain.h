/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"

#include "rxwnd.h"
#include "cmwnd.h"
#include "cmcfg.h"
#include "proccfg/chanopt.h"
#include "proccfg/filtcfg.h"
#include "proccfg/sqlcfg.h"
#include "proccfg/fmcfg.h"
#include "ctrl/levelbar.h"
#include "freqwnd.h"
#include "specview.h"
#include "watrview.h"
#include "sets/setwnd.h"
#include "audctrl.h"
#include "../input/rtlsdr.h"

/* ---------------------------------------------------------------------------------------------- */

int uimain(HINSTANCE h_inst, int n_show, ini_data_t *ini, ini_data_t *inich);

/* ---------------------------------------------------------------------------------------------- */
