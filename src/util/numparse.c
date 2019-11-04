/* ---------------------------------------------------------------------------------------------- */

#include "numparse.h"

/* ---------------------------------------------------------------------------------------------- */

int parse_int(TCHAR *buf, int *pout)
{
	TCHAR *endptr;

	*pout = _tcstol(buf, &endptr, 10);

	while(_istspace(*endptr))
		endptr++;

	return (*endptr == 0);
}

/* ---------------------------------------------------------------------------------------------- */

TCHAR *fmt_dbl(TCHAR *buf, size_t bufsize, double value, int prefix, int initfrac, int minfrac)
{
	TCHAR *p, *fracptr;
	int n, trimfrac;

	/* scale for thousands prefix */
	switch(prefix)
	{
	case 1:
		value *= 1e-3;
		initfrac += 3;
		break;
	case 2:
		value *= 1e-6;
		initfrac += 6;
		break;
	case 3:
		value *= 1e-9;
		initfrac += 9;
		break;
	}

	/* format value */
	n = _sntprintf(buf, bufsize, _T("%.*f"), initfrac, value);

	if( (n <= 0) || (n >= (int)bufsize) ) {
		_tcscpy(buf, _T("?"));
		return buf;
	}

	/* is trailing zeros trimming requested? */
	if(minfrac < initfrac)
	{
		/* find decimal point and check format */
		if( ((fracptr = _tcsrchr(buf, _T('.'))) != NULL) &&
			(_tcslen(fracptr) == (size_t)(initfrac + 1)) )
		{
			/* trim trailing zeros */
			p = fracptr + initfrac;
			trimfrac = initfrac - minfrac;

			while( (trimfrac > 0) && (*p == _T('0')) )
			{
				*(p--) = 0;
				trimfrac--;
			}

			/* trim decimal point */
			if(*p == _T('.'))
				*p = 0;
		}
	}

	/* append thousands prefix */
	switch(prefix)
	{
	case 1: _tcscat(buf, _T("k")); break;
	case 2: _tcscat(buf, _T("M")); break;
	case 3: _tcscat(buf, _T("G")); break;
	}

	return buf;
}

/* ---------------------------------------------------------------------------------------------- */

/* parse double number */
int parse_dbl(TCHAR *buf, int allow_prefix, double *p_value, int *p_used_prefix)
{
	TCHAR *endptr;
	double value;
	int used_prefix;

	value = _tcstod(buf, &endptr);
	used_prefix = 0;

	if(allow_prefix)
	{
		while(_istspace(*endptr))
			endptr++;

		switch( *endptr /*_totupper(*endptr)*/ )
		{
		case _T('k'):
			value *= 1e3;
			used_prefix = 1;
			endptr++;
			break;
		case _T('M'):
			value *= 1e6;
			used_prefix = 2;
			endptr++;
			break;
		case _T('G'):
			value *= 1e9;
			used_prefix = 3;
			endptr++;
			break;
		}
	}

	while(_istspace(*endptr))
		endptr++;

	if(*endptr != 0)
		return 0;

	if(p_value != NULL)
		*p_value = value;

	if(p_used_prefix != NULL)
		*p_used_prefix = used_prefix;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* format rgb number */
TCHAR *fmt_rgb(TCHAR *buf, COLORREF cr)
{
	_stprintf(buf, _T("%06x"), _byteswap_ulong(cr) >> 8);
	return buf;
}

/* ---------------------------------------------------------------------------------------------- */

/* parse rgb number */
int parse_rgb(TCHAR *buf, COLORREF *out)
{
	TCHAR *endptr;

	*out = _byteswap_ulong(_tcstoul(buf, &endptr, 16)) >> 8;

	while(_istspace(*endptr))
		endptr++;

	return (*endptr == 0);
}

/* ---------------------------------------------------------------------------------------------- */

void append_dbl(TCHAR *buf, TCHAR **pptr, size_t bufsize,
				double value, int prefix, int initfrac, int minfrac)
{
	TCHAR *ptr;
	size_t bufsize_remain;

	if((pptr != NULL) && (*pptr != NULL)) {
		ptr = *pptr;
	} else {
		ptr = buf;
	}

	bufsize_remain = bufsize - (ptr - buf);

	fmt_dbl(ptr, bufsize_remain, value, prefix, initfrac, minfrac);

	if(pptr != NULL)
	{
		ptr += _tcslen(ptr);
		*pptr = ptr;
	}
}

/* ---------------------------------------------------------------------------------------------- */
