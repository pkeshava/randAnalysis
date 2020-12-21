/*****************************************************************************
		      L O N G E S T   R U N S   T E S T
 *****************************************************************************/

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


// Exit codes: 110 thru 119

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "../utils/externs.h"
#include "../utils/cephes.h"
#include "../utils/utilities.h"
#include "../utils/debug.h"


/*
 * Private stats - stats.txt information for this test
 */
struct LongestRunOfOnes_private_stats {
	bool success;		// Success or failure of iteration test
	long int N;		// Number of independent M-bit blocks the bit stream is partitioned into
	long int M;		// Length of each block to be tested
	double chi2;		// Sum of chi^2 for each iteration
	int runs_table_index;	// Index in the runs_table[] being used

	/*
	 * When 0 < i < CLASS_COUNT_LONGEST_RUN, count[i] contains the number of times that the longest run of ones in an
	 * M-bit block is equal to (min_class + i). When i == 0, count[i] contains the number of times that the longest run
	 * of ones in an M-bit block is smaller or equal to (min_class). When i >= CLASS_COUNT_LONGEST_RUN,
	 * count[CLASS_COUNT_LONGEST_RUN] contains the number of times that the longest run of ones in an M-bit
	 * block is bigger or equal to (max_class).
	 */
	unsigned long count[CLASS_COUNT_LONGEST_RUN + 1];
};


/*
 * Static const variables declarations
 */
static const enum test test_num = TEST_LONGEST_RUN;	// This test number

/*
 * Theoretical probabilities of the longest run in a bit string
 *
 * This structure replaces the old if-then-else logic filled
 * with magic constants that once was in the iteration code.
 */
struct runs_table {
	const long int min_n;	// Minimum n from test table
	const long int M;	// Length of each block to be tested
	const int min_class;	// Minimum length to consider (0 < min_class)
	const int max_class;	// Maximum length to consider (min_class + CLASS_COUNT_LONGEST_RUN)
	const double pi_term[CLASS_COUNT_LONGEST_RUN + 1]; // Theoretical probabilities (see comment above)
};

/*
 * The runs_table[] constants were computing using calc:
 *
 *    http://www.isthe.com/chongo/tech/comp/calc/index.html
 *
 * via the calc script:
 *
 *      ../tools/runs_table.cal
 *
 * For a given block of length M, and where K = max_class - min_class:
 *
 *   For 0 < i <= min_class:
 *
 *      pi_term[0] is the probability that the length of the longest run of ones
 *      in a M-bit block will be <= min_class.
 *
 *   For min_class < i < max_class:
 *
 *      pi_term[i - min_class] is the probability that length of the the longest run of ones
 *      in a M-bit block will be = i.
 *
 *   For max_class <= i <= M:
 *
 *      pi_term[K] is the probability that length of the the longest run of ones
 *      in a M-bit block will be >= max_class.
 *
 * The runs_table[] array is sorted by min_n.  For n (bitcount, length of a single bit stream)
 * find last the table entry where n > runs_table->min_n and where runs_table->min_n != 0.
 *
 * This runs_table[] array is computed with parameters from SP800-22Rev1a section 2.4.2,
 * (the 1st table) and section 2.4.4 (the 1st and 2nd tables).
 *
 * The pi_term array for a given entry is computed from the non-commented constants
 * printed by runstbl(M, min_class, max_class) from the runs_table.cal calc script.
 *
 * There are 3 entries in the 1st table found in section 2.4.2.  We use this table
 * to produce the 3 entries of constants found in the runs_table[] array below.
 * The prob_*[] arrays of constants were obtained from the following runstbl() calls
 * to the runs_table.cal calc script:
 *
 *      runstbl(8, 1, 4)
 *      runstbl(128, 4, 9)
 *      runstbl(10000, 10, 16)
 *
 *      NOTE: For large values of M (such as 10000), this script takes
 *            a very long time to run (as in around 8 CPU days)!
 *
 * where the uncommented floating point values were rounded to 20 decimal digits.
 *
 * However, we modified the above tables so that the K values is the maximum
 * of those 3 tables.  In other words, we force max_class - min_class to be consistent
 * for all table entries.
 *
 * Fixing K for all tables has the effect of considering even longer runs
 * with lesser probabilities.  It has the advantage of making the pi_term[]
 * arrays all the same length.  Therefore we used the following calls:
 *
 *      runstbl(8, 1, 7)
 *      runstbl(128, 4, 10)
 *      runstbl(10000, 10, 16)
 *
 *      NOTE: For large values of M (such as 10000), this script takes
 *            a very long time to run (as in around 8 CPU days)!
 *
 * NOTE: MIN_LENGTH_LONGESTRUN <= n <= ULONG_MAX
 */
