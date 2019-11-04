/* ---------------------------------------------------------------------------------------------- */

#include "wavio.h"

/* ---------------------------------------------------------------------------------------------- */

static int wavio_setfilepointer(wav_io_state_t *wi, DWORD pos)
{
	LARGE_INTEGER distance;

	distance.LowPart = pos;
	distance.HighPart = 0;

	return (int)SetFilePointerEx(wi->fh, distance, NULL, FILE_BEGIN);
}

/* ---------------------------------------------------------------------------------------------- */

static int wav_write_headers(wav_io_state_t *wi)
{
	DWORD temp;
	wav_file_hdr_t filehdr;
	wav_chunk_hdr_t fmthdr, datahdr;
	int success = 1;

	/* Write RIFF header */
	filehdr.riff_id = WAV_RIFF_ID;
	filehdr.file_size = wi->file_size - 8;
	filehdr.wave_id = WAV_WAVE_ID;
	WriteFile(wi->fh, &filehdr, sizeof(filehdr), &temp, NULL);
	success = success && (temp == sizeof(filehdr));

	/* Write format header */
	fmthdr.chunk_id = WAV_FMT_ID;
	fmthdr.chunk_size = sizeof(PCMWAVEFORMAT);
	WriteFile(wi->fh, &fmthdr, sizeof(fmthdr), &temp, NULL);
	success = success && (temp == sizeof(fmthdr));

	/* Write format data */
	WriteFile(wi->fh, &(wi->fmt), sizeof(wi->fmt), &temp, NULL);
	success = success && (temp == sizeof(wi->fmt));

	/* Write data header */
	datahdr.chunk_id = WAV_DATA_ID;
	datahdr.chunk_size = wi->data_size;
	WriteFile(wi->fh, &datahdr, sizeof(datahdr), &temp, NULL);
	success = success && (temp == sizeof(datahdr));

	/* Flush file buffers */
	FlushFileBuffers(wi->fh);

	return success;
}

/* ---------------------------------------------------------------------------------------------- */

int wav_write_sync(wav_io_state_t *wi)
{
	int success;

	if(wi->is_sync)
		return 1;

	/* Update headers */
	success =
		wavio_setfilepointer(wi, 0) &&
		wav_write_headers(wi);

	/* Return file pointer to data */
	SetFilePointer(wi->fh, 0, NULL, FILE_END);

	wi->is_sync = 1;

	return success;
}

/* ---------------------------------------------------------------------------------------------- */

int wav_write_data(wav_io_state_t *wi, void *buf, size_t size)
{
	int success;
	DWORD temp;

	WriteFile(wi->fh, buf, (DWORD)size, &temp, NULL);
	success = (temp == (DWORD)size);

	wi->data_size += temp;
	wi->file_size += temp;

	wi->is_sync = 0;

	return success;
}

/* ---------------------------------------------------------------------------------------------- */

