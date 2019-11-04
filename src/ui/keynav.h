/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

/* ---------------------------------------------------------------------------------------------- */

typedef enum keynav_mode {

	KEYNAV_MODE_OFF,
	KEYNAV_MODE_FLAT,
	KEYNAV_MODE_TREE,

	KEYNAV_MODE_COUNT

} keynav_mode_t;

/* ---------------------------------------------------------------------------------------------- */

typedef enum keynav_wndtype {

	KEYNAV_WNDTYPE_OTHER,

	KEYNAV_WNDTYPE_EDIT,
	KEYNAV_WNDTYPE_COMBOBOX,

} keynav_wndtype_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct keynav_wndentry {

	HWND hwnd;
	HWND hwndsub;

	keynav_wndtype_t type;

} keynav_wndentry_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct keynav_ctx {

	HINSTANCE h_inst;

	int is_enabled;
	int is_confirm_enabled;

	HWND hwnd;

	keynav_mode_t mode;

	size_t we_cap;
	size_t we_cnt;
	size_t we_cur_idx;
	keynav_wndentry_t *we_buf;

} keynav_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

int keynav_msghandle(keynav_ctx_t *kn, MSG *msg);

void keynav_init(keynav_ctx_t *kn, HINSTANCE h_inst);
void keynav_cleanup(keynav_ctx_t *kn);

int keynav_setcurwnd(keynav_ctx_t *kn, HWND hwnd, int is_confirm_enabled);
int keynav_unsetcurwnd(keynav_ctx_t *kn, HWND hwnd);

int keynav_update(keynav_ctx_t *kn, HWND hwnd);

/* ---------------------------------------------------------------------------------------------- */