static const struct runs_table runs_table[] = {

	/*
	 * NOTE: The K = 3, M = 8 case where the max_class was 4
	 * 	 has been extended so that max_class - min_class
	 *       is CLASS_COUNT_LONGEST_RUN (6).
	 */
	{
	 MIN_LENGTH_LONGESTRUN, 	// min_n
	 8, 				// M
	 1, 				// min_class
	 7,				// max_class

	 {
	  0.21484375,			// pi_term[0], v <= 1
	  0.3671875,			// pi_term[1], v == 2
	  0.23046875,			// pi_term[2], v == 3
	  0.109375,			// pi_term[3], v == 4
	  0.046875,			// pi_term[4], v == 5
	  0.01953125,			// pi_term[5], v == 6
	  0.01171875,			// pi_term[6], v >= 7
	  },
	 },

	/*
	 * NOTE: The K = 6, M = 128 case where the max_class was 9
	 * 	 has been extended so that max_class - min_class
	 *       is CLASS_COUNT_LONGEST_RUN (6).
	 */
	{
	 6272,				// min_n
	 128,				// M
	 4,				// min_class
	 10,				// max_class

	 {
	  0.11740357883779323143,	// pi_term[0], v <= 4
	  0.24295595927745485921,	// pi_term[1], v == 5
	  0.24936348317907796964,	// pi_term[2], v == 6
	  0.17517706034678234949,	// pi_term[3], v == 7
	  0.10270117130405369391,	// pi_term[4], v == 8
	  0.05521550943390406075,	// pi_term[5], v == 9
	  0.05718333762093383558,	// pi_term[6], v >= 10
	  },
	 },

	{
	 750000, 			// min_n
	 10000,				// M
	 10,				// min_class
	 16,				// max_class

	 {
	  0.08663231117995278587,	// pi_term[0], v <= 10
	  0.20820064838760340198,	// pi_term[1], v == 11
	  0.24841858194169954122,	// pi_term[2], v == 12
	  0.19391278674165693004,	// pi_term[3], v == 13
	  0.12145848508900441468,	// pi_term[4], v == 14
	  0.06801108930393995064,	// pi_term[5], v == 15
	  0.07336609745614297557,	// pi_term[6], v >= 16
	  },
	 },
};


/*
 * Forward static function declarations
 */
static bool LongestRunOfOnes_print_stat(FILE * stream, struct state *state, struct LongestRunOfOnes_private_stats *stat,
					double p_value);
static bool LongestRunOfOnes_print_p_value(FILE * stream, double p_value);
static void LongestRunOfOnes_metric_print(struct state *state, long int sampleCount, long int toolow, long int *freqPerBin);


/*
 * LongestRunOfOnes_init - initialize the Longest Runs test
 *
 * given:
 *      state           // run state to test under
 *
 * This function is called for each and every iteration noted in state->tp.numOfBitStreams.
 *
 * NOTE: The initialize function must be called first.
 */
void
LongestRunOfOnes_init(struct state *state)
{
	long int n;		// Length of a single bit stream

	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(110, __func__, "state arg is NULL");
	}
	if (state->testVector[test_num] != true) {
		dbg(DBG_LOW, "init driver interface for %s[%d] called when test vector was false", state->testNames[test_num],
		    test_num);
		return;
	}
	if (state->cSetup != true) {
		err(110, __func__, "test constants not setup prior to calling %s for %s[%d]",
		    __func__, state->testNames[test_num], test_num);
	}

	/*
	 * Collect parameters from state
	 */
	n = state->tp.n;

	/*
	 * Disable test if conditions do not permit this test from being run
	 */
	if (n < MIN_LENGTH_LONGESTRUN) {
		warn(__func__, "disabling test %s[%d]: requires bitcount(n): %ld >= %d",
		     state->testNames[test_num], test_num, n, MIN_LENGTH_LONGESTRUN);
		state->testVector[test_num] = false;
		return;
	}

	/*
	 * Create working sub-directory if forming files such as results.txt and stats.txt
	 */
	if (state->resultstxtFlag == true) {
		state->subDir[test_num] = precheckSubdir(state, state->testNames[test_num]);
		dbg(DBG_HIGH, "test %s[%d] will use subdir: %s", state->testNames[test_num], test_num, state->subDir[test_num]);
	}

	/*
	 * Allocate dynamic arrays
	 */
	if (state->resultstxtFlag == true) {
		state->stats[test_num] = create_dyn_array(sizeof(struct LongestRunOfOnes_private_stats),
							  DEFAULT_CHUNK, state->tp.numOfBitStreams, false);        // stats.txt
	}
	state->p_val[test_num] = create_dyn_array(sizeof(double),
						  DEFAULT_CHUNK, state->tp.numOfBitStreams, false);	// results.txt

	/*
	 * Determine format of data*.txt filenames based on state->partitionCount[test_num]
	 * NOTE: If we are not partitioning the p_values, no data*.txt filenames are needed
	 */
	state->datatxt_fmt[test_num] = data_filename_format(state->partitionCount[test_num]);
	dbg(DBG_HIGH, "%s[%d] will form data*.txt filenames with the following format: %s",
	    state->testNames[test_num], test_num, state->datatxt_fmt[test_num]);

	return;
}


/*
 * LongestRunOfOnes_iterate - iterate one bit stream for Longest Runs test
 *
 * given:
 *      state           // run state to test under
 *
 * This function is called for each and every iteration noted in state->tp.numOfBitStreams.
 *
 * NOTE: The initialize function must be called first.
 */
