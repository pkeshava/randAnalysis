/*
 * mkapertemplate - make an Aperiodic Template
 *
 * NOTE: This function was based on, in part, code from NIST Special Publication 800-22 Revision 1a:
 *
 *      http://csrc.nist.gov/groups/ST/toolkit/rng/documents/SP800-22rev1a.pdf
 *
 * In particular see section F.2 (page F-4) of the document revised April 2010.
 *
 * ***
 *
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
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#   include <stdint.h>
#else				// __STDC_VERSION__ >= 199901L
#   ifndef int64_t
typedef unsigned long long int64_t;	/* 64-bit unsigned word */
#   endif
#endif				// __STDC_VERSION__ >= 199901L

#include "../src/utils/debug.h"

#define B (8*sizeof(unsigned long long))

int debuglevel = 0;
char *program = NULL;
const char *const version = "mkapertemplate-2.1.2-2.cisco";
static const char *const usage = "bits template dataInfo";
static void displayBits(FILE * fp, unsigned int *A, unsigned long long value, long int count, long int *nonPeriodic);

int
main(int argc, char *argv[])
{
	char *template;		// filename of template
	FILE *fp1;		// open stream for template
	char *dataInfo;		// filenane of dataInfo
	FILE *fp2;		// open stream for dataInfo
	long int M;		// bit size of template
	long int count;
	unsigned long long num;
	long int nonPeriodic;
	unsigned int *A;
	unsigned long long i;

	/*
	 * parse args
	 */
	program = argv[0];
	if (argc != 4) {
		fprintf(stderr, "%s: expected 4 arguments, found only %d\n", program,argc);
		fprintf(stderr, "%s: %s\n", program, usage);
	}
	errno = 0;
	M = strtol(argv[1], NULL, 0);
	if (errno != 0) {
		errp(2, __func__, "error in parsing bits argument: %s", argv[1]);
	}
	template = argv[2];
	dataInfo = argv[3];
	if (M < 1) {
		err(3, __func__, "bits argument: %ld must be > 1", M);
	}
	if (M >= B) {
		err(3, __func__, "bits argument: %ld must be < %ld", M, B);
	}

	/*
	 * allocation of bit as byte array
	 */
	A = calloc(B, sizeof(unsigned int));
	if (A == NULL) {
		errp(1, __func__, "cannot calloc %ld unsigned int of %ld bytes each", B, sizeof(unsigned int));
	}

	/*
	 * open files for output
	 */
	fp1 = fopen(template, "w");
	if (fp1 == NULL) {
		errp(2, __func__, "cannot open for writing, %s", template);
	}
	fp2 = fopen(dataInfo, "a");
	if (fp2 == NULL) {
		errp(2, __func__, "cannot open for appending, %s", dataInfo);
	}

	/*
	 * setup to generate templates
	 */
	/*
	 * NOTE: Mathematical expression code rewrite, old code commented out below:
	 *
	 * num = pow(2, M);
	 * count = log(num) / log(2);
	 */
	num = (unsigned long long) 1 << M;
	count = M;
	nonPeriodic = 0;

	/*
	 * form non-periodic bits
	 */
	for (i = 1; i < num; i++) {
		displayBits(fp1, A, i, count, &nonPeriodic);
	}

	/*
	 * report
	 */
	fprintf(fp2, "M = %ld\n", M);
	fprintf(fp2, "# of nonperiodic templates = %ld\n", nonPeriodic);
	fprintf(fp2, "# of all possible templates = %lld\n", num);
	fprintf(fp2, "{# nonperiodic}/{# templates} = %f\n", (double) nonPeriodic / num);
	fprintf(fp2, "==========================================================\n");

	/*
	 * cleanup
	 */
	fclose(fp1);
	fclose(fp2);
	free(A);
	A = NULL;
	exit(0);
}

