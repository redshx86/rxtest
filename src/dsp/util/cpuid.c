/* ---------------------------------------------------------------------------------------------- */

#include "cpuid.h"

/* ---------------------------------------------------------------------------------------------- */

unsigned int is_mmx_enabled  = 0;
unsigned int is_sse_enabled  = 0;
unsigned int is_sse2_enabled = 0;

/* ---------------------------------------------------------------------------------------------- */

static void win_version(unsigned int *nt, unsigned int *build,
						unsigned int *minor, unsigned int *major)
{
	DWORD version;

	version = GetVersion();

	*nt = (version & 0x80000000) ? 0 : 1;
	*build = (version >> 16) & 0x7fff;
	*minor = (version >> 8) & 0xff;
	*major = version & 0xff;
}

/* ---------------------------------------------------------------------------------------------- */

void check_cpu_features()
{
	unsigned int buf[4];
	unsigned int nt, build, minor, major;

	win_version(&nt, &build, &minor, &major);

	/* Enable SSE code on NT 5.x and 6.x */
	if( nt && (build >= 2195) && (build <= 9600) )
	{
		if(_is_cpuid_supported())
		{
			_cpuid(buf, 1);

			is_mmx_enabled  = (buf[2] >> 23) & 1;
			is_sse_enabled  = (buf[2] >> 25) & 1;
			is_sse2_enabled = (buf[2] >> 26) & 1;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */
