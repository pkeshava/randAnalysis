/*
 * This code has been heavily modified by the following people:
 *
 *      Landon Curt Noll
 *      Tom Gilgan
 *      Riccardo Paccagnella
 *
 * See the README.md and the initial comment in sts.c for more information.
 *
 * WE (THOSE LISTED ABOVE WHO HEAVILY MODIFIED THIS CODE) DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL WE (THOSE LISTED ABOVE
 * WHO HEAVILY MODIFIED THIS CODE) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * Share and enjoy! :-)
 */

#include <stdio.h>
#include <math.h>
#include "cephes.h"
#include "debug.h"

extern long int debuglevel;	// -v lvl: defines the level of verbosity for debugging

const double MACHEP = (double) 1.11022302462515654042363e-16;	// 2**-53
const double MAXLOG = (double) 7.0978271289338399673222e2;	// ln(2**1024*(1-MACHEP))
const double MAXNUM = (double) 1.79769313486231570814527e308;	// 2**1024*(1-MACHEP)

// Use C defined value of PI if available
#if defined(M_PI)
#   define PI (M_PI)
#else
// static const double PI = (double) 3.14159265358979323846264;
#endif

// Use C defined value of sqrt(2) if available
#if defined(M_SQRT2)
#   define SQRT2 (M_SQRT2)
#else
static const double SQRT2 = (double) 1.41421356237309504880;
#endif

static const double big = 4.503599627370496e15;
static const double biginv = 2.22044604925031308085e-16;

#if !defined(HAVE_LGAMMA)
static int sgngam = 0;

static double cephes_p1evl(double x, double *coef, int N);
static double cephes_polevl(double x, double *coef, int N);
#endif /* HAVE_LGAMMA */

double
cephes_igamc(double a, double x)
{
	double ans;
	double ax;
	double c;
	double yc;
	double r;
	double t;
	double y;
	double z;
	double pk;
	double pkm1;
	double pkm2;
	double qk;
	double qkm1;
	double qkm2;

	if ((x <= 0) || (a <= 0)) {
		return (1.0);
	}

	if ((x < 1.0) || (x < a)) {
		return (1.e0 - cephes_igam(a, x));
	}

	ax = a * log(x) - x - cephes_lgam(a);

	if (ax < -MAXLOG) {
		dbg(DBG_VVHIGH, "igamc: UNDERFLOW\n");
		return 0.0;
	}
	ax = exp(ax);

	/*
	 * continued fraction
	 */
	y = 1.0 - a;
	z = x + y + 1.0;
	c = 0.0;
	pkm2 = 1.0;
	qkm2 = x;
	pkm1 = x + 1.0;
	qkm1 = z * x;
	ans = pkm1 / qkm1;

	do {
		c += 1.0;
		y += 1.0;
		z += 2.0;
		yc = y * c;
		pk = pkm1 * z - pkm2 * yc;
		qk = qkm1 * z - qkm2 * yc;
		if (qk != 0) {
			r = pk / qk;
			t = fabs((ans - r) / r);
			ans = r;
		} else {
			t = 1.0;
		}
		pkm2 = pkm1;
		pkm1 = pk;
		qkm2 = qkm1;
		qkm1 = qk;
		if (fabs(pk) > big) {
			pkm2 *= biginv;
			pkm1 *= biginv;
			qkm2 *= biginv;
			qkm1 *= biginv;
		}
	} while (t > MACHEP);

	return ans * ax;
}

double
cephes_igam(double a, double x)
{
	double ans;
	double ax;
	double c;
	double r;

	if ((x <= 0) || (a <= 0)) {
		return 0.0;
	}

	if ((x > 1.0) && (x > a)) {
		return 1.e0 - cephes_igamc(a, x);
	}

	/*
	 * Compute x**a * exp(-x) / gamma(a)
	 */
	ax = a * log(x) - x - cephes_lgam(a);
	if (ax < -MAXLOG) {
		dbg(DBG_VVHIGH, "igam: UNDERFLOW\n");
		return 0.0;
	}
	ax = exp(ax);

	/*
	 * power series
	 */
	r = a;
	c = 1.0;
	ans = 1.0;

	do {
		r += 1.0;
		c *= x / r;
		ans += c;
	} while (c / ans > MACHEP);

	return ans * ax / a;
}


#if !defined(HAVE_LGAMMA)
/*
 * A[]: Stirling's formula expansion of log gamma B[], C[]: log gamma function between 2 and 3
 */
static unsigned short A[] = {
	0x6661, 0x2733, 0x9850, 0x3f4a,
	0xe943, 0xb580, 0x7fbd, 0xbf43,
	0x5ebb, 0x20dc, 0x019f, 0x3f4a,
	0xa5a1, 0x16b0, 0xc16c, 0xbf66,
	0x554b, 0x5555, 0x5555, 0x3fb5
};
static unsigned short B[] = {
	0x6761, 0x8ff3, 0x8901, 0xc095,
	0xb93e, 0x355b, 0xf234, 0xc0e2,
	0x89e5, 0xf890, 0x3d73, 0xc114,
	0xdb51, 0xf994, 0xbc82, 0xc131,
	0xf20b, 0x0219, 0x4589, 0xc13a,
	0x055e, 0x5418, 0x0c67, 0xc12a
};
static unsigned short C[] = {
	/*
	 * 0x0000,0x0000,0x0000,0x3ff0,
	 */
	0x12b2, 0x1cf3, 0xfd0d, 0xc075,
	0xd757, 0x7b89, 0xaa0d, 0xc0d0,
	0x4c9b, 0xb974, 0xeb84, 0xc10a,
	0x0043, 0x7195, 0x6286, 0xc131,
	0xf34c, 0x892f, 0x5255, 0xc143,
	0xe14a, 0x6a11, 0xce4b, 0xc13e
};

