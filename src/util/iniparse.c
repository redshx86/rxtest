/* ---------------------------------------------------------------------------------------------- */

#include "iniparse.h"

/* ---------------------------------------------------------------------------------------------- */

static ini_entry_t *ini_ent_find(ini_sect_t *sect, TCHAR *name)
{
	ini_entry_t *ent;

	for(ent = sect->ent_head; ent != NULL; ent = ent->ent_fwd)
	{
		if(_tcscmp(ent->name, name) == 0)
			return ent;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

static ini_entry_t *ini_ent_new(ini_sect_t *sect, TCHAR *name, TCHAR *value)
{
	ini_entry_t *ent;
	size_t sizehdr, sizeentry, size;

	/* calculate memory block size for entry */
	sizehdr = sizeof(ini_entry_t) + (_tcslen(name) + 1UL) * sizeof(TCHAR);
	sizeentry = sizehdr + (_tcslen(value) + 1UL) * sizeof(TCHAR);
	size = (sizeentry + 7UL) & ~7UL;

	/* allocate entry memory */
	if( (ent = malloc(size)) == NULL )
		return NULL;

	/* initialize entry structure */
	ent->sect = sect;
	ent->name = (void*)((char*)ent + sizeof(ini_entry_t));
	ent->value = (void*)((char*)ent + sizehdr);
	ent->valuebufsize = ((size - sizehdr) / sizeof(TCHAR));

	_tcscpy(ent->name, name);
	_tcscpy(ent->value, value);

	/* insert entry to section */
	DLIST_INSBACK(ent, sect->ent_head, sect->ent_tail, ent_fwd, ent_back);

	return ent;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_set(ini_sect_t *sect, TCHAR *name, TCHAR *value)
{
	ini_entry_t *ent;

	if((sect == NULL) || (name == NULL) || (value == NULL))
		return 0;

	/* check for existing entry */
	ent = ini_ent_find(sect, name);
	if(ent != NULL)
	{
		/* replace value if buffer space is enough */
		if(ent->valuebufsize >= _tcslen(value) + 1UL)
		{
			_tcscpy(ent->value, value);
			return 1;
		}
		/* or delete current entry */
		DLIST_REMOVE(ent, sect->ent_head, sect->ent_tail, ent_fwd, ent_back);
		free(ent);
	}

	/* allocate and insert new entry */
	if( (ent = ini_ent_new(sect, name, value)) == NULL )
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_seti(ini_sect_t *sect, TCHAR *name, int value)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, _ltot(value, sect->ini->buf, 10));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_setu(ini_sect_t *sect, TCHAR *name, unsigned int value)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, _ultot(value, sect->ini->buf, 10));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_setus(ini_sect_t *sect, TCHAR *name, size_t value)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, _ultot((unsigned long)value, sect->ini->buf, 10));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_setux(ini_sect_t *sect, TCHAR *name, unsigned int value)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, _ultot(value, sect->ini->buf, 16));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_sete(ini_sect_t *sect, TCHAR *name, ini_enum_t value)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, _ltot((long)value, sect->ini->buf, 10));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_setcr(ini_sect_t *sect, TCHAR *name, COLORREF cr)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, fmt_rgb(sect->ini->buf, cr));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_setf(ini_sect_t *sect, TCHAR *name, int count, double value)
{
	if(sect == NULL)
		return 0;

	return ini_set(sect, name, fmt_dbl(sect->ini->buf, INI_BUFSIZE, value, 0, count, 1));
}

/* ---------------------------------------------------------------------------------------------- */

int ini_setb(ini_sect_t *sect, TCHAR *name, int value)
{
	return ini_set(sect, name, value ? _T("yes") : _T("no"));
}

/* ---------------------------------------------------------------------------------------------- */