void
LongestRunOfOnes_iterate(struct thread_state *thread_state)
{
	struct LongestRunOfOnes_private_stats stat;	// Stats for this iteration
	const double *pi_term;	// Theoretical probabilities (see runs_table struct above)
	long int n;		// Length of a single bit stream
	double p_value;		// p_value iteration test result(s)
	int min_class;		// Minimum length to consider
	int max_class;		// Maximum length to consider
	long int v_obs;		// Current maximum run length for current block
	double chi_term;	// Term for the statistic formula: chi^2 = chi_term * chi_term
	long int run;		// Counter used to find longest run of ones
	long int i;
	long int j;

	/*
	 * Check preconditions (firewall)
	 */
	if (thread_state == NULL) {
		err(111, __func__, "thread_state arg is NULL");
	}
	struct state *state = thread_state->global_state;
	if (state == NULL) {
		err(111, __func__, "state arg is NULL");
	}
	if (state->testVector[test_num] != true) {
		dbg(DBG_LOW, "iterate function[%d] %s called when test vector was false", test_num, __func__);
		return;
	}
	if (state->epsilon == NULL) {
		err(111, __func__, "state->epsilon is NULL");
	}
	if (state->epsilon[thread_state->thread_id] == NULL) {
		err(111, __func__, "state->epsilon[%ld] is NULL", thread_state->thread_id);
	}

	/*
	 * Collect parameters from state
	 */
	n = state->tp.n;

	/*
	 * Find the appropriate runs_table entry that first satisfies the min_n requirement
	 */
	stat.runs_table_index = 0;
	while ((stat.runs_table_index < (sizeof(runs_table) / sizeof(runs_table[0]))) &&
	       (n > runs_table[stat.runs_table_index].min_n)) {
		++stat.runs_table_index;
	}
	if (stat.runs_table_index >= (sizeof(runs_table) / sizeof(runs_table[0]))) {
		// ran off end of table, use the last table entry
		stat.runs_table_index = (sizeof(runs_table) / sizeof(runs_table[0])) - 1;
	}

	/*
	 * Setup test parameters
	 */
	stat.M = runs_table[stat.runs_table_index].M;
	pi_term = runs_table[stat.runs_table_index].pi_term;
	min_class = runs_table[stat.runs_table_index].min_class;
	max_class = runs_table[stat.runs_table_index].max_class;
	stat.N = n / stat.M;

	/*
	 * Clear counters
	 */
	memset(stat.count, 0, sizeof(stat.count));

	/*
	 * Step 1: partition the sequence into N independent M-bit blocks
	 */
	for (i = 0; i < stat.N; i++) {

		/*
		 * Step 2a: determine maximum 1-bit run length for this block
		 */
		v_obs = 0;
		run = 0;
		for (j = 0; j < stat.M; j++) {
			if (state->epsilon[thread_state->thread_id][(i * stat.M) + j] == 1) {
				run++;
				if (run > v_obs) {
					v_obs = run;
				}
			} else {
				run = 0;
			}
		}

		/*
		 * Step 2b: count the class based on the current run length
		 */
		if (v_obs <= min_class) {
			stat.count[0]++;
		} else if (v_obs <= max_class) {
			stat.count[v_obs - min_class]++;
		} else {
			stat.count[CLASS_COUNT_LONGEST_RUN]++;
		}
	}

	/*
	 * Step 3: compute the test statistic
	 */
	stat.chi2 = 0.0;
	for (i = 0; i <= CLASS_COUNT_LONGEST_RUN; i++) {
		chi_term = stat.count[i] - (stat.N * pi_term[i]);
		stat.chi2 += chi_term * chi_term / (stat.N * pi_term[i]);
	}

	/*
	 * Step 4: compute the test P-value
	 */
	p_value = cephes_igamc((double) CLASS_COUNT_LONGEST_RUN / 2.0, stat.chi2 / 2.0);

	/*
	 * Lock mutex before making changes to the shared state
	 */
	if (thread_state->mutex != NULL) {
		pthread_mutex_lock(thread_state->mutex);
	}

	/*
	 * Record success or failure for this iteration
	 */
	state->count[test_num]++;	// Count this iteration
	state->valid[test_num]++;	// Count this valid iteration
	if (isNegative(p_value)) {
		state->failure[test_num]++;	// Bogus p_value < 0.0 treated as a failure
		stat.success = false;		// FAILURE
		warn(__func__, "iteration %ld of test %s[%d] produced bogus p_value: %f < 0.0\n",
		     thread_state->iteration_being_done + 1, state->testNames[test_num], test_num, p_value);
	} else if (isGreaterThanOne(p_value)) {
		state->failure[test_num]++;	// Bogus p_value > 1.0 treated as a failure
		stat.success = false;		// FAILURE
		warn(__func__, "iteration %ld of test %s[%d] produced bogus p_value: %f > 1.0\n",
		     thread_state->iteration_being_done + 1, state->testNames[test_num], test_num, p_value);
	} else if (p_value < state->tp.alpha) {
		state->valid_p_val[test_num]++;	// Valid p_value in [0.0, 1.0] range
		state->failure[test_num]++;	// Valid p_value but too low is a failure
		stat.success = false;		// FAILURE
	} else {
		state->valid_p_val[test_num]++;	// Valid p_value in [0.0, 1.0] range
		state->success[test_num]++;	// Valid p_value not too low is a success
		stat.success = true;		// SUCCESS
	}

	/*
	 * Record values computed during this iteration
	 */
	if (state->resultstxtFlag == true) {
		append_value(state->stats[test_num], &stat);
	}
	append_value(state->p_val[test_num], &p_value);

	/*
	 * Unlock mutex after making changes to the shared state
	 */
	if (thread_state->mutex != NULL) {
		pthread_mutex_unlock(thread_state->mutex);
	}

	return;
}


