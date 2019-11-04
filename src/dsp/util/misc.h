/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <math.h>
#include <string.h>

/* ---------------------------------------------------------------------------------------------- */

int factor(unsigned int *buf, unsigned int n);

unsigned int gcd(unsigned int x, unsigned int y);
unsigned int lcm(unsigned int x, unsigned int y);

size_t convf(float *out, float *a, float *b, size_t len_a, size_t len_b);

double sumf(float *buf, size_t count);
double sumsqrf(float *buf, size_t count);
double meanf(float *buf, size_t count);
double meansqrf(float *buf, size_t count);
double rmsf(float *buf, size_t count);

/* ---------------------------------------------------------------------------------------------- */
