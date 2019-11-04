/* ---------------------------------------------------------------------------------------------- */

#include "pathutil.h"

/* ---------------------------------------------------------------------------------------------- */

/* get path relative to main program module */
int get_local_path(TCHAR *pathbuf, size_t buflen, TCHAR *what)
{
	int res;
	TCHAR *fnamecur, *p;
	size_t basepathlen;

	/* get current module full path */
	res = GetModuleFileName(NULL, pathbuf, (int)buflen);
	if((res == 0) || (res == (int)buflen))
		return 0;

	/* find filename */
	fnamecur = pathbuf;
	for(p = pathbuf; *p != 0; p++)
	{
		if((*p == _T('\\')) || (*p == _T('/')))
			fnamecur = p + 1;
	}

	/* check buffer space enough */
	basepathlen = fnamecur - pathbuf;
	if(basepathlen + _tcslen(what) + 1 > buflen)
		return 0;

	/* replace filename with what needed */
	_tcscpy(fnamecur, what);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* appned path to %path% variable */
int append_path_env(TCHAR *appendwhat)
{
	TCHAR *buf, *buftmp, *p;
	DWORD res, buflen, buflen_req;
	TCHAR *curvar, *varsep;
	int already_contains;

	buflen = APPEND_PATH_ENV_BUFLEN;

	/* alloc buffer */
	if( (buf = malloc(buflen * sizeof(TCHAR))) == NULL )
		return 0;

	/* try getting %path% value */
	res = GetEnvironmentVariable(_T("PATH"), buf, buflen);

	/* check for error */
	if(res == 0)
	{
		if(GetLastError() != ERROR_ENVVAR_NOT_FOUND)
			goto __cleanup_and_return;
		*buf = 0;
	}

	/* expand buffer and try again */
	if(res >= buflen)
	{
		/* expand buffer */
		buflen = res + (DWORD)_tcslen(appendwhat) + 2;
		if( (buftmp = realloc(buf, buflen * sizeof(TCHAR))) == NULL )
			goto __cleanup_and_return;
		buf = buftmp;

		/* try getting %path% value again */
		res = GetEnvironmentVariable(_T("PATH"), buf, buflen);
		if((res == 0) || (res >= buflen))
			goto __cleanup_and_return;
	}

	/* check path already contains specified substring */
	curvar = buf;
	already_contains = 0;
	for( ; ; )
	{
		/* find substring separator and insert terminator temporarly */
		varsep = _tcschr(curvar, _T(';'));
		if(varsep != NULL)
			*varsep = 0;

		/* compare current substring with appended string */
		if(_tcsicmp(curvar, appendwhat) == 0) {
			already_contains = 1;
			break;
		}

		/* exit if done */
		if(varsep == NULL)
			break;

		/* restore separator and check next substring */
		*varsep = _T(';');
		curvar = varsep + 1;
	}

	/* append %path% if not it already contains specified substring */
	if(!already_contains)
	{
		/* calculate required buffer length */
		buflen_req = (DWORD)(_tcslen(buf) + _tcslen(appendwhat) + 2);

		/* expand buffer if required */
		if(buflen_req > buflen)
		{
			if((buftmp = realloc(buf, buflen_req * sizeof(TCHAR))) == NULL)
				goto __cleanup_and_return;
			buf = buftmp;
			buflen = buflen_req;
		}

		/* append new substring  to path */
		p = buf + _tcslen(buf);

		if(p != buf) {
			*(p++) = _T(';');
		}

		_tcscpy(p, appendwhat);

		/* set new variable */
		if(!SetEnvironmentVariable(_T("PATH"), buf))
			goto __cleanup_and_return;
	}

	free(buf);
	return 1;

__cleanup_and_return:
	free(buf);
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