int wav_write_open(wav_io_state_t *wi, TCHAR *fname, unsigned int fs,
				   unsigned int nch, unsigned int bps, wav_io_error_t *error)
{
	DWORD err;

	/* create file */
	wi->fh = CreateFile(fname, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(wi->fh == INVALID_HANDLE_VALUE)
	{
		err = GetLastError();
		switch(err)
		{
		case ERROR_BAD_PATHNAME:
		case ERROR_PATH_NOT_FOUND:
			*error = WAV_IO_ERR_BADPATH;
			break;
		case ERROR_ACCESS_DENIED:
			*error = WAV_IO_ERR_ACCESS;
			break;
		default:
			*error = WAV_IO_ERR_CANTCREATE;
			break;
		}
	}
	else
	{
		/* fill format structure */
		wi->fmt.wf.wFormatTag = WAVE_FORMAT_PCM;
		wi->fmt.wf.nChannels = (WORD)nch;
		wi->fmt.wf.nSamplesPerSec = (DWORD)fs;
		wi->fmt.wf.nAvgBytesPerSec = (DWORD)(fs * nch * bps / 8);
		wi->fmt.wf.nBlockAlign = (WORD)(nch * bps / 8);
		wi->fmt.wBitsPerSample = (WORD)bps;

		/* calculate file size */
		wi->data_offset =
			sizeof(wav_file_hdr_t)	+	/* file header */
			sizeof(wav_chunk_hdr_t) +	/* format chunk header  */
			sizeof(PCMWAVEFORMAT) +		/* format data */
			sizeof(wav_chunk_hdr_t);	/* data chunk header  */
		wi->data_size = 0;
		wi->file_size = wi->data_offset + wi->data_size;

		wi->is_sync = 0;

		/* write initial headers */
		if(!wav_write_headers(wi)) {
			*error = WAV_IO_ERR_CANTCREATE;
		} else {
			*error = WAV_IO_OK;
			return 1;
		}

		CloseHandle(wi->fh);
		DeleteFile(fname);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void wav_write_close(wav_io_state_t *wi)
{
	wav_write_sync(wi);
	CloseHandle(wi->fh);
}

/* ---------------------------------------------------------------------------------------------- */

static int wav_find_chunk(wav_io_state_t *wi, DWORD chunk_id,
						  DWORD *pdata_offset, DWORD *pdata_size,
						  wav_io_error_t *error)
{
	wav_chunk_hdr_t chunk_hdr;
	DWORD temp, current_pos;

	current_pos = sizeof(wav_file_hdr_t);

	for( ; ; )
	{
		/* check address */
		if(current_pos >= wi->file_size) {
			/* required chunk not found */
			*error = WAV_IO_ERR_BADWAV;
			return 0;
		}

		/* goto chunk start address */
		if(!wavio_setfilepointer(wi, current_pos)) {
			/* seek error */
			*error = WAV_IO_ERR_SEEK;
			return 0;
		}

		/* check file size */
		if( (ULONGLONG)current_pos + (ULONGLONG)sizeof(chunk_hdr) >
			(ULONGLONG)(wi->file_size) )
		{
			/* truncated */
			*error = WAV_IO_ERR_TRUNC;
			return 0;
		}

		/* read chunk header */
		ReadFile(wi->fh, &chunk_hdr, sizeof(chunk_hdr), &temp, NULL);
		if(temp != sizeof(chunk_hdr)) {
			/* can't read chunk header */
			*error = WAV_IO_ERR_READ;
			return 0;
		}

		/* check chunk size */
		if( (ULONGLONG)current_pos + (ULONGLONG)sizeof(chunk_hdr) +
				(ULONGLONG)(chunk_hdr.chunk_size) > (ULONGLONG)(wi->file_size) )
		{
			/* chunk exceeds file size */
			*error = WAV_IO_ERR_TRUNC;
			return 0;
		}

		/* check current chunk id */
		if(chunk_hdr.chunk_id == chunk_id)
		{
			/* chunk found and valid */
			*pdata_offset = current_pos + sizeof(chunk_hdr);
			*pdata_size = chunk_hdr.chunk_size;
			*error = WAV_IO_OK;
			return 1;
		}

		/* goto next chunk */
		current_pos += sizeof(chunk_hdr);
		current_pos += ((chunk_hdr.chunk_size + 1) & ~1);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int wav_check_format(PCMWAVEFORMAT *fmt, wav_io_error_t *error)
{
	DWORD blockalign, bytespersec;

	/* check format, should be pcm */
	if(fmt->wf.wFormatTag != WAVE_FORMAT_PCM) {
		/* not pcm file error */
		*error = WAV_IO_ERR_NOTPCM;
		return 0;
	}

	/* check number of channels, generally 1 or 2 */
	if( (fmt->wf.nChannels != 1) && (fmt->wf.nChannels != 2) ) {
		/* invalid number of channels error */
		*error = WAV_IO_ERR_BADNCH;
		return 0;
	}

	/* check sample rate, should be nonzero */
	if(fmt->wf.nSamplesPerSec == 0) {
		/* bad sample rate error */
		*error = WAV_IO_ERR_BADRATE;
		return 0;
	}

	/* check bits per sample, generally 8 or 16 */
	if( (fmt->wBitsPerSample != 8) && (fmt->wBitsPerSample != 16) ) {
		*error = WAV_IO_ERR_BADBPS;
		return 0;
	}

	/* check block align, should be correct, or ignored if zero */
	blockalign = fmt->wf.nChannels * fmt->wBitsPerSample / 8;
	if( (fmt->wf.nBlockAlign != blockalign) && (fmt->wf.nBlockAlign != 0) )
	{
		/* invalid block align in header (something wrong) */
		*error = WAV_IO_ERR_BADFMT;
		return 0;
	}

	/* check average bytes per second, should be correct, or ignored if zero */
	bytespersec = fmt->wf.nSamplesPerSec * blockalign;
	if( (fmt->wf.nAvgBytesPerSec != bytespersec) && (fmt->wf.nAvgBytesPerSec != 0) ) {
		/* invalid bytes per second in header (something wrong) */
		*error = WAV_IO_ERR_BADFMT;
		return 0;
	}

	*error = WAV_IO_OK;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int wav_read_headers(wav_io_state_t *wi, DWORD file_size, wav_io_error_t *error)
{
	DWORD temp;
	DWORD hdr_file_size, fmt_offset, fmt_size, data_offset, data_size;
	wav_file_hdr_t filehdr;

	/* check file size */
	if(sizeof(filehdr) > file_size) {
		/* file too small */
		*error = WAV_IO_ERR_NOTWAV;
		return 0;
	}
	/* read file header */
	ReadFile(wi->fh, &filehdr, sizeof(filehdr), &temp, NULL);
	if(temp != sizeof(filehdr)) {
		/* can't read header */
		*error = WAV_IO_ERR_READ;
		return 0;
	}
	/* check signatures */
	if( (filehdr.riff_id != WAV_RIFF_ID) || (filehdr.wave_id != WAV_WAVE_ID) ) {
		/* no wav signatures */
		*error = WAV_IO_ERR_NOTWAV;
		return 0;
	}
	/* check file size */
	if(filehdr.file_size != 0)
	{
		/* file size from header */
		hdr_file_size = filehdr.file_size + 8;

		/* file truncation check */
		if(hdr_file_size > file_size) {
			/* file truncated */
			*error = WAV_IO_ERR_TRUNC;
			return 0;
		}

		/* file excess size check */
		if(hdr_file_size < file_size)
			file_size = hdr_file_size;
	}
	/* save correct file size */
	wi->file_size = file_size;

	/* find format chunk */
	if(!wav_find_chunk(wi, WAV_FMT_ID, &fmt_offset, &fmt_size, error))
		return 0;
	/* check format chunk size */
	if(fmt_size < sizeof(PCMWAVEFORMAT)) {
		/* format chunk too small */
		*error = WAV_IO_ERR_BADWAV;
		return 0;
	}
	/* read format chunk */
	ReadFile(wi->fh, &(wi->fmt), sizeof(wi->fmt), &temp, NULL);
	if(temp != sizeof(wi->fmt)) {
		/* can't read format chunk */
		*error = WAV_IO_ERR_READ;
		return 0;
	}
	/* check format */
	if(!wav_check_format(&(wi->fmt), error)) {
		/* invalid format */
		return 0;
	}

	/* find data chunk */
	if(!wav_find_chunk(wi, WAV_DATA_ID, &data_offset, &data_size, error))
		return 0;
	/* check data size */
	if(data_size == 0) {
		*error = WAV_IO_ERR_NODATA;
		return 0;
	}
	/* save data offset and size */
	wi->data_offset = data_offset;
	wi->data_size = data_size;

	/* headers ok */
	*error = WAV_IO_OK;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int wav_read_open(wav_io_state_t *wi, TCHAR *fname, unsigned int *pfs, unsigned int *pnch,
				  unsigned int *pbps, size_t *pnumsamples, wav_io_error_t *error)
{
	DWORD res;
	LARGE_INTEGER fsize;

	wi->fh = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if(wi->fh == INVALID_HANDLE_VALUE)
	{
		/* check open error */
		res = GetLastError();
		switch(res) {
			case ERROR_BAD_PATHNAME:
				*error = WAV_IO_ERR_BADPATH;
				break;
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				*error = WAV_IO_ERR_NOTFOUND;
				break;
			case ERROR_ACCESS_DENIED:
				*error = WAV_IO_ERR_ACCESS;
				break;
			default:
				*error = WAV_IO_ERR_CANTOPEN;
				break;
		}
	}
	else
	{
		/* get file size */
		if(!GetFileSizeEx(wi->fh, &fsize)) {
			/* can't get file size */
			*error = WAV_IO_ERR_READ;
		} else {
			/* check file size, should be <= 2^32-1 */
			if(fsize.HighPart != 0) {
				/* file too large */
				*error = WAV_IO_ERR_BADWAV;
			} else {
				/* read headers */
				if(wav_read_headers(wi, fsize.LowPart, error))
				{
					*pfs = (unsigned int)(wi->fmt.wf.nSamplesPerSec);
					*pnch = (unsigned int)(wi->fmt.wf.nChannels);
					*pbps = (unsigned int)(wi->fmt.wBitsPerSample);

					*pnumsamples = (size_t)(wi->data_size /
						(wi->fmt.wf.nChannels * wi->fmt.wBitsPerSample / 8));

					*error = WAV_IO_OK;

					return 1;
				}
			}
		}

		CloseHandle(wi->fh);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void wav_read_close(wav_io_state_t *wi)
{
	CloseHandle(wi->fh);
}

/* ---------------------------------------------------------------------------------------------- */

int wav_read_data(wav_io_state_t *wi, void *buf, size_t size)
{
	DWORD temp;

	ReadFile(wi->fh, buf, (DWORD)size, &temp, NULL);
	return (temp == (DWORD)size);
}

/* ---------------------------------------------------------------------------------------------- */

int wav_read_rewind(wav_io_state_t *wi)
{
	return wavio_setfilepointer(wi, wi->data_offset);
}

/* ---------------------------------------------------------------------------------------------- */

TCHAR *wav_io_errmsg(wav_io_error_t error)
{
	TCHAR *message;

	switch(error)
	{
	case WAV_IO_OK:
		message = _T("no error");
		break;
	case WAV_IO_ERR_BADPATH:
		message = _T("bad file path");
		break;
	case WAV_IO_ERR_NOTFOUND:
		message = _T("file not found");
		break;
	case WAV_IO_ERR_ACCESS:
		message = _T("access denied");
		break;
	case WAV_IO_ERR_CANTCREATE:
		message = _T("can't create file");
		break;
	case WAV_IO_ERR_CANTOPEN:
		message = _T("can't open file");
		break;
	case WAV_IO_ERR_NOTWAV:
		message = _T("not a WAV file");
		break;
	case WAV_IO_ERR_TRUNC:
		message = _T("file truncated");
		break;
	case WAV_IO_ERR_BADWAV:
		message = _T("bad WAV file");
		break;
	case WAV_IO_ERR_NOTPCM:
		message = _T("not PCM format");
		break;
	case WAV_IO_ERR_BADNCH:
		message = _T("bad number of channels");
		break;
	case WAV_IO_ERR_BADRATE:
		message = _T("bad sample rate");
		break;
	case WAV_IO_ERR_BADBPS:
		message = _T("bad bits per sample");
		break;
	case WAV_IO_ERR_BADFMT:
		message = _T("bad wave format");
		break;
	case WAV_IO_ERR_NODATA:
		message = _T("file is empty");
		break;
	case WAV_IO_ERR_SEEK:
		message = _T("can't seek file");
		break;
	case WAV_IO_ERR_READ:
		message = _T("can't read file");
		break;
	case WAV_IO_ERR_WRITE:
		message = _T("can't write file");
		break;
	default:
		message = _T("unknown error");
		break;
	}

	return message;
}

/* ---------------------------------------------------------------------------------------------- */