#   define MAXLGM 2.556348e305

/*
 * Logarithm of gamma function
 */
double
cephes_lgam(double x)
{
	double p;
	double q;
	double u;
	double w;
	double z;
	int i;

	sgngam = 1;

	if (x < -34.0) {
		q = -x;
		w = cephes_lgam(q);	/* note this modifies sgngam! */
		p = floor(q);
		if (p == q) {
			dbg(DBG_VHIGH, "lgam: #1 OVERFLOW\n");

			return sgngam * MAXNUM;
		}
		i = (int) p;
		if ((i & 1) == 0) {
			sgngam = -1;
		} else {
			sgngam = 1;
		}
		z = q - p;
		if (z > 0.5) {
			p += 1.0;
			z = p - q;
		}
		z = q * sin(PI * z);
		if (z == 0.0) {
			dbg(DBG_VHIGH, "lgam: #2 OVERFLOW\n");

			return sgngam * MAXNUM;
		}
		/*
		 * z = log(PI) - log( z ) - w;
		 */
		z = log(PI) - log(z) - w;
		return z;
	}

	if (x < 13.0) {
		z = 1.0;
		p = 0.0;
		u = x;
		while (u >= 3.0) {
			p -= 1.0;
			u = x + p;
			z *= u;
		}
		while (u < 2.0) {
			if (u == 0.0) {
				dbg(DBG_VHIGH, "lgam: #3 OVERFLOW\n");

				return sgngam * MAXNUM;
			}
			z /= u;
			p += 1.0;
			u = x + p;
		}
		if (z < 0.0) {
			sgngam = -1;
			z = -z;
		} else {
			sgngam = 1;
		}
		if (u == 2.0) {
			return (log(z));
		}
		p -= 2.0;
		x = x + p;
		p = x * cephes_polevl(x, (double *) B, 5) / cephes_p1evl(x, (double *) C, 6);

		return log(z) + p;
	}

	if (x > MAXLGM) {
		dbg(DBG_VHIGH, "lgam: #2 OVERFLOW\n");

		return sgngam * MAXNUM;
	}

	q = (x - 0.5) * log(x) - x + log(sqrt(2 * PI));
	if (x > 1.0e8) {
		return q;
	}

	p = 1.0 / (x * x);
	if (x >= 1000.0) {
		q += ((7.9365079365079365079365e-4 * p - 2.7777777777777777777778e-3) * p + 0.0833333333333333333333) / x;
	} else {
		q += cephes_polevl(p, (double *) A, 4) / x;
	}

	return q;
}

static double
cephes_polevl(double x, double *coef, int N)
{
	double ans;
	int i;
	double *p;

	p = coef;
	ans = *p++;
	i = N;

	do {
		ans = ans * x + *p++;
	} while (--i);

	return ans;
}

static double
cephes_p1evl(double x, double *coef, int N)
{
	double ans;
	double *p;
	int i;

	p = coef;
	ans = x + *p++;
	i = N - 1;

	do {
		ans = ans * x + *p++;
	} while (--i);

	return ans;
}
#endif /* !HAVE_LGAMMA */

#if 0
static const double rel_error = 1E-12;

double
cephes_erf(double x)
{
	static const double two_sqrtpi = 1.128379167095512574;
	double sum = x;
	double term = x;
	double xsqr = x * x;
	int j = 1;

	if (fabs(x) > 2.2) {
		return 1.0 - cephes_erfc(x);
	}

	do {
		term *= xsqr / j;
		sum -= term / (2 * j + 1);
		j++;
		term *= xsqr / j;
		sum += term / (2 * j + 1);
		j++;
	} while (fabs(term) / sum > rel_error);

	return two_sqrtpi * sum;
}

double
cephes_erfc(double x)
{
	static const double one_sqrtpi = 0.564189583547756287;
	double a = 1;
	double b = x;
	double c = x;
	double d = x * x + 0.5;
	double q1;
	double q2 = b / d;
	double n = 1.0;
	double t;

	if (fabs(x) < 2.2) {
		return 1.0 - cephes_erf(x);
	}
	if (x < 0) {
		return 2.0 - cephes_erfc(-x);
	}

	do {
		t = a * n + b * x;
		a = b;
		b = t;
		t = c * n + d * x;
		c = d;
		d = t;
		n += 0.5;
		q1 = q2;
		q2 = b / d;
	} while (fabs(q1 - q2) / q2 > rel_error);

	return one_sqrtpi * exp(-x * x) * q2;
}
#endif

double
cephes_normal(double x)
{
	double arg;
	double result;

	if (x > 0) {
		arg = x / SQRT2;
		result = 0.5 * (1 + erf(arg));
	} else {
		arg = -x / SQRT2;
		result = 0.5 * (1 - erf(arg));
	}

	return (result);
}
