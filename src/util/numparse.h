/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* ---------------------------------------------------------------------------------------------- */

int parse_int(TCHAR *buf, int *pout);

/* ---------------------------------------------------------------------------------------------- */

/* format double number */
TCHAR *fmt_dbl(TCHAR *buf, size_t bufsize, double value, int prefix, int initfrac, int minfrac);

/* parse double number */
int parse_dbl(TCHAR *buf, int allow_prefix, double *p_value, int *p_used_prefix);

/* ---------------------------------------------------------------------------------------------- */

/* format rgb number */
TCHAR *fmt_rgb(TCHAR *buf, COLORREF cr);

/* parse rgb number */
int parse_rgb(TCHAR *buf, COLORREF *out);

/* ---------------------------------------------------------------------------------------------- */

void append_dbl(TCHAR *buf, TCHAR **pptr, size_t bufsize,
				double value, int prefix, int initfrac, int minfrac);

/* ---------------------------------------------------------------------------------------------- */