/*
 * LongestRunOfOnes_print_stat - print private_stats information to the end of an open file
 *
 * given:
 *      stream          // open writable FILE stream
 *      state           // run state to test under
 *      stat            // struct LongestRunOfOnes_private_stats for format and print
 *      p_value         // p_value iteration test result(s)
 *
 * returns:
 *      true --> no errors
 *      false --> an I/O error occurred
 */
static bool
LongestRunOfOnes_print_stat(FILE * stream, struct state *state, struct LongestRunOfOnes_private_stats *stat, double p_value)
{
	int min_class;		// minimum length to consider
	int max_class;		// maximum length to consider
	int io_ret;		// I/O return status
	int i;

	/*
	 * Check preconditions (firewall)
	 */
	if (stream == NULL) {
		err(112, __func__, "stream arg is NULL");
	}
	if (state == NULL) {
		err(112, __func__, "state arg is NULL");
	}
	if (stat == NULL) {
		err(112, __func__, "stat arg is NULL");
	}
	if (stat->runs_table_index < 0) {
		err(112, __func__, "runs_table_index: %d < 0", stat->runs_table_index);
	}
	if (stat->runs_table_index >= sizeof(runs_table) / sizeof(runs_table[0])) {
		err(112, __func__, "runs_table_index: %d off end of runs_table array: %ld",
		    stat->runs_table_index, sizeof(runs_table) / sizeof(runs_table[0]));
	}
	if (p_value == NON_P_VALUE && stat->success == true) {
		err(112, __func__, "p_value was set to NON_P_VALUE but stat->success == true");
	}

	/*
	 * Note class minimum
	 */
	min_class = runs_table[stat->runs_table_index].min_class;
	max_class = runs_table[stat->runs_table_index].max_class;

	/*
	 * Print stat to a file
	 */
	if (state->legacy_output == true) {
		io_ret = fprintf(stream, "\t\t\t  LONGEST RUNS OF ONES TEST\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\t---------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\tCOMPUTATIONAL INFORMATION:\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\t---------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
	} else {
		io_ret = fprintf(stream, "\t\t\t  Longest runs of ones test\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\t-------------------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
	}
	io_ret = fprintf(stream, "\t\t(a) N (# of blocks)  = %ld\n", stat->N);
	if (io_ret <= 0) {
		return false;
	}
	io_ret = fprintf(stream, "\t\t(b) M (block length) = %ld\n", stat->M);
	if (io_ret <= 0) {
		return false;
	}
	io_ret = fprintf(stream, "\t\t(c) Chi^2            = %f\n", stat->chi2);
	if (io_ret <= 0) {
		return false;
	}
	if (state->legacy_output == true) {
		io_ret = fprintf(stream, "\t\t---------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\t      F R E Q U E N C Y\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\t---------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
	} else {
		io_ret = fprintf(stream, "\t\t-------------------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\tCount per frequency class\n");
		if (io_ret <= 0) {
			return false;
		}
		io_ret = fprintf(stream, "\t\t-------------------------------------------------------\n");
		if (io_ret <= 0) {
			return false;
		}
	}

	/*
	 * Legacy output: header and nu counts
	 */
	if (state->legacy_output == true) {

		/*
		 * Print counter header
		 */
		io_ret = fprintf(stream, "\t\t");
		if (io_ret <= 0) {
			return false;
		}
		for (i = 0; i <= CLASS_COUNT_LONGEST_RUN; ++i) {

			/*
			 * Print class type
			 */
			switch (i) {
			case 0:
				// Class is a <= class
				io_ret = fprintf(stream, "<=%2d", min_class);
				if (io_ret <= 0) {
					return false;
				}
				break;
			case CLASS_COUNT_LONGEST_RUN:
				// Class is a >= class
				io_ret = fprintf(stream, " >=%2d", max_class);
				if (io_ret <= 0) {
					return false;
				}
				break;
			default:
				// Class is a == class
				io_ret = fprintf(stream, " %3d", i + min_class);
				if (io_ret <= 0) {
					return false;
				}
				break;
			}
		}
		io_ret = fprintf(stream, " P-value  Assignment\n");
		if (io_ret <= 0) {
			return false;
		}

		/*
		 * Print counts
		 */
		io_ret = fprintf(stream, "\t\t");
		if (io_ret <= 0) {
			return false;
		}
		for (i = 0; i <= CLASS_COUNT_LONGEST_RUN - 1; ++i) {
			io_ret = fprintf(stream, " %3ld", stat->count[i]);
			if (io_ret <= 0) {
				return false;
			}
		}
		io_ret = fprintf(stream, "  %3ld ", stat->count[CLASS_COUNT_LONGEST_RUN]);
		if (io_ret <= 0) {
			return false;
		}
	}

	/*
	 * New output: header and nu counts
	 */
	else {

		/*
		 * Print counter header
		 */
		io_ret = fprintf(stream, "\t\t  ");
		if (io_ret <= 0) {
			return false;
		}
		for (i = 0; i <= CLASS_COUNT_LONGEST_RUN; ++i) {

			/*
			 * Print class type
			 */
			switch (i) {
			case 0:
				// Class is a <= class
				io_ret = fprintf(stream, "<= %-5d", min_class);
				if (io_ret <= 0) {
					return false;
				}
				break;
			case CLASS_COUNT_LONGEST_RUN:
				// Class is a >= class
				io_ret = fprintf(stream, ">= %-5d", max_class);
				if (io_ret <= 0) {
					return false;
				}
				break;
			default:
				// Class is a == class
				io_ret = fprintf(stream, " = %-5d", i + min_class);
				if (io_ret <= 0) {
					return false;
				}
				break;
			}
		}
		io_ret = fputc('\n', stream);
		if (io_ret == EOF) {
			return false;
		}

		/*
		 * Print counts
		 */
		io_ret = fprintf(stream, "\t\t");
		if (io_ret <= 0) {
			return false;
		}
		for (i = 0; i <= CLASS_COUNT_LONGEST_RUN; ++i) {
			io_ret = fprintf(stream, " %7ld", stat->count[i]);
			if (io_ret <= 0) {
				return false;
			}
		}
		io_ret = fputc('\n', stream);
		if (io_ret == EOF) {
			return false;
		}
	}

	/*
	 * Report on success or failure
	 */
	if (stat->success == true) {
		io_ret = fprintf(stream, "SUCCESS\t\tp_value = %f\n\n", p_value);
		if (io_ret <= 0) {
			return false;
		}
	} else if (p_value == NON_P_VALUE) {
		io_ret = fprintf(stream, "FAILURE\t\tp_value = __INVALID__\n\n");
		if (io_ret <= 0) {
			return false;
		}
	} else {
		io_ret = fprintf(stream, "FAILURE\t\tp_value = %f\n\n", p_value);
		if (io_ret <= 0) {
			return false;
		}
	}

	/*
	 * All printing successful
	 */
	return true;
}


/*
 * LongestRunOfOnes_print_p_value - print p_value information to the end of an open file
 *
 * given:
 *      stream          // open writable FILE stream
 *      stat            // struct LongestRunOfOnes_private_stats for format and print
 *      p_value         // p_value iteration test result(s)
 *
 * returns:
 *      true --> no errors
 *      false --> an I/O error occurred
 */
static bool
LongestRunOfOnes_print_p_value(FILE * stream, double p_value)
{
	int io_ret;		// I/O return status

	/*
	 * Check preconditions (firewall)
	 */
	if (stream == NULL) {
		err(113, __func__, "stream arg is NULL");
	}

	/*
	 * Print p_value to a file
	 */
	if (p_value == NON_P_VALUE) {
		io_ret = fprintf(stream, "__INVALID__\n");
		if (io_ret <= 0) {
			return false;
		}
	} else {
		io_ret = fprintf(stream, "%f\n", p_value);
		if (io_ret <= 0) {
			return false;
		}
	}

	/*
	 * All printing successful
	 */
	return true;
}


/*
 * LongestRunOfOnes_print - print to results.txt, data*.txt, stats.txt for all iterations
 *
 * given:
 *      state           // run state to test under
 *
 * This function is called for once to print dynamic arrays into
 * results.txt, data*.txt, stats.txt.
 *
 * NOTE: The initialize and iterate functions must be called before this function is called.
 */
void
LongestRunOfOnes_print(struct state *state)
{
	struct LongestRunOfOnes_private_stats *stat;	// Pointer to statistics of an iteration
	double p_value;			// p_value iteration test result(s)
	FILE *stats = NULL;		// Open stats.txt file
	FILE *results = NULL;		// Open results.txt file
	FILE *data = NULL;		// Open data*.txt file
	char *stats_txt = NULL;		// Pathname for stats.txt
	char *results_txt = NULL;	// Pathname for results.txt
	char *data_txt = NULL;		// Pathname for data*.txt
	char data_filename[BUFSIZ + 1];	// Basename for a given data*.txt pathname
	bool ok;			// true -> I/O was OK
	int snprintf_ret;		// snprintf return value
	int io_ret;			// I/O return status
	long int i;
	long int j;

	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(114, __func__, "state arg is NULL");
	}
	if (state->testVector[test_num] != true) {
		dbg(DBG_HIGH, "Print driver interface for %s[%d] called when test vector was false", state->testNames[test_num],
		    test_num);
		return;
	}
	if (state->resultstxtFlag == false) {
		dbg(DBG_HIGH, "Print driver interface for %s[%d] was not enabled with -s", state->testNames[test_num], test_num);
		return;
	}
	if (state->partitionCount[test_num] < 1) {
		err(114, __func__,
		    "print driver interface for %s[%d] called with state.partitionCount: %d < 0",
		    state->testNames[test_num], test_num, state->partitionCount[test_num]);
	}
	if (state->p_val[test_num]->count != (state->tp.numOfBitStreams * state->partitionCount[test_num])) {
		err(114, __func__,
		    "print driver interface for %s[%d] called with p_val count: %ld != %ld*%d=%ld",
		    state->testNames[test_num], test_num, state->p_val[test_num]->count,
		    state->tp.numOfBitStreams, state->partitionCount[test_num],
		    state->tp.numOfBitStreams * state->partitionCount[test_num]);
	}
	if (state->datatxt_fmt[test_num] == NULL) {
		err(114, __func__, "format for data0*.txt filename is NULL");
	}

	/*
	 * Open stats.txt file
	 */
	stats_txt = filePathName(state->subDir[test_num], "stats.txt");
	dbg(DBG_HIGH, "about to open/truncate: %s", stats_txt);
	stats = openTruncate(stats_txt);

	/*
	 * Open results.txt file
	 */
	results_txt = filePathName(state->subDir[test_num], "results.txt");
	dbg(DBG_HIGH, "about to open/truncate: %s", results_txt);
	results = openTruncate(results_txt);

	/*
	 * Write results.txt and stats.txt files
	 */
	for (i = 0; i < state->stats[test_num]->count; ++i) {

		/*
		 * Locate stat for this iteration
		 */
		stat = addr_value(state->stats[test_num], struct LongestRunOfOnes_private_stats, i);

		/*
		 * Get p_value for this iteration
		 */
		p_value = get_value(state->p_val[test_num], double, i);

		/*
		 * Print stat to stats.txt
		 */
		errno = 0;	// paranoia
		ok = LongestRunOfOnes_print_stat(stats, state, stat, p_value);
		if (ok == false) {
			errp(114, __func__, "error in writing to %s", stats_txt);
		}

		/*
		 * Print p_value to results.txt
		 */
		errno = 0;	// paranoia
		ok = LongestRunOfOnes_print_p_value(results, p_value);
		if (ok == false) {
			errp(114, __func__, "error in writing to %s", results_txt);
		}
	}

	/*
	 * Flush and close stats.txt, free pathname
	 */
	errno = 0;		// paranoia
	io_ret = fflush(stats);
	if (io_ret != 0) {
		errp(114, __func__, "error flushing to: %s", stats_txt);
	}
	errno = 0;		// paranoia
	io_ret = fclose(stats);
	if (io_ret != 0) {
		errp(114, __func__, "error closing: %s", stats_txt);
	}
	free(stats_txt);
	stats_txt = NULL;

	/*
	 * Flush and close results.txt, free pathname
	 */
	errno = 0;		// paranoia
	io_ret = fflush(results);
	if (io_ret != 0) {
		errp(114, __func__, "error flushing to: %s", results_txt);
	}
	errno = 0;		// paranoia
	io_ret = fclose(results);
	if (io_ret != 0) {
		errp(114, __func__, "error closing: %s", results_txt);
	}
	free(results_txt);
	results_txt = NULL;

	/*
	 * Write data*.txt for each data file if we need to partition results
	 */
	if (state->partitionCount[test_num] > 1) {
		for (j = 0; j < state->partitionCount[test_num]; ++j) {

			/*
			 * Form the data*.txt basename
			 */
			errno = 0;	// paranoia
			snprintf_ret = snprintf(data_filename, BUFSIZ, state->datatxt_fmt[test_num], j + 1);
			data_filename[BUFSIZ] = '\0';	// paranoia
			if (snprintf_ret <= 0 || snprintf_ret >= BUFSIZ || errno != 0) {
				errp(114, __func__,
				     "snprintf failed for %d bytes for data%03ld.txt, returned: %d", BUFSIZ, j + 1, snprintf_ret);
			}

			/*
			 * Form the data*.txt filename
			 */
			data_txt = filePathName(state->subDir[test_num], data_filename);
			dbg(DBG_HIGH, "about to open/truncate: %s", data_txt);
			data = openTruncate(data_txt);

			/*
			 * Write this particular data*.txt filename
			 */
			if (j < state->p_val[test_num]->count) {
				for (i = j; i < state->p_val[test_num]->count; i += state->partitionCount[test_num]) {

					/*
					 * Get p_value for an iteration belonging to this data*.txt filename
					 */
					p_value = get_value(state->p_val[test_num], double, i);

					/*
					 * Print p_value to results.txt
					 */
					errno = 0;	// paranoia
					ok = LongestRunOfOnes_print_p_value(data, p_value);
					if (ok == false) {
						errp(114, __func__, "error in writing to %s", data_txt);
					}

				}
			}

			/*
			 * Flush and close data*.txt, free pathname
			 */
			errno = 0;	// paranoia
			io_ret = fflush(data);
			if (io_ret != 0) {
				errp(114, __func__, "error flushing to: %s", data_txt);
			}
			errno = 0;	// paranoia
			io_ret = fclose(data);
			if (io_ret != 0) {
				errp(114, __func__, "error closing: %s", data_txt);
			}
			free(data_txt);
			data_txt = NULL;
		}
	}

	return;
}


/*
 * LongestRunOfOnes_metric_print - print uniformity and proportional information for a tallied count
 *
 * given:
 *      state           // run state to test under
 *      sampleCount             // Number of bitstreams in which we counted p_values
 *      toolow                  // p_values that were below alpha
 *      freqPerBin              // uniformity frequency bins
 */
static void
LongestRunOfOnes_metric_print(struct state *state, long int sampleCount, long int toolow, long int *freqPerBin)
{
	long int passCount;			// p_values that pass
	double p_hat;				// 1 - alpha
	double proportion_threshold_max;	// When passCount is too high
	double proportion_threshold_min;	// When passCount is too low
	double chi2;				// Sum of chi^2 for each tenth
	double uniformity;			// Uniformity of frequency bins
	double expCount;			// Sample size divided by frequency bin count
	int io_ret;				// I/O return status
	long int i;

	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(115, __func__, "state arg is NULL");
	}
	if (freqPerBin == NULL) {
		err(115, __func__, "freqPerBin arg is NULL");
	}

	/*
	 * Determine the number tests that passed
	 */
	if ((sampleCount <= 0) || (sampleCount < toolow)) {
		passCount = 0;
	} else {
		passCount = sampleCount - toolow;
	}

	/*
	 * Determine proportion thresholds
	 */
	p_hat = 1.0 - state->tp.alpha;
	proportion_threshold_max = (p_hat + 3.0 * sqrt((p_hat * state->tp.alpha) / sampleCount)) * sampleCount;
	proportion_threshold_min = (p_hat - 3.0 * sqrt((p_hat * state->tp.alpha) / sampleCount)) * sampleCount;

	/*
	 * Compute uniformity p-value
	 */
	chi2 = 0.0;
	expCount = sampleCount / state->tp.uniformity_bins;
	if (expCount <= 0.0) {
		// Not enough samples for uniformity check
		uniformity = 0.0;
	} else {
		// Sum chi squared of the frequency bins
		for (i = 0; i < state->tp.uniformity_bins; ++i) {
			chi2 += (freqPerBin[i] - expCount) * (freqPerBin[i] - expCount) / expCount;
		}
		// Uniformity threshold level
		uniformity = cephes_igamc((state->tp.uniformity_bins - 1.0) / 2.0, chi2 / 2.0);
	}

	/*
	 * Save or print results
	 */
	if (state->legacy_output == true) {

		/*
		 * Output uniformity results in traditional format to finalAnalysisReport.txt
		 */
		for (i = 0; i < state->tp.uniformity_bins; ++i) {
			fprintf(state->finalRept, "%3ld ", freqPerBin[i]);
		}
		if (expCount <= 0.0) {
			// Not enough samples for uniformity check
			fprintf(state->finalRept, "    ----    ");
			dbg(DBG_HIGH, "too few iterations for uniformity check on %s", state->testNames[test_num]);
		} else if (uniformity < state->tp.uniformity_level) {
			// Uniformity failure (the uniformity p-value is smaller than the minimum uniformity_level (default 0.0001)
			fprintf(state->finalRept, " %8.6f * ", uniformity);
			dbg(DBG_HIGH, "metrics detected uniformity failure for %s", state->testNames[test_num]);
		} else {
			// Uniformity success
			fprintf(state->finalRept, " %8.6f   ", uniformity);
			dbg(DBG_HIGH, "metrics detected uniformity success for %s", state->testNames[test_num]);
		}

		/*
		 * Output proportional results in traditional format to finalAnalysisReport.txt
		 */
		if (sampleCount == 0) {
			// Not enough samples for proportional check
			fprintf(state->finalRept, " ------     %s\n", state->testNames[test_num]);
			dbg(DBG_HIGH, "too few samples for proportional check on %s", state->testNames[test_num]);
		} else if ((passCount < proportion_threshold_min) || (passCount > proportion_threshold_max)) {
			// Proportional failure
			fprintf(state->finalRept, "%4ld/%-4ld *	 %s\n", passCount, sampleCount, state->testNames[test_num]);
			dbg(DBG_HIGH, "metrics detected proportional failure for %s", state->testNames[test_num]);
		} else {
			// Proportional success
			fprintf(state->finalRept, "%4ld/%-4ld	 %s\n", passCount, sampleCount, state->testNames[test_num]);
			dbg(DBG_HIGH, "metrics detected proportional success for %s", state->testNames[test_num]);
		}

		/*
		 * Flush the output file buffer
		 */
		errno = 0;                // paranoia
		io_ret = fflush(state->finalRept);
		if (io_ret != 0) {
			errp(115, __func__, "error flushing to: %s", state->finalReptPath);
		}

	} else {
		bool uniformity_passed = true;
		bool proportion_passed = true;

		/*
		 * Check uniformity results
		 */
		if (expCount <= 0.0 || uniformity < state->tp.uniformity_level) {
			// Uniformity failure or not enough samples for uniformity check
			uniformity_passed = false;
			dbg(DBG_HIGH, "metrics detected uniformity failure for %s", state->testNames[test_num]);
		}

		/*
		 * Check proportional results
		 */
		if (sampleCount == 0 || (passCount < proportion_threshold_min) || (passCount > proportion_threshold_max)) {
			// Proportional failure or not enough samples for proportional check
			proportion_passed = false;
			dbg(DBG_HIGH, "metrics detected proportional failure for %s", state->testNames[test_num]);
		}

		if (proportion_passed == false && uniformity_passed == false) {
			state->metric_results.longest_run = FAILED_BOTH;
		} else if (proportion_passed == false) {
			state->metric_results.longest_run = FAILED_PROPORTION;
		} else if (uniformity_passed == false) {
			state->metric_results.longest_run = FAILED_UNIFORMITY;
		} else {
			state->metric_results.longest_run = PASSED_BOTH;
			state->successful_tests++;
		}
	}

	return;
}


/*
 * LongestRunOfOnes_metrics - uniformity and proportional analysis of a test
 *
 * given:
 *      state           // run state to test under
 *
 * This function is called once to complete the test analysis for all iterations.
 *
 * NOTE: The initialize and iterate functions must be called before this function is called.
 */
void
LongestRunOfOnes_metrics(struct state *state)
{
	long int sampleCount;	// Number of bitstreams in which we will count p_values
	long int toolow;	// p_values that were below alpha
	double p_value;		// p_value iteration test result(s)
	long int *freqPerBin;	// Uniformity frequency bins
	long int i;
	long int j;

	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(116, __func__, "state arg is NULL");
	}
	if (state->testVector[test_num] != true) {
		dbg(DBG_LOW, "metrics driver interface for %s[%d] called when test vector was false", state->testNames[test_num],
		    test_num);
		return;
	}
	if (state->partitionCount[test_num] < 1) {
		err(116, __func__,
		    "metrics driver interface for %s[%d] called with state.partitionCount: %d < 0",
		    state->testNames[test_num], test_num, state->partitionCount[test_num]);
	}
	if (state->p_val[test_num]->count != (state->tp.numOfBitStreams * state->partitionCount[test_num])) {
		warn(__func__,
		     "metrics driver interface for %s[%d] called with p_val length: %ld != bit streams: %ld",
		     state->testNames[test_num], test_num, state->p_val[test_num]->count,
		     state->tp.numOfBitStreams * state->partitionCount[test_num]);
	}

	/*
	 * Allocate uniformity frequency bins
	 */
	freqPerBin = malloc(state->tp.uniformity_bins * sizeof(freqPerBin[0]));
	if (freqPerBin == NULL) {
		errp(116, __func__, "cannot malloc of %ld elements of %ld bytes each for freqPerBin",
		     state->tp.uniformity_bins, sizeof(long int));
	}

	/*
	 * Print for each partition (or the whole set of p_values if partitionCount is 1)
	 */
	for (j = 0; j < state->partitionCount[test_num]; ++j) {

		/*
		 * Set counters to zero
		 */
		toolow = 0;
		sampleCount = 0;
		memset(freqPerBin, 0, state->tp.uniformity_bins * sizeof(freqPerBin[0]));

		/*
		 * Tally p_value
		 */
		for (i = j; i < state->p_val[test_num]->count; i += state->partitionCount[test_num]) {

			// Get the iteration p_value
			p_value = get_value(state->p_val[test_num], double, i);
			if (p_value == NON_P_VALUE) {
				continue;	// the test was not possible for this iteration
			}
			// Case: random excursion test
			if (state->is_excursion[test_num] == true) {
				// Random excursion tests only sample > 0 p_values
				if (p_value > 0.0) {
					++sampleCount;
				} else {
					// Ignore p_value of 0 for random excursion tests
					continue;
				}
			}
			// Case: general (non-random excursion) test
			else {
				// All other tests count all p_values
				++sampleCount;
			}

			// Count the number of p_values below alpha
			if (p_value < state->tp.alpha) {
				++toolow;
			}
			// Tally the p_value in a uniformity bin
			if (p_value >= 1.0) {
				++freqPerBin[state->tp.uniformity_bins - 1];
			} else if (p_value >= 0.0) {
				++freqPerBin[(int) floor(p_value * (double) state->tp.uniformity_bins)];
			} else {
				++freqPerBin[0];
			}
		}

		/*
		 * Print uniformity and proportional information for a tallied count
		 */
		LongestRunOfOnes_metric_print(state, sampleCount, toolow, freqPerBin);

		/*
		 * Track maximum samples
		 */
		if (state->is_excursion[test_num] == true) {
			if (sampleCount > state->maxRandomExcursionSampleSize) {
				state->maxRandomExcursionSampleSize = sampleCount;
			}
		} else {
			if (sampleCount > state->maxGeneralSampleSize) {
				state->maxGeneralSampleSize = sampleCount;
			}
		}
	}

	/*
	 * Free allocated storage
	 */
	free(freqPerBin);
	freqPerBin = NULL;

	return;
}


