/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include "../dsp/complex.h"
#include "../util/iniparse.h"
#include "../ui/uicommon.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct buf_input {

	size_t capacity;
	size_t nsamples;
	cpxf_t *data;

	struct buf_input *next;

} input_buf_t;

/* ---------------------------------------------------------------------------------------------- */

struct input_module_desc;

typedef int (*instream_open_t)(void *ctx, HANDLE event, TCHAR *errbuf, size_t errbufsize);
typedef void (*instream_close_t)(void *ctx);
typedef int (*instream_start_t)(void *ctx, TCHAR *errbuf, size_t errbufsize);
typedef void (*instream_stop_t)(void *ctx);

typedef void (*instream_reset_buffers_t)(void *ctx);
typedef void (*instream_add_buffers_t)(void *ctx, input_buf_t *bufs, size_t count);
typedef input_buf_t* (*instream_remove_buffer_t)(void *ctx);
typedef int (*instream_check_error_t)(void *ctx);

typedef unsigned int (*instream_get_fs_t)(void *ctx);

typedef int (*instream_get_fc_t)(void *ctx, double *fcp);
typedef int (*instream_set_fc_t)(void *ctx, double *fcp);

typedef void* (*inmodule_init_t)(struct input_module_desc *desc, TCHAR *errbuf, size_t errbufsize);
typedef void (*inmodule_cleanup_t)(void *ctx);

typedef void (*inmodule_config_showdialog_t)(void*, uicommon_t*, HWND);

typedef void (*inmodule_config_load_t)(void *ctx, ini_data_t *ini);
typedef void (*inmodule_config_save_t)(void *ctx, ini_data_t *ini);

typedef void (*inmodule_cleanup_desc_t)(struct input_module_desc *desc);

/* ---------------------------------------------------------------------------------------------- */

typedef struct input_module_desc {

	/* inputs list pointers */
	unsigned int module_id;
	struct input_module_desc *module_next;
	struct input_module_desc *module_prev;

	void *moduledata;

	TCHAR *name;
	TCHAR *display_name;

	instream_open_t fn_open;
	instream_close_t fn_close;
	instream_start_t fn_start;
	instream_stop_t fn_stop;

	instream_reset_buffers_t fn_reset_bufs;
	instream_add_buffers_t fn_add_bufs;
	instream_remove_buffer_t fn_remove_buf;
	instream_check_error_t fn_check_err;

	instream_get_fs_t fn_get_fs;

	instream_get_fc_t fn_get_fc;
	instream_set_fc_t fn_set_fc;

	inmodule_init_t fn_init;
	inmodule_cleanup_t fn_cleanup;

	inmodule_config_showdialog_t fn_config_showdialog;
	inmodule_config_load_t fn_config_load;
	inmodule_config_save_t fn_config_save;

	inmodule_cleanup_desc_t fn_cleanup_desc;

} input_module_desc_t;

/* ---------------------------------------------------------------------------------------------- */
