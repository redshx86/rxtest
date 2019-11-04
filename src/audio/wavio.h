/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>

/* ---------------------------------------------------------------------------------------------- */

#define WAV_RIFF_ID		MAKEFOURCC('R','I','F','F')
#define WAV_WAVE_ID		MAKEFOURCC('W','A','V','E')
#define WAV_FMT_ID		MAKEFOURCC('f','m','t',' ')
#define WAV_DATA_ID		MAKEFOURCC('d','a','t','a')

/* ---------------------------------------------------------------------------------------------- */

typedef struct wav_io_state
{
	HANDLE fh;

	PCMWAVEFORMAT fmt;

	DWORD data_offset;
	DWORD data_size;
	DWORD file_size;

	int is_sync;
}
wav_io_state_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct wav_file_hdr
{
	DWORD riff_id;
	DWORD file_size;
	DWORD wave_id;
}
wav_file_hdr_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct wav_chunk_hdr
{
	DWORD chunk_id;
	DWORD chunk_size;
}
wav_chunk_hdr_t;

/* ---------------------------------------------------------------------------------------------- */

typedef enum wav_io_error {
	WAV_IO_OK,
	WAV_IO_ERR_BADPATH,
	WAV_IO_ERR_NOTFOUND,
	WAV_IO_ERR_ACCESS,
	WAV_IO_ERR_CANTCREATE,
	WAV_IO_ERR_CANTOPEN,
	WAV_IO_ERR_NOTWAV,
	WAV_IO_ERR_TRUNC,
	WAV_IO_ERR_BADWAV,
	WAV_IO_ERR_NOTPCM,
	WAV_IO_ERR_BADNCH,
	WAV_IO_ERR_BADRATE,
	WAV_IO_ERR_BADBPS,
	WAV_IO_ERR_BADFMT,
	WAV_IO_ERR_NODATA,
	WAV_IO_ERR_SEEK,
	WAV_IO_ERR_READ,
	WAV_IO_ERR_WRITE,
} wav_io_error_t;

/* ---------------------------------------------------------------------------------------------- */

int wav_write_open(wav_io_state_t *wi, TCHAR *fname, unsigned int fs,
				   unsigned int nch, unsigned int bps, wav_io_error_t *error);
void wav_write_close(wav_io_state_t *wi);
int wav_write_data(wav_io_state_t *wi, void *buf, size_t size);
int wav_write_sync(wav_io_state_t *wi);

/* ---------------------------------------------------------------------------------------------- */

int wav_read_open(wav_io_state_t *wi, TCHAR *fname, unsigned int *pfs, unsigned int *pnch,
				  unsigned int *pbps, size_t *pnumsamples, wav_io_error_t *error);
void wav_read_close(wav_io_state_t *wi);
int wav_read_data(wav_io_state_t *wi, void *buf, size_t size);
int wav_read_rewind(wav_io_state_t *wi);

/* ---------------------------------------------------------------------------------------------- */

TCHAR *wav_io_errmsg(wav_io_error_t error);

/* ---------------------------------------------------------------------------------------------- */