TCHAR *ini_get(ini_sect_t *sect, TCHAR *name)
{
	ini_entry_t *ent;

	if((sect == NULL) || (name == NULL))
		return NULL;
	if( (ent = ini_ent_find(sect, name)) != NULL )
		return ent->value;
	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_geti(ini_sect_t *sect, TCHAR *name, int *valp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL) {
		*valp = (int)_tcstol(value, NULL, 10);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getir(ini_sect_t *sect, TCHAR *name, int *valp, int mn, int mx)
{
	int val;

	if(ini_geti(sect, name, &val) && (val >= mn) && (val <= mx)) {
		*valp = val;
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getu(ini_sect_t *sect, TCHAR *name, unsigned int *valp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL) {
		*valp = (unsigned int)_tcstoul(value, NULL, 10);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getur(ini_sect_t *sect, TCHAR *name, unsigned int *valp,
			  unsigned int mn, unsigned int mx)
{
	unsigned int val;

	if(ini_getu(sect, name, &val) && (val >= mn) && (val <= mx)) {
		*valp = val;
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getus(ini_sect_t *sect, TCHAR *name, size_t *valp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL) {
		*valp = (size_t)_tcstoul(value, NULL, 10);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getusr(ini_sect_t *sect, TCHAR *name, size_t *valp, size_t mn, size_t mx)
{
	size_t val;

	if(ini_getus(sect, name, &val) && (val >= mn) && (val <= mx)) {
		*valp = val;
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getux(ini_sect_t *sect, TCHAR *name, unsigned int *valp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL) {
		*valp = (unsigned int)_tcstoul(value, NULL, 16);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getuxr(ini_sect_t *sect, TCHAR *name, unsigned int *valp,
			   unsigned int mn, unsigned int mx)
{
	unsigned int val;

	if(ini_getux(sect, name, &val) && (val >= mn) && (val <= mx)) {
		*valp = val;
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_gete(ini_sect_t *sect, TCHAR *name, ini_enum_t *valp, ini_enum_t count)
{
	ini_enum_t val;
	
	TCHAR *str;

	if((str = ini_get(sect, name)) != NULL) {
		val = (ini_enum_t)_tcstol(str, NULL, 10);
		if((val >= 0) && (val < count)) {
			*valp = val;
			return 1;
		}
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getcr(ini_sect_t *sect, TCHAR *name, COLORREF *crp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL) {
		*crp = _byteswap_ulong(_tcstoul(value, NULL, 16)) >> 8;
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getf(ini_sect_t *sect, TCHAR *name, double *valp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL) {
		*valp = _tcstod(value, NULL);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getfr(ini_sect_t *sect, TCHAR *name, double *valp, double mn, double mx)
{
	double val;

	if(ini_getf(sect, name, &val) && (val >= mn) && (val <= mx)) {
		*valp = val;
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_getb(ini_sect_t *sect, TCHAR *name, int *valp)
{
	TCHAR *value;

	if((value = ini_get(sect, name)) != NULL)
	{
		if( (_tcscmp(value, _T("no")) == 0) ||
			(_tcscmp(value, _T("false")) == 0) )
		{
			*valp = 0;
			return 1;
		}
		if( (_tcscmp(value, _T("yes")) == 0) ||
			(_tcscmp(value, _T("true")) == 0) )
		{
			*valp = 1;
			return 1;
		}
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_copys(ini_sect_t *sect, TCHAR *name, TCHAR *buf, size_t bufsize)
{
	TCHAR *value;

	value = ini_get(sect, name);
	if((value != NULL) && (_tcslen(value) < bufsize)) {
		_tcscpy(buf, value);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_del(ini_sect_t *sect, TCHAR *name)
{
	ini_entry_t *ent;

	if((sect == NULL) || (name == NULL))
		return 0;

	/* find entry */
	if((ent = ini_ent_find(sect, name)) != NULL)
	{
		/* delete entry from section */
		DLIST_REMOVE(ent, sect->ent_head, sect->ent_tail, ent_fwd, ent_back);
		free(ent);
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

ini_sect_t *ini_sect_find(ini_data_t *ini, TCHAR *name)
{
	ini_sect_t *sect;

	for(sect = ini->sect_head; sect != NULL; sect = sect->sect_fwd) {
		if(_tcscmp(sect->name, name) == 0)
			return sect;
	}
	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

ini_sect_t *ini_sect_get(ini_data_t *ini, TCHAR *name, int create)
{
	ini_sect_t *sect;
	size_t size;

	/* find section */
	sect = ini_sect_find(ini, name);
	/* alloc section */
	if((sect == NULL) && create) {
		size = sizeof(ini_sect_t) + (_tcslen(name) + 1) * sizeof(TCHAR);
		if((sect = malloc(size)) == NULL)
			return NULL;
		memset(sect, 0, sizeof(ini_sect_t));
		sect->ini = ini;
		sect->name = (TCHAR*)((char*)sect + sizeof(ini_sect_t));
		_tcscpy(sect->name, name);
		DLIST_INSBACK(sect, ini->sect_head, ini->sect_tail, sect_fwd, sect_back);
	}
	return sect;
}

/* ---------------------------------------------------------------------------------------------- */

void ini_sect_delete(ini_sect_t *sect)
{
	ini_entry_t *entry, *next_entry;

	/* remove section */
	DLIST_REMOVE(sect, sect->ini->sect_head, sect->ini->sect_tail, sect_fwd, sect_back);

	/* free section */
	for(entry = sect->ent_head; entry != NULL; entry = next_entry)
	{
		next_entry = entry->ent_fwd;
		free(entry);
	}
	free(sect);
}

/* ---------------------------------------------------------------------------------------------- */

int ini_write(ini_data_t *ini, TCHAR *fname)
{
	FILE *fp;
	ini_sect_t *sect;
	ini_entry_t *ent;

	if((fp = _tfopen(fname, _T("wt"))) == NULL)
		return 0;

	for(sect = ini->sect_head; sect != NULL; sect = sect->sect_fwd)
	{
		_ftprintf(fp, _T("[%s]\n"), sect->name);
		for(ent = sect->ent_head; ent != NULL; ent = ent->ent_fwd)
			_ftprintf(fp, _T("%s = %s\n"), ent->name, ent->value);
		if(sect->sect_fwd != NULL)
			_ftprintf(fp, _T("\n"));
	}

	fclose(fp);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void ini_trim_spaces(TCHAR *buf)
{
	size_t len, n;
	TCHAR *p;

	len = _tcslen(buf);

	if((n = _tcsspn(buf, _T(" \t"))) != 0) {
		memmove(buf, buf + n, (len - n + 1) * sizeof(TCHAR));
		len -= n;
	}

	for(p = buf + len - 1; p >= buf; p--) {
		if((*p != ' ') && (*p != _T('\t')) && (*p != _T('\n')))
			break;
		*p = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int ini_parse_sect_hdr(TCHAR *buf)
{
	size_t len;

	len = _tcslen(buf);
	if((len >= 2) && (buf[0] == _T('[')) && (buf[len - 1] == _T(']'))) {
		memmove(buf, buf + 1, len * sizeof(TCHAR));
		buf[len - 2] = 0;
		ini_trim_spaces(buf);
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int ini_parse_entry(TCHAR *buf, TCHAR **pvalue)
{
	TCHAR *value;

	if((value = _tcschr(buf, _T('='))) == NULL)
		return 0;
	*(value++) = 0;

	ini_trim_spaces(buf);
	ini_trim_spaces(value);

	*pvalue = value;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int ini_read(ini_data_t *ini, TCHAR *fname)
{
	FILE *fp;
	TCHAR *value;
	ini_sect_t *cur_sect = NULL;
	int success = 0;

	ini->sect_head = NULL;
	ini->sect_tail = NULL;

	if((fp = _tfopen(fname, _T("rt"))) != NULL)
	{
		success = 1;
		while(_fgetts(ini->buf, INI_BUFSIZE, fp))
		{
			ini_trim_spaces(ini->buf);

			/* check for comment */
			if(ini->buf[0] == _T(';'))
				continue;

			/* check for section */
			if(ini_parse_sect_hdr(ini->buf))
			{
				if((cur_sect = ini_sect_get(ini, ini->buf, 1)) == NULL)
				{
					success = 0;
					break;
				}
			}

			/* check for entry */
			else if((cur_sect != NULL) && ini_parse_entry(ini->buf, &value))
			{
				if(!ini_set(cur_sect, ini->buf, value))
				{
					success = 0;
					break;
				}
			}

		}
		fclose(fp);
	}

	/* cleanup */
	if(!success) {
		ini_cleanup(ini);
	}

	return success;
}

/* ---------------------------------------------------------------------------------------------- */

ini_data_t *ini_alloc()
{
	ini_data_t *ini;
	
	if( (ini = calloc(1, sizeof(ini_data_t))) != NULL )
	{
		ini->buf = malloc((INI_BUFSIZE + 1) * sizeof(TCHAR));

		if(ini->buf != NULL)
		{
			ini->buf[INI_BUFSIZE] = 0;
			return ini;
		}

		free(ini->buf);
		free(ini);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

void ini_cleanup(ini_data_t *ini)
{
	ini_sect_t *sect, *next_sect;
	ini_entry_t *entry, *next_entry;

	for(sect = ini->sect_head; sect != NULL; sect = next_sect)
	{
		for(entry = sect->ent_head; entry != NULL; entry = next_entry)
		{
			next_entry = entry->ent_fwd;
			free(entry);
		}
		next_sect = sect->sect_fwd;
		free(sect);
	}

	free(ini->buf);
	free(ini);
}

/* ---------------------------------------------------------------------------------------------- */

int ini_file_load(ini_file_t *inif, TCHAR *fname)
{
	DWORD res;

	inif->ini = ini_alloc();
	inif->fn = malloc(MAX_PATH * sizeof(TCHAR));

	if( (inif->ini != NULL) && (inif->fn != NULL) )
	{
		if(get_local_path(inif->fn, MAX_PATH, fname))
		{
			res = GetFileAttributes(inif->fn);

			if(res == INVALID_FILE_ATTRIBUTES)
			{
				if(GetLastError() == ERROR_FILE_NOT_FOUND)
					return 1;
			}
			else if( ! (res & FILE_ATTRIBUTE_DIRECTORY) )
			{
				if(ini_read(inif->ini, inif->fn))
					return 1;
			}
		}
	}

	free(inif->fn);
	ini_cleanup(inif->ini);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void ini_file_save(ini_file_t *inif)
{
	ini_write(inif->ini, inif->fn);
}

/* ---------------------------------------------------------------------------------------------- */

void ini_file_cleanup(ini_file_t *inif)
{
	ini_cleanup(inif->ini);
	free(inif->fn);
}

/* ---------------------------------------------------------------------------------------------- */
