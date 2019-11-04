/* ---------------------------------------------------------------------------------------------- */

#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "list.h"
#include "numparse.h"
#include "pathutil.h"

/* ---------------------------------------------------------------------------------------------- */

#define INI_BUFSIZE		4095

/* ---------------------------------------------------------------------------------------------- */

struct ini_sect;

typedef struct ini_entry {
	struct ini_sect *sect;
	struct ini_entry *ent_fwd;
	struct ini_entry *ent_back;
	TCHAR *name;
	TCHAR *value;
	size_t valuebufsize;
} ini_entry_t;

/* ---------------------------------------------------------------------------------------------- */

struct ini_data;

typedef struct ini_sect {
	struct ini_data *ini;
	struct ini_sect *sect_fwd;
	struct ini_sect *sect_back;
	ini_entry_t *ent_head;
	ini_entry_t *ent_tail;
	TCHAR *name;
} ini_sect_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct ini_data {
	TCHAR *buf;
	ini_sect_t *sect_head;
	ini_sect_t *sect_tail;
} ini_data_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct ini_file {
	TCHAR *fn;
	ini_data_t *ini;
} ini_file_t;

/* ---------------------------------------------------------------------------------------------- */

typedef enum ini_enum ini_enum_t;

ini_sect_t *ini_sect_get(ini_data_t *ini, TCHAR *name, int create);
void ini_sect_delete(ini_sect_t *sect);

int ini_set(ini_sect_t *sect, TCHAR *name, TCHAR *value);
int ini_seti(ini_sect_t *sect, TCHAR *name, int value);
int ini_setu(ini_sect_t *sect, TCHAR *name, unsigned int value);
int ini_setus(ini_sect_t *sect, TCHAR *name, size_t value);
int ini_setux(ini_sect_t *sect, TCHAR *name, unsigned int value);
int ini_sete(ini_sect_t *sect, TCHAR *name, ini_enum_t value);
int ini_setcr(ini_sect_t *sect, TCHAR *name, COLORREF cr);
int ini_setf(ini_sect_t *sect, TCHAR *name, int count, double value);
int ini_setb(ini_sect_t *sect, TCHAR *name, int value);

TCHAR *ini_get(ini_sect_t *sect, TCHAR *name);
int ini_geti(ini_sect_t *sect, TCHAR *name, int *valp);
int ini_getir(ini_sect_t *sect, TCHAR *name, int *valp, int mn, int mx);
int ini_getu(ini_sect_t *sect, TCHAR *name, unsigned int *valp);
int ini_getur(ini_sect_t *sect, TCHAR *name, unsigned int *valp,
			  unsigned int mn, unsigned int mx);
int ini_getus(ini_sect_t *sect, TCHAR *name, size_t *valp);
int ini_getusr(ini_sect_t *sect, TCHAR *name, size_t *valp, size_t mn, size_t mx);
int ini_getux(ini_sect_t *sect, TCHAR *name, unsigned int *valp);
int ini_getuxr(ini_sect_t *sect, TCHAR *name, unsigned int *valp,
			   unsigned int mn, unsigned int mx);
int ini_gete(ini_sect_t *sect, TCHAR *name, ini_enum_t *valp, ini_enum_t count);
int ini_getcr(ini_sect_t *sect, TCHAR *name, COLORREF *crp);
int ini_getf(ini_sect_t *sect, TCHAR *name, double *valp);
int ini_getfr(ini_sect_t *sect, TCHAR *name, double *valp, double mn, double mx);
int ini_getb(ini_sect_t *sect, TCHAR *name, int *valp);
int ini_copys(ini_sect_t *sect, TCHAR *name, TCHAR *buf, size_t bufsize);

int ini_del(ini_sect_t *sect, TCHAR *name);

/* ---------------------------------------------------------------------------------------------- */

ini_data_t *ini_alloc();
void ini_cleanup(ini_data_t *ini);

int ini_read(ini_data_t *ini, TCHAR *fname);
int ini_write(ini_data_t *ini, TCHAR *fname);

/* ---------------------------------------------------------------------------------------------- */

int ini_file_load(ini_file_t *inif, TCHAR *fname);
void ini_file_save(ini_file_t *inif);
void ini_file_cleanup(ini_file_t *inif);

/* ---------------------------------------------------------------------------------------------- */