/*
 * LongestRunOfOnes_destroy - post process results for this text
 *
 * given:
 *      state           // run state to test under
 *
 * This function is called once to cleanup any storage or state
 * associated with this test.
 */
void
LongestRunOfOnes_destroy(struct state *state)
{
	/*
	 * Check preconditions (firewall)
	 */
	if (state == NULL) {
		err(117, __func__, "state arg is NULL");
	}
	if (state->testVector[test_num] != true) {
		dbg(DBG_LOW, "destroy function[%d] %s called when test vector was false", test_num, __func__);
		return;
	}

	/*
	 * Free dynamic arrays
	 */
	if (state->stats[test_num] != NULL) {
		free_dyn_array(state->stats[test_num]);
		free(state->stats[test_num]);
		state->stats[test_num] = NULL;
	}
	if (state->p_val[test_num] != NULL) {
		free_dyn_array(state->p_val[test_num]);
		free(state->p_val[test_num]);
		state->p_val[test_num] = NULL;
	}

	/*
	 * Free other test storage
	 */
	if (state->datatxt_fmt[test_num] != NULL) {
		free(state->datatxt_fmt[test_num]);
		state->datatxt_fmt[test_num] = NULL;
	}
	if (state->subDir[test_num] != NULL) {
		free(state->subDir[test_num]);
		state->subDir[test_num] = NULL;
	}

	return;
}
