/* ---------------------------------------------------------------------------------------------- */

#include "strutil.h"

/* ---------------------------------------------------------------------------------------------- */

void wrapcpy(void *dst, size_t dstofs, size_t dstwrp,
			 void *src, size_t srcofs, size_t srcwrp,
			 size_t blocksize, size_t numblocks)
{
	char *dstp, *srcp;
	char *dstend, *srcend;
	size_t copysize, dstchunk, srcchunk;

	dstp = (char*)dst + dstofs * blocksize;
	srcp = (char*)src + srcofs * blocksize;

	dstend = (char*)dst + dstwrp * blocksize;
	srcend = (char*)src + srcwrp * blocksize;

	copysize = blocksize * numblocks;

	if(dstp + copysize > dstend)
	{
		dstchunk = dstend - dstp;

		if(srcp + dstchunk > srcend)
		{
			srcchunk = srcend - srcp;
			memcpy(dstp, srcp, srcchunk);
			dstp += srcchunk;
			srcp = src;
			dstchunk -= srcchunk;
		}

		memcpy(dstp, srcp, dstchunk);
		dstp = dst;
		srcp += dstchunk;
		copysize -= dstchunk;
	}

	if(srcp + copysize > srcend)
	{
		srcchunk = srcend - srcp;
		memcpy(dstp, srcp, srcchunk);
		dstp += srcchunk;
		srcp = src;
		copysize -= srcchunk;
	}

	memcpy(dstp, srcp, copysize);
}

/* ---------------------------------------------------------------------------------------------- */
