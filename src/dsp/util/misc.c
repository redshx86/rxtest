/* ---------------------------------------------------------------------------------------------- */

#include "misc.h"

/* ---------------------------------------------------------------------------------------------- */

int factor(unsigned int *buf, unsigned int n)
{
	unsigned int m;
	unsigned int sr;
	int cnt;

	m = 2;
	cnt = 0;

	sr = (unsigned int)
		(floor(sqrt(n)) + 0.5);

	do {
		while(n % m != 0) {
			if(m == 2) m++;
			else m += 2;
			if(m > sr)
				m = n;
		}
		buf[cnt++] = m;
		n /= m;
	} while(n > 1);

	return cnt;
}

/* ---------------------------------------------------------------------------------------------- */

unsigned int gcd(unsigned int x, unsigned int y)
{
	unsigned int temp;

	while(y != 0)
	{
		temp = y;
		y = x % y;
		x = temp;
	}

	return x;
}

/* ---------------------------------------------------------------------------------------------- */

unsigned int lcm(unsigned int x, unsigned int y)
{
	unsigned __int64 temp;

	temp = (unsigned __int64)x *
		(unsigned __int64)y;

	return (unsigned int)(temp / gcd(x, y));
}

/* ---------------------------------------------------------------------------------------------- */

size_t convf(float *out, float *a, float *b, size_t len_a, size_t len_b)
{
	size_t i, j, len;

	len = len_a + len_b - 1;

	memset(out, 0, len * sizeof(float));
	for(i = 0; i < len_a; i++) {
		for(j = 0; j < len_b; j++)
			out[i + j] += a[i] * b[j];
	}

	return len;
}

/* ---------------------------------------------------------------------------------------------- */

double sumf(float *buf, size_t count)
{
	size_t i;
	double accum;

	accum = 0;
	for(i = 0; i < count; i++)
		accum += buf[i];
	return accum;
}

/* ---------------------------------------------------------------------------------------------- */

double sumsqrf(float *buf, size_t count)
{
	size_t i;
	double accum;

	accum = 0;
	for(i = 0; i < count; i++)
		accum += buf[i] * buf[i];
	return accum;
}

/* ---------------------------------------------------------------------------------------------- */

double meanf(float *buf, size_t count)
{
	double x;

	x = sumf(buf, count) / (double)count;
	return x;
}

/* ---------------------------------------------------------------------------------------------- */

double meansqrf(float *buf, size_t count)
{
	double x;

	x = sumsqrf(buf, count) / (double)count;
	return x;
}

/* ---------------------------------------------------------------------------------------------- */

double rmsf(float *buf, size_t count)
{
	double x;

	x = sqrt(sumsqrf(buf, count) / (double)count);
	return x;
}

/* ---------------------------------------------------------------------------------------------- */