static void
displayBits(FILE * fp, unsigned int *A, unsigned long long value, long int count, long int *nonPeriodic)
{
	unsigned long long displayMask;
	int match = 0;
	int c;
	int i;

	displayMask = (unsigned long long) 1 << (B - 1);
	for (i = 0; i < B; i++)
		A[i] = 0;
	for (c = 1; c <= B; c++) {
		if (value & displayMask)
			A[c - 1] = 1;
		else
			A[c - 1] = 0;
		value <<= (unsigned long long) 1;
	}
	for (i = 1; i < count; i++) {
		match = 1;
		if ((A[B - count] != A[B - 1]) && ((A[B - count] != A[B - 2]) || (A[B - count + 1] != A[B - 1]))) {
			for (c = B - count; c <= (B - 1) - i; c++) {
				if (A[c] != A[c + i]) {
					match = 0;
					break;
				}
			}
		}
		if (match) {
			/*
			 * printf("\nPERIODIC TEMPLATE: SHIFT = %d\n",i);
			 */
			break;
		}
	}
	if (!match) {
		for (c = B - count; c < (B - 1); c++)
			fprintf(fp, "%u ", A[c]);
		fprintf(fp, "%u\n", A[B - 1]);
		(*nonPeriodic)++;
	}
	return;
}


/*
 * err - issue a fatal error message and exit
 *
 * given:
 *      exitcode        value to exit with
 *      name            name of function issuing the warning
 *      fmt             format of the warning
 *      ...             optional format args
 *
 * This function does not return.
 *
 * Example:
 *
 *      err(99, __func__, "bad foobar: %s", message);
 */
void
err(int exitcode, char const *name, char const *fmt, ...)
{
	va_list ap;		/* argument pointer */
	int ret;		/* return code holder */

	/*
	 * Start the var arg setup and fetch our first arg
	 */
	va_start(ap, fmt);

	/*
	 * Check preconditions (firewall)
	 */
	if (exitcode >= 256) {
		warn(__func__, "called with exitcode >= 256: %d", exitcode);
		exitcode = FORCED_EXIT;
		warn(__func__, "forcing exit code: %d", exitcode);
	}
	if (exitcode < 0) {
		warn(__func__, "called with exitcode < 0: %d", exitcode);
		exitcode = FORCED_EXIT;
		warn(__func__, "forcing exit code: %d", exitcode);
	}
	if (name == NULL) {
		warn(__func__, "called with NULL name");
		name = "((NULL name))";
	}
	if (fmt == NULL) {
		warn(__func__, "called with NULL fmt");
		fmt = "((NULL fmt))";
	}

	/*
	 * Issue the fatal error
	 */
	fprintf(stderr, "FATAL: %s: ", name);
	ret = vfprintf(stderr, fmt, ap);
	if (ret <= 0) {
		fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
	}
	fputc('\n', stderr);

	/*
	 * Clean up stdarg stuff
	 */
	va_end(ap);

	/*
	 * Terminate
	 */
	exit(exitcode);
}


/*
 * errp - issue a fatal error message, errno string and exit
 *
 * given:
 *      exitcode        value to exit with
 *      name            name of function issuing the warning
 *      fmt             format of the warning
 *      ...             optional format args
 *
 * This function does not return.  Unlike err() this function
 * also prints an errno message.
 *
 * Example:
 *
 *      errp(99, __func__, "I/O failure: %s", message);
 */
void
errp(int exitcode, char const *name, char const *fmt, ...)
{
	va_list ap;		/* argument pointer */
	int ret;		/* return code holder */
	int saved_errno;	/* errno at function start */

	/*
	 * Start the var arg setup and fetch our first arg
	 */
	saved_errno = errno;
	va_start(ap, fmt);

	/*
	 * Check preconditions (firewall)
	 */
	if (exitcode >= 256) {
		warn(__func__, "called with exitcode >= 256: %d", exitcode);
		exitcode = FORCED_EXIT;
		warn(__func__, "forcing exit code: %d", exitcode);
	}
	if (exitcode < 0) {
		warn(__func__, "called with exitcode < 0: %d", exitcode);
		exitcode = FORCED_EXIT;
		warn(__func__, "forcing exit code: %d", exitcode);
	}
	if (name == NULL) {
		warn(__func__, "called with NULL name");
		name = "((NULL name))";
	}
	if (fmt == NULL) {
		warn(__func__, "called with NULL fmt");
		fmt = "((NULL fmt))";
	}

	/*
	 * Issue the fatal error
	 */
	fprintf(stderr, "FATAL: %s: ", name);
	ret = vfprintf(stderr, fmt, ap);
	if (ret <= 0) {
		fprintf(stderr, "[%s vfprintf returned error: %d]", __func__, ret);
	}
	fputc('\n', stderr);
	fprintf(stderr, "errno[%d]: %s\n", saved_errno, strerror(saved_errno));

	/*
	 * Clean up stdarg stuff
	 */
	va_end(ap);

	/*
	 * Terminate
	 */
	exit(exitcode);
}
