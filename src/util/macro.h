/* ---------------------------------------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------------------------------------- */

#define ARRAY_SIZE(a)			(sizeof(a) / sizeof(*(a)))

#define INRANGE(x,a,b)			( ((x) >= (a)) && ((x) <= (b)) )
#define INRANGE_MINMAX(x,ab)	( ((x) >= (ab##_MIN)) && ((x) <= (ab##_MAX)) )

#define ISPOWEROFTWO(x)			( ((x) & ((x) - 1)) == 0 )

#define FLOOR2POT(x,a)			( (x) & ~((a) - 1) )
#define CEIL2POT(x,a)			( ((x) + (a) - 1) & ~((a) - 1) )

#define SWAP(a,b,temp)			{ (temp) = (a); (a) = (b); (b) = (temp); }

#define MSECS2NSAMP(x,fs)		(size_t)( (double)(fs) * ((x) * 0.001) + 0.5 )

/* ---------------------------------------------------------------------------------------------- */
