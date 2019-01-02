#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>
#include "clomp_tm.h"

#ifdef RTM
#include "tm.h"
#endif

/* Some linux's don't prototype nearbyint, so do it here */
extern double nearbyint(double x);

/* Command line parameters, see usage info (initially -1 for sanity check)*/
long CLOMP_numThreads = -2;       /* > 0 or -1 valid */
long CLOMP_allocThreads = -2;     /* > 0 or -1 valid */
long CLOMP_numParts = -1;         /* > 0 valid */
long CLOMP_zonesPerPart = -1;     /* > 0 valid */
long CLOMP_flopScale = -1;        /* > 0 valid, 1 nominal */
long CLOMP_timeScale = -1;        /* > 0 valid, 100 nominal */
long CLOMP_scrubRate = -1;        /* > 0 valid, 66 nominal */
long CLOMP_zoneSize = -1;         /* > 0 valid, (sizeof(Zone) true min)*/
char *CLOMP_scatterMode = NULL;   /* "None","Adjacent", etc. (forced toupper)*/
long CLOMP_scatterPartMod = -1;   /* "Mods" PartId for scatter Mode */
char *CLOMP_scatterModeUnparsed = NULL; /* What user specfied before parsing*/
long CLOMP_altScatterCount = 0;  /* Number of scatterZones to use altScatter Mode*/
char *CLOMP_altScatterMode = NULL;   /* "None","Adjacent", etc. (forced toupper)*/
long CLOMP_altScatterPartMod = -1;/* "Mods" PartId for altScatter Mode */
long CLOMP_scatter = -1;       /* > 0 valid, 3 nominal */
long CLOMP_randomSeedOffset = 0;  /* Added to CLOMP_randomSeed at beginning */
char *CLOMP_exe_name = NULL;      /* Points to argv[0] */

/* Exposed so can modify seed via command line parameter -JCG /12/19/11 */
long CLOMP_randomSeed = 123456789; /* Changes as ran_small_int() called */


/* Calculated number of extra values that can be validily accessed at the
 * end of zone.   Used to touch every cache line in zone, so can use space
 * in caches.
 */
long CLOMP_numExtraValues = -1;

/* Holds start index for extra values so update_part can start at this
 * index and advance CALC_STRIDE indexs and hit exactly the last extra
 * value at the end.
 */
long CLOMP_startExtraValues = 1000000;

/* Part array working on (now array of Part pointers)*/
Part **partArray = NULL;

/* Used to avoid dividing by numParts */
double CLOMP_partRatio =0.0;

/* Number of iterations to do approximately 0.5 to 2 second of work on a
 * fast new machine (deterministic calculated using heuristics).   */
long CLOMP_num_iterations = 0.0;

/* Used to check residue of checksum */
double CLOMP_max_residue = 0.0;

/* Used to determine if a calculation difference is a rounding error or
 * the result of the compiler, runtime, or test doing something bad
 * (i.e., races, bad barriers, etc.).
 * The loose error bound is calculated to be slightly smaller than
 * the smallest update of any zone.  This should catch races preventing
 * an update from happenning.
 */
double CLOMP_error_bound = 0.0;

/* The tight error bound is calculated to be the smallest difference between
 * two adjacent zone updates.   This should detect swapped updates (which
 * probably is not possible).   May be tighter than needed.
 */
double CLOMP_tightest_error_bound = 0.0;

void print_usage()
{
	fprintf (stderr,
	     "Usage: %s numThreads allocThreads numParts zonesPerPart zoneSize \\\n"
	     "          scatterMode[%%mainMod][,altScatterCount,altScatterMode[%%altMod]] \\\n"
	     "          scatter flopScale randomSeed scrubRate timeScale\n",
	     CLOMP_exe_name);
	fprintf (stderr, "\n");
	fprintf (stderr, "  numThreads: Number of OpenMP threads to use (-1 for system default)\n");
	fprintf (stderr, "  allocThreads: #threads when allocating data (-1 for numThreads)\n");
	fprintf (stderr, "  numParts: Number of independent pieces of work (loop iterations)\n");
	fprintf (stderr, "            xN sets numParts to 'N * numThreads' (N >= 1)\n");
	fprintf (stderr, "  zonesPerPart: Number of zones in each part. (>= 1)\n");
	fprintf (stderr, "                dM sets zonesPerPart to 'M / numParts' (d6144 nominal)\n");
	fprintf (stderr, "  zoneSize: Bytes in zone, 1 calc for every 64 bytes (power of 2 >= 32 valid)\n");
	fprintf (stderr, "  flopScale: Scales flops/zone to increase memory reuse (1 nominal, >=1 Valid)\n");
	fprintf (stderr, "  randomSeed: Added to initial random seed (0 nominal, >=0 Valid)\n");
	fprintf (stderr, "  scrubRate: Hardware SpecId scrub rate (66 default, 6 recommended)\n");
	fprintf (stderr, "  timeScale: Scales target time per test (10-100 nominal, 1-100000 Valid)\n");

	fprintf (stderr,
	     "  scatterMode: None (no races), Stride1 (no races), Adjacent, firstParts,\n"
	     "               Random, InPart (no races), firstZone (no races), randFirstZone\n");
	fprintf (stderr,
	     "  NOTE: ALL scatterModes, even Random, produces the same layout every run.\n");
	fprintf (stderr,
	     "  mainMod: Applies %% mainMod to partId when doing scatterMode (more overlap)\n");
	fprintf (stderr,
	     "           /D sets mainMod to 'numParts / D'\n");
	fprintf (stderr,
	     "  altScatterCount: # of zones selected via altScatterMode (>=1 up to 90%% of total)\n");
	fprintf (stderr,
	     "  altScatterMode: Used instead of scatterMode for randomly selected updates\n");
	fprintf (stderr,
	     "  altMod: Applies %% altMod to partId when doing altScatterMode (more overlap)\n");
	fprintf (stderr,
	     "          /D sets altMod to 'numParts / D'\n");
	fprintf (stderr, "  scatter: Number of zones updated when 'updating' a zone  (>=1 Valid)\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "Some interesting testcases (last number controls run time, use <=10 for simulator):\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "Ten Atomic versus One TM (or using SE) testcases:\n");
	fprintf (stderr, "       No conflicts:  %s -1 -1 x8 d32768 256 Stride1 10 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "Very Rare conflicts:  %s -1 -1 x8 d32768 256 Stride1,8,Adjacent 10 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "\n");
	fprintf (stderr, "Prefetch-friendly testcases:\n");
	fprintf (stderr, "Very Rare conflicts:  %s -1 1 x4 d6144 128 None,10,Adjacent 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "     Rare conflicts:  %s -1 1 x4 d6144 128 Adjacent 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "     High conflicts:  %s -1 1 64 100 128 firstParts 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "       No conflicts:  %s -1 1 x1 d6144 128 Stride1 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "\n");
	fprintf (stderr, "TM Rand emulations (set altScatterCount to 6144 * 3 * target_percentage):\n");
	fprintf (stderr, "       TM Rand (1%%):  %s -1 1 x1 d6144 128 firstZone,184,randFirstZone 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "      TM Rand (50%%):  %s -1 1 x1 d6144 128 firstZone,9216,randFirstZone 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "      TM Rand (99%%):  %s -1 1 x1 d6144 128 randFirstZone,184,firstZone 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "                      Note: Need to flip scatter modes for >= 90%% altScatterCount\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "TM Rand plus varation where only two threads can hit same memory location:\n");
	fprintf (stderr, "  TM Rand plus (1%%):  %s -1 1 x1 d6144 128 firstZone,184,firstZone%%/2 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "  TM Rand plus (50%%):  %s -1 1 x1 d6144 128 firstZone,9216,firstZone%%/2 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "  TM Rand plus (99%%):  %s -1 1 x1 d6144 128 firstZone%%/2,184,firstZone 3 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "\n");
	fprintf (stderr, "Prefetch-unfriendly testcases:\n");
	fprintf (stderr, "Very Rare conflicts:  %s -1 1 64 100 128 InPart,10,Random 10 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "     Rare conflicts:  %s -1 1 64 100 128 Random 10 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "Huge atomic regions:  %s -1 1 64 10 128 Random 100 1 0 6 1000\n", CLOMP_exe_name);
	fprintf (stderr, "       No conflicts:  %s -1 1 64 100 128 InPart 10 1 0 6 1000\n", CLOMP_exe_name);
}

#define check_alignment(x) _check_alignment(CLOMP_XSTR(x), x)

/* Punt if posix_memalign didn't actually return memory aligned properly*/
void _check_alignment (char *name, void *ptr)
{
	/* Use zoneSize (expected alignment) to create address mask */
	long long align_mask = CLOMP_zoneSize -1;

	/* Should get 0 if data is properly aligned */
	long long alignment_check = (long long)ptr & (long long)align_mask;

	/* Punt if data not aligned as expected */
	if (alignment_check != 0)
	{
	printf ("*** Alignment check failed for zoneSize %li\n",
		CLOMP_zoneSize);
	printf ("Error: check_alignment: %s %p after mask (%llx) != 0 with mask %llx\n", name, ptr, alignment_check, align_mask);
	exit (1);
	}

}

/* Punt if extra_value calculation didn't actually return memory aligned
 * to 4 double boundary -JCG 4/1/11
 */
void check_SIMD_alignment (void *ptr)
{
	/* Use size of 4 doubles to create address mask */
	long long align_mask = 32 -1;

	/* Should get 0 if data is properly aligned */
	long long alignment_check = (long long)ptr & (long long)align_mask;

	/* Punt if data not aligned as expected */
	if (alignment_check != 0)
	{
	printf ("*** Alignment check failed for SIMD extra values\n");
	printf ("Error: check_SIMD_alignment: %p after mask (%llx) != 0 with mask %llx\n", ptr, alignment_check, align_mask);
	exit (1);
	}
}

/* Convert parm_val to a positive long value.  Punts if negative or not
 * completely translated
 */
long convert_to_positive_long (const char *parm_name, const char *parm_val)
{
	long val;
	char *endPtr;

	/* Sanity check */
	if ((parm_name == NULL) || (parm_val == NULL))
	{
	fprintf (stderr,
		 "Error in convert_to_positive_long: Passed NULL pointers!\n");
	exit (1);
	}

	/* Convert string to long */
	val = strtol (parm_val, &endPtr, 0);

	/* Make sure everything in string was converted */
	if (endPtr[0] != 0)
	{
	fprintf (stderr, "Error converting '%s' parameter value '%s' to long at '%s'!\n",
		 parm_name, parm_val, endPtr);
	exit (1);
	}

	/* Make sure final value > 0 except for numThreads and allocThreads,
	 * which also takes -1
	 */
	if ((strcmp (parm_name, "numThreads") == 0) ||
	(strcmp (parm_name, "allocThreads") == 0))
	{
	if ((val < 1) && (val != -1))
	{
	    fprintf (stderr, "Invalid value %ld for parameter %s, must be > 0 or -1!\n",
		     val, parm_name);

	    print_usage();
	    exit (1);
	}

	}
	/* Allow randomSeed to be 0 (the default value) */
	else if (strcmp (parm_name, "randomSeed") == 0)
	{
	if (val < 0)
	{
	    fprintf (stderr, "Invalid value %ld for parameter %s, must be >= 0!\n",
		     val, parm_name);

	    print_usage();
	    exit (1);
	}
	}
	/* Make sure final value > 0 (except for numThreads checked above) */
	else if (val < 1)
	{
	fprintf (stderr, "Invalid value %ld for parameter %s, must be > 0\n",
		 val, parm_name);

	print_usage();
	exit (1);
	}

	return (val);
}

/* Generate random integer between 0 and max (including max),
 * meant to produce same sequence on every run, independent of system.
 *
 * Based on the "good" random number generator illustrated in
 * Steve Park & Dave Geyer's rngs.c  (http://www.cs.wm.edu/~va/software/park/)
 * and described in:
 *  "Random Number Generators: Good Ones Are Hard To Find"
 *                   Steve Park and Keith Miller
 *              Communications of the ACM, October 1988
 */
int rand_small_int(int max)
{
	const long Q = 2147483647 / 48271;
	const long R = 2147483647 % 48271;
	long t;
	double fraction;
	int value;

	t = 48271 * (CLOMP_randomSeed % Q) - R * (CLOMP_randomSeed / Q);
	if (t > 0)
	CLOMP_randomSeed = t;
	else
	CLOMP_randomSeed = t + 2147483647;

	fraction = (double) CLOMP_randomSeed / 2147483647;
	value = fraction * (max + 1);

	/* Sanity check */
	if (value < 0)
	{
	fprintf (stderr,
		 "Algorithm error: rand_small_int returned %i (< 0)\n",
		 value);
	exit (1);
	}
	if (value > max)
	{
	fprintf (stderr,
		 "Algorithm error: rand_small_int returned %i (> max)\n",
		 value);
	exit (1);
	}

	return (value);
}


/* Update all the zones in the part using a small transational memory region.
 * Many of the scatter modes will produce the wrong results
 * without transactional memory
 */
void update_part_small_TM (Part *part, double incoming_deposit)
{
	Zone *zone, *scatter_zone;
	double deposit_ratio, remaining_deposit, deposit;
	long scale_count, scatter_count, extra_index;

	/* Update count of updates for this part (for error checking)
	 * Just part 0's count will be zeroed regularly.   Others may wrap.
	 */
	part->update_count++;

	/* Get the deposit_ratio from part*/
	deposit_ratio = part->deposit_ratio;

	/* Initially, the remaining_deposit is the incoming deposit */
	remaining_deposit = incoming_deposit;

	/* If have the most common case (original case) where CLOMP_flopScale = 1,
	 * use a specialized loop to get best performance for this important case.
	 * (Since the faster the loop, the more OpenMP overhead matters.)
	 */
	if (CLOMP_flopScale == 1)
	{
	/* Run through each zone, depositing 'deposit_ratio' part of the
	 * remaining_deposit in the zone and carrying the rest to the remaining
	 * zones
	 */
	for (zone = part->firstZone; zone != NULL; zone = zone->nextZone)
	{
	    for (scatter_count = 0; scatter_count < CLOMP_scatter;
		 scatter_count ++)
	    {
		/* Determine the zone to deposit into from this zone*/
		scatter_zone = zone->scatterZones[scatter_count];

		/* Initialize deposit to 0.0 before calculation below
		 * that so results in zero as an answer
		 */
		deposit = 0.0;

		/* Read from 'extra values' in zone to pull all of zone
		 * into caches, posibly displacing other cache lines
		 * Assumes 64 byte cache line or larger.
		 * All these extra values are initialized to 0.0 to prevent
		 * changing answer.
		 */
		for (extra_index = CLOMP_startExtraValues;
		     extra_index < CLOMP_numExtraValues;
		     extra_index += CALC_STRIDE)
		{
#ifdef ENABLE_SIMD
		    deposit +=
			CALCZERO(scatter_zone->extraValues[extra_index]);
#else
		    deposit =
			CALCZERO((scatter_zone->extraValues[extra_index]+
				  deposit));
#endif
		}

		/* Calculate the real deposit for this zone (zero incoming)*/
		deposit += remaining_deposit * deposit_ratio;

		/* In most scatter modes, this value update can cause
		 * race conditions and transational memory is needed
		 * to prevent the wrong value from being written
		 */
#ifdef NOTM
#elif RTM
TM_BEGIN();
#endif
		{
		    /* Add deposit to the zone's value */
		    scatter_zone->value += deposit;
		}
#ifdef NOTM
#elif RTM
TM_END();
#endif
		/* Self scrub specIds, if compiled with -DSELF_SCRUB */
		do_small_self_scrub();

		/* Remove deposit from the remaining_deposit */
		remaining_deposit -= deposit;
	    }
	}
	}

	/* Otherwise, if CLOMP_flopScale != 1, use inner loop version */
	else
	{
	/* Run through each zone, depositing 'deposit_ratio' part of the
	 * remaining_deposit in the zone and carrying the rest to the remaining
	 * zones
	 */
	for (zone = part->firstZone; zone != NULL; zone = zone->nextZone)
	{
	    for (scatter_count = 0; scatter_count < CLOMP_scatter;
		 scatter_count ++)
	    {
		/* Aggregate deposit then do transaction */
				double net_deposit = 0.0;

		/* Determine the zone to deposit into from this zone*/
		scatter_zone = zone->scatterZones[scatter_count];

		/* Allow scaling of the flops per double loaded, so that you
		 * can get expensive iterations without blowing the cache.
		 */
		for (scale_count = 0; scale_count < CLOMP_flopScale; scale_count++)
		{
		    /* Initialize deposit to 0.0 before calculation below
		     * that so results in zero as an answer
		     */
		    deposit = 0.0;

		    /* Read from 'extra values' in zone to pull all of zone
		     * into caches, posibly displacing other cache lines
		     * Assumes 64 byte cache line or larger.
		     * All these extra values are initialized to 0.0 to prevent
		     * changing answer.
		     */
		    for (extra_index = CLOMP_startExtraValues;
			 extra_index < CLOMP_numExtraValues;
			 extra_index += CALC_STRIDE)
		    {
#ifdef ENABLE_SIMD
			deposit +=
			    CALCZERO(scatter_zone->extraValues[extra_index]);
#else
			deposit =
			    CALCZERO((scatter_zone->extraValues[extra_index]+
				      deposit));
#endif
		    }

		    /* Calculate the real deposit for this zone (zero incoming)*/
		    deposit += remaining_deposit * deposit_ratio;

		    /* Use local variable to calculate net deposit */
					net_deposit += deposit;

					/* Remove deposit from the remaining_deposit */
					remaining_deposit -= deposit;
		}

		/* In most scatter modes, this value update can cause
		 * race conditions and transational memory is needed
		 * to prevent the wrong value from being written
		 *
		 * Pull out of loop so flops per zone can be changed
		 * independently from the number of transcations.
		 */
#ifdef NOTM
#elif RTM
TM_BEGIN();
#endif
		{
		    /* Add deposit to the zone's value */
		    scatter_zone->value += net_deposit;
		}
#ifdef NOTM
#elif RTM
TM_END();
#endif

		/* Self scrub specIds, if compiled with -DSELF_SCRUB */
		do_small_self_scrub();
	    }
	}
	}

	/* Put the left over deposit in the Part's residue field */
	part->residue = remaining_deposit;
}


/* Update all the zones in the part using a large transational memory region.
 * Many of the scatter modes will produce the wrong results
 * without atomic/critical/transactional memory
 */
void update_part_large_TM (Part *part, double incoming_deposit)
{
	Zone *zone, *scatter_zone;
	double deposit_ratio, remaining_deposit, deposit;
	long scale_count, scatter_count, extra_index;

	/* Update count of updates for this part (for error checking)
	 * Just part 0's count will be zeroed regularly.   Others may wrap.
	 */
	part->update_count++;

	/* Get the deposit_ratio from part*/
	deposit_ratio = part->deposit_ratio;

	/* Initially, the remaining_deposit is the incoming deposit */
	remaining_deposit = incoming_deposit;

	/* If have the most common case (original case) where CLOMP_flopScale = 1,
	 * use a specialized loop to get best performance for this important case.
	 * (Since the faster the loop, the more OpenMP overhead matters.)
	 */
	if (CLOMP_flopScale == 1)
	{
	/* Run through each zone, depositing 'deposit_ratio' part of the
	 * remaining_deposit in the zone and carrying the rest to the remaining
	 * zones
	 */
	for (zone = part->firstZone; zone != NULL; zone = zone->nextZone)
	{
#ifdef NOTM
#elif RTM
TM_BEGIN();
#endif
	  {
	    for (scatter_count = 0; scatter_count < CLOMP_scatter;
		 scatter_count ++)
	    {
		/* Determine the zone to deposit into from this zone*/
		scatter_zone = zone->scatterZones[scatter_count];

		/* Initialize deposit to 0.0 before calculation below
		 * that so results in zero as an answer
		 */
		deposit = 0.0;

		/* Read from 'extra values' in zone to pull all of zone
		 * into caches, posibly displacing other cache lines
		 * Assumes 64 byte cache line or larger.
		 * All these extra values are initialized to 0.0 to prevent
		 * changing answer.
		 */
		for (extra_index = CLOMP_startExtraValues;
		     extra_index < CLOMP_numExtraValues;
		     extra_index += CALC_STRIDE)
		{
#ifdef ENABLE_SIMD
		    deposit +=
			CALCZERO(scatter_zone->extraValues[extra_index]);
#else
		    deposit =
			CALCZERO((scatter_zone->extraValues[extra_index]+
				  deposit));
#endif
		}

		/* Calculate the real deposit for this zone (zero incoming)*/
		deposit += remaining_deposit * deposit_ratio;

		/* In most scatter modes, this value update can cause
		 * race conditions and transational memory is needed
		 * to prevent the wrong value from being written
		 */
		{
		    /* Add deposit to the zone's value */
		    scatter_zone->value += deposit;
		}

		/* Remove deposit from the remaining_deposit */
		remaining_deposit -= deposit;
	    }
	  }
#ifdef NOTM
#elif RTM
TM_END();
#endif

	  /* Self scrub specIds, if compiled with -DSELF_SCRUB */
	  do_large_self_scrub();
	}
	}

	/* Otherwise, if CLOMP_flopScale != 1, use inner loop version */
	else
	{
	/* Run through each zone, depositing 'deposit_ratio' part of the
	 * remaining_deposit in the zone and carrying the rest to the remaining
	 * zones
	 */
	for (zone = part->firstZone; zone != NULL; zone = zone->nextZone)
	{
#ifdef NOTM
#elif RTM
TM_BEGIN();
#endif
	  {
	    for (scatter_count = 0; scatter_count < CLOMP_scatter;
		 scatter_count ++)
	    {
		/* Aggregate deposit then do transaction */
				double net_deposit = 0.0;

		/* Determine the zone to deposit into from this zone*/
		scatter_zone = zone->scatterZones[scatter_count];

		/* Allow scaling of the flops per double loaded, so that you
		 * can get expensive iterations without blowing the cache.
		 */
		for (scale_count = 0; scale_count < CLOMP_flopScale; scale_count++)
		{
		    /* Initialize deposit to 0.0 before calculation below
		     * that so results in zero as an answer
		     */
		    deposit = 0.0;

		    /* Read from 'extra values' in zone to pull all of zone
		     * into caches, posibly displacing other cache lines
		     * Assumes 64 byte cache line or larger.
		     * All these extra values are initialized to 0.0 to prevent
		     * changing answer.
		     */
		    for (extra_index = CLOMP_startExtraValues;
			 extra_index < CLOMP_numExtraValues;
			 extra_index += CALC_STRIDE)
		    {
#ifdef ENABLE_SIMD
			deposit +=
			    CALCZERO(scatter_zone->extraValues[extra_index]);
#else
			deposit =
			    CALCZERO((scatter_zone->extraValues[extra_index]+
				      deposit));
#endif
		    }

		    /* Calculate the real deposit for this zone (zero incoming)*/
		    deposit += remaining_deposit * deposit_ratio;


		    /* Use local variable to calculate net deposit */
					net_deposit += deposit;

					/* Remove deposit from the remaining_deposit */
					remaining_deposit -= deposit;
		}

		/* In most scatter modes, this value update can cause
		 * race conditions and transational memory is needed
		 * to prevent the wrong value from being written
		 *
		 * Pull out of loop so flops per zone can be changed
		 * independently from the number of transcations.
		 */
		{
		    /* Add deposit to the zone's value */
		    scatter_zone->value += net_deposit;
		}
	    }
	  }
#ifdef NOTM
#elif RTM
TM_END();
#endif
	  /* Self scrub specIds, if compiled with -DSELF_SCRUB */
	  do_large_self_scrub();
	}
	}

	/* Put the left over deposit in the Part's residue field */
	part->residue = remaining_deposit;
}

/* Must be indentical to update_part_small_TM above except for the
 * the transactional memory pragmas commented out!!!
 * Used to calculate best case update speed so that
 * transactional memory overhead can be calculated.
 */
void update_part_no_TM (Part *part, double incoming_deposit)
{
	Zone *zone, *scatter_zone;
	double deposit_ratio, remaining_deposit, deposit;
	long scale_count, scatter_count, extra_index;

	/* Update count of updates for this part (for error checking)
	 * Just part 0's count will be zeroed regularly.   Others may wrap.
	 */
	part->update_count++;

	/* Get the deposit_ratio from part*/
	deposit_ratio = part->deposit_ratio;

	/* Initially, the remaining_deposit is the incoming deposit */
	remaining_deposit = incoming_deposit;

	/* If have the most common case (original case) where CLOMP_flopScale = 1,
	 * use a specialized loop to get best performance for this important case.
	 * (Since the faster the loop, the more OpenMP overhead matters.)
	 */
	if (CLOMP_flopScale == 1)
	{
	/* Run through each zone, depositing 'deposit_ratio' part of the
	 * remaining_deposit in the zone and carrying the rest to the remaining
	 * zones
	 */
	for (zone = part->firstZone; zone != NULL; zone = zone->nextZone)
	{
	    for (scatter_count = 0; scatter_count < CLOMP_scatter;
		 scatter_count ++)
	    {
		/* Determine the zone to deposit into from this zone*/
		scatter_zone = zone->scatterZones[scatter_count];

		/* Initialize deposit to 0.0 before calculation below
		 * that so results in zero as an answer
		 */
		deposit = 0.0;

		/* Read from 'extra values' in zone to pull all of zone
		 * into caches, posibly displacing other cache lines
		 * Assumes 64 byte cache line or larger.
		 * All these extra values are initialized to 0.0 to prevent
		 * changing answer.
		 */
		for (extra_index = CLOMP_startExtraValues;
		     extra_index < CLOMP_numExtraValues;
		     extra_index += CALC_STRIDE)
		{
#ifdef ENABLE_SIMD
		    deposit +=
			CALCZERO(scatter_zone->extraValues[extra_index]);
#else
		    deposit =
			CALCZERO((scatter_zone->extraValues[extra_index]+
				  deposit));
#endif
		}

		/* Calculate the real deposit for this zone (zero incoming)*/
		deposit += remaining_deposit * deposit_ratio;

		/* In most scatter modes, this value update can cause
		 * race conditions and transational memory is needed
		 * to prevent the wrong value from being written
		 */
/* NOT FOR NO_TM VERSION #pragma tm atomic default(trans) */
		{
		    /* Add deposit to the zone's value */
		    scatter_zone->value += deposit;
		}

		/* Remove deposit from the remaining_deposit */
		remaining_deposit -= deposit;
	    }
	}
	}

	/* Otherwise, if CLOMP_flopScale != 1, use inner loop version */
	else
	{
	/* Run through each zone, depositing 'deposit_ratio' part of the
	 * remaining_deposit in the zone and carrying the rest to the remaining
	 * zones
	 */
	for (zone = part->firstZone; zone != NULL; zone = zone->nextZone)
	{
	    for (scatter_count = 0; scatter_count < CLOMP_scatter;
		 scatter_count ++)
	    {
		/* Aggregate deposit then do transaction */
				double net_deposit = 0.0;

		/* Determine the zone to deposit into from this zone*/
		scatter_zone = zone->scatterZones[scatter_count];

		/* Allow scaling of the flops per double loaded, so that you
		 * can get expensive iterations without blowing the cache.
		 */
		for (scale_count = 0; scale_count < CLOMP_flopScale; scale_count++)
		{
		    /* Initialize deposit to 0.0 before calculation below
		     * that so results in zero as an answer
		     */
		    deposit = 0.0;

		    /* Read from 'extra values' in zone to pull all of zone
		     * into caches, posibly displacing other cache lines
		     * Assumes 64 byte cache line or larger.
		     * All these extra values are initialized to 0.0 to prevent
		     * changing answer.
		     */
		    for (extra_index = CLOMP_startExtraValues;
			 extra_index < CLOMP_numExtraValues;
			 extra_index += CALC_STRIDE)
		    {
#ifdef ENABLE_SIMD
			deposit +=
			    CALCZERO(scatter_zone->extraValues[extra_index]);
#else
			deposit =
			    CALCZERO((scatter_zone->extraValues[extra_index]+
				      deposit));
#endif
		    }

		    /* Calculate the real deposit for this zone (zero incoming)*/
		    deposit += remaining_deposit * deposit_ratio;

		    /* Use local variable to calculate net deposit */
					net_deposit += deposit;

					/* Remove deposit from the remaining_deposit */
					remaining_deposit -= deposit;
		}

		/* In most scatter modes, this value update can cause
		 * race conditions and transational memory is needed
		 * to prevent the wrong value from being written
		 *
		 * Pull out of loop so flops per zone can be changed
		 * independently from the number of transcations.
		 */
/* NOT FOR NO_TM VERSION #pragma tm atomic default(trans) */
		{
		    /* Add deposit to the zone's value */
		    scatter_zone->value += net_deposit;
		}
	    }
	}
	}

	/* Put the left over deposit in the Part's residue field */
	part->residue = remaining_deposit;
}

/* Resets parts to initial state and warms up cache */
void reinitialize_parts()
{
	long pidx;
	Zone *zone;

	/* Reset all the zone values to 0.0 and the part residue to 0.0 */
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	{
	for (zone = partArray[pidx]->firstZone;
	     zone != NULL;
	     zone = zone->nextZone)
	{
	    /* Reset zone's value to 0 */
	    zone->value = 0.0;
	}

	/* Reset residue */
	partArray[pidx]->residue = 0.0;

	/* Reset update count */
	partArray[pidx]->update_count = 0;
	}

	/* Scan through zones and add zero to each zone to warm up cache*/
	/* Also sets each zone update_count to 1, which sanity check wants */
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	{
	update_part_no_TM (partArray[pidx], 0.0);
	}
}

/* Helper routine that prints a line of the benchmark pseudocode
 * prefixed with the description
 */
void print_pseudocode (const char *desc, const char *pseudocode)
{
	printf ("%13s:| %s\n", desc, pseudocode);
}

/* Check data for consistency and print out a one line data stat summary
 * that should help tell if data is all right  (total should be number
 * of iterations run).
 */
void print_data_stats (const char *desc)
{
	double value_sum, residue_sum, last_value, dtotal;
	double zone_value, expected_value;
	double residue, expected_residue;
	double error_bound;
	long pidx;
	Zone *zone;
	int is_reference, error_count;

	/* Initialize value and residue sums to zero */
	value_sum = 0.0;
	residue_sum = 0.0;

	/* Use "Serial Ref" as the reference calculation and check all
	 * values against that.  Since the same code is used to calculate
	 * all these values, we should not have any issues with reordering
	 * calculations causing small differences.
	 */
	if (strcmp (desc, "Serial Ref") == 0)
	is_reference = 1;
	else
	is_reference = 0;

	/* Initialize count of check errors */
	error_count = 0;

	/* Scan through each part, check that values decrease monotonically
	 * and sum up all the values.  Also check that the part residue and
	 * the part's first zone's value are the expected value.
	 */
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	{
	/* If have reference calculation, grab the first zone's value
	 * and the part residue for comparison later
	 */
	if (is_reference)
	{
	    partArray[pidx]->expected_first_value =
		partArray[pidx]->firstZone->value;
	    partArray[pidx]->expected_residue = partArray[pidx]->residue;
	}

	/* Otherwise, make sure this part matches the expected values
	 * from the reference calculation.  With many of the scatter modes,
	 * the order of update for each zone is semi-random, so rounding errors
	 * will occur.   Will use calculated error bounds (starting with
	 * the tightest and if that is to tight, go to the 'max' one.
	 * -JCG 1/6/11
	 */
	else
	{
	    /* Check that first zone's value is what is expected
	     * (within calculated error bounds,
	     * since we now can have semi random ordering of processes)
	     * NOTE: this will only detect the first zone's errors.
	     */
	    zone_value = partArray[pidx]->firstZone->value;
	    expected_value = partArray[pidx]->expected_first_value;
	    /* Start with tightest error bound, change if too tight */
	    error_bound = CLOMP_tightest_error_bound;
	    if (((zone_value - error_bound) > expected_value) ||
		((zone_value + error_bound) < expected_value))
	    {
		error_count++;
		fprintf (stderr,
			 "%s check failure: part %i first zone value (%E) off by %E!\n",
			 desc, (int) pidx, zone_value,
			 (expected_value - zone_value));
	    }
	    /* This calculation has fixed order with same routine but
	     * clever inlining can cause different optimizations of sum
	     * for different testcases (seen with gcc).
	     */
	    residue = partArray[pidx]->residue;
	    expected_residue = partArray[pidx]->expected_residue;
	    /* Start with tightest error bound, change if too tight */
	    error_bound = CLOMP_tightest_error_bound;
	    if (((residue - error_bound) > expected_residue) ||
		((residue + error_bound) < expected_residue))
	    {
		error_count++;
		fprintf(stderr,
			"%s check failure: part %i residue (%E) off by %E\n",
			desc, (int)pidx, expected_residue,
			(expected_residue- residue));
	    }
	}

	/* Use first zone's value as initial last_value */
	last_value = partArray[pidx]->firstZone->value;

	/* Sum up values to make sure updates were not lost */
	for (zone = partArray[pidx]->firstZone;
	     zone != NULL;
	     zone = zone->nextZone)
	{
	    /* Sum up values */
	    value_sum += zone->value;

	    /* This value now is last_value */
	    last_value = zone->value;
	}

	/* Sum up part residue's */
	residue_sum += partArray[pidx]->residue;
	}

	/* Calculate the total of value_sum + residue_sum.  This should
	 * equal the number of subcycles run (due to construction of benchmark)
	 */
	dtotal = value_sum + residue_sum;

	/* Start with tightest error bound, change if too tight */
	/*error_bound = CLOMP_tightest_error_bound;  */
	/* Now use looser error bound that only detects missed update */
	error_bound = CLOMP_error_bound;

	/* Sanity check, use calculated error bounds before alert */
	if (((dtotal + error_bound) < ((double)CLOMP_num_iterations * 10.0)) ||
	((dtotal - error_bound) > ((double)CLOMP_num_iterations * 10.0)))
	{
	fprintf (stderr,
		 "*** %s check failure:  Total (%-.15g) != Expected (%.15g) (~%.0f zone updates missing)\n",
		 desc, dtotal, ((double)CLOMP_num_iterations * 10.0),
				 (fabs(((double)CLOMP_num_iterations * 10.0) - dtotal)/CLOMP_error_bound));
	error_count++;
	}

	/* Start with tightest error bound, change if too tight */
	error_bound = CLOMP_tightest_error_bound;

	/* Sanity check, residue must be within reasonable bounds */
	if ((residue_sum < 0.0) ||
	(residue_sum > (CLOMP_max_residue + error_bound)))
	{
	fprintf (stderr,
		 "*** %s check failure: Residue (%-.15g) outside bounds 0 - %.15g\n",
		 desc, residue_sum, CLOMP_max_residue);
	error_count++;
	}

	/* Make sure part 0's update count is exactly one.  This detects
	 * illegal optimization of calc_deposit().
	 */
	if (partArray[0]->update_count != 1)
	{
	fprintf (stderr, "Error in calc_deposit: Part updated %i times since last calc_deposit!\n",
		 (int) partArray[0]->update_count);
	fprintf (stderr, "Benchmark designed to have calc_deposit called exactly once per update!\n");
	fprintf (stderr, "Critical error: Exiting...\n");
	exit (1);
	}

	if (error_count > 0)
	{
	fprintf (stderr,
		 "ERROR: %i check failures detected in '%s' data. Exiting...\n",
		 error_count, desc);
	exit (1);
	}

	/* Print out check text so results can be visually inspected */
	printf ("%13s Checksum: Sum=%-8.8g Residue=%-8.8g Total=%-.9g\n",
	    desc, value_sum, residue_sum, dtotal);


}

/* Calculates the amount to deposit in each part this subcycle.
 * Based on the residue of all parts from last subcycle.
 * Normally, this would be done with some sort of MPI exchange of
 * data but here we are just using info from local zones.
 *
 * Should be used by all subcycles to calculate amount to deposit.
 *
 * Cannot be moved past subcycle loops without getting wrong answer.
 */
double calc_deposit ()
{
	double residue, deposit;
	long pidx;

	/* Sanity check, make sure residues have be updated since last calculation
	 * This code cannot be pulled out of loops or above loops!
	 * Only check/update part 0 (other counts will just continue and may wrap)
	 */
	if (partArray[0]->update_count != 1)
	{
	fprintf (stderr, "Error in calc_deposit: Part updated %i times since last call!\n",
		 (int) partArray[0]->update_count);
	fprintf (stderr, "Benchmark designed to have calc_deposit called exactly once per update!\n");
	fprintf (stderr, "Critical error: Exiting...\n");
	exit (1);
	}

	/* Mark that we are using the updated info, so we can detect if this
	 * code has been moved illegally.
	 */
	partArray[0]->update_count = 0;

	/* Calculate residue from previous subcycle (normally this is done
	 * with an MPI data exchange to other domains, but emulate here).
	 */
	residue = 0.0;
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	{
	residue += partArray[pidx]->residue;
	}

	/* Calculate deposit for this subcycle based on residue and part ratio */
	deposit = (1.0 + residue) * CLOMP_partRatio;

	/* Return the amount to deposit in each part this subcycle */
	return (deposit);
}

/*
 * --------------------------------------------------------------------
 * Variation: Serial Ref
 * --------------------------------------------------------------------
 */

/* Do module one's work serially (contains 1 subcycle) */
void serial_ref_module1(void (*update_part_ptr)(Part *, double))
{
    double deposit;
    long pidx;

    /* ---------------- SUBCYCLE 1 OF 1 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
}

/* Do module two's work serially (contains 2 subcycles) */
void serial_ref_module2(void (*update_part_ptr)(Part *, double))
{
    double deposit;
    long pidx;

    /* ---------------- SUBCYCLE 1 OF 2 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);

    /* ---------------- SUBCYCLE 2 OF 2 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
}

/* Do module three's work serially (contains 3 subcycles) */
void serial_ref_module3(void (*update_part_ptr)(Part *, double))
{
    double deposit;
    long pidx;

    /* ---------------- SUBCYCLE 1 OF 3 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);

    /* ---------------- SUBCYCLE 2 OF 3 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);

    /* ---------------- SUBCYCLE 3 OF 3 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
}

/* Do module four's work serially (contains 4 subcycles) */
void serial_ref_module4(void (*update_part_ptr)(Part *, double))
{
    double deposit;
    long pidx;

    /* ---------------- SUBCYCLE 1 OF 4 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);

    /* ---------------- SUBCYCLE 2 OF 4 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)

	update_part_ptr (partArray[pidx], deposit);

    /* ---------------- SUBCYCLE 3 OF 4 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);

    /* ---------------- SUBCYCLE 4 OF 4 ----------------- */

    /* Calculate deposit for this subcycle based on last subcycle's residue */
    deposit = calc_deposit ();

    /* Scan through zones and add appropriate deposit to each zone */
    for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
}


/* Do one cycle (10 subcycles) serially, no OpenMP */
void serial_ref_cycle()
{
    /* Emulate calls to 4 different packages, do 10 subcycles total */
    serial_ref_module1(update_part_no_TM);
    serial_ref_module2(update_part_no_TM);
    serial_ref_module3(update_part_no_TM);
    serial_ref_module4(update_part_no_TM);
}

/* Do all the cycles (10 subcycles/cycle) serially, no OpenMP */
void do_serial_ref_version()
{
    long iteration;

    /* Do the specified number of iterations */
    for (iteration = 0; iteration < CLOMP_num_iterations; iteration ++)
	serial_ref_cycle();
}

/*
 * --------------------------------------------------------------------
 * Generic static OMP loop with configurable update_part
 * --------------------------------------------------------------------
 */

/* Do module one's work using "omp parallel for schedule(static)"
 * (contains 1 subcycle)
 */
void generic_module1(void (*update_part_ptr)(Part *, double))
{
	double deposit;
	long pidx;

	/* ---------------- SUBCYCLE 1 OF 1 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
#ifdef RTM
TM_THREAD_ENTER();
#endif

	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif
#ifdef RTM
TM_THREAD_EXIT();
#endif
	}
}

/* Do module two's work using "omp parallel for schedule(static)"
 * (contains 2 subcycles)
 */
void generic_module2(void (*update_part_ptr)(Part *, double))
{
	double deposit;
	long pidx;

	/* ---------------- SUBCYCLE 1 OF 2 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif

#ifdef RTM
TM_THREAD_EXIT();
#endif
	}

	/* ---------------- SUBCYCLE 2 OF 2 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif

	#ifdef RTM
	TM_THREAD_EXIT();
	#endif
	}
}

/* Do module three's work using "omp parallel for schedule(static)"
 * (contains 3 subcycles)
 */
void generic_module3(void (*update_part_ptr)(Part *, double))
{
	double deposit;
	long pidx;

	/* ---------------- SUBCYCLE 1 OF 3 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif
	#ifdef RTM
	TM_THREAD_EXIT();
	#endif
	}
	/* ---------------- SUBCYCLE 2 OF 3 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif

	#ifdef RTM
	TM_THREAD_EXIT();
	#endif
	}

	/* ---------------- SUBCYCLE 3 OF 3 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif
	#ifdef RTM
	TM_THREAD_EXIT();
	#endif
	}
}

/* Do module four's work using "omp parallel for schedule(static)"
 * (contains 4 subcycles)
 */
void generic_module4(void (*update_part_ptr)(Part *, double))
{
	double deposit;
	long pidx;

	/* ---------------- SUBCYCLE 1 OF 4 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif

	#ifdef RTM
	TM_THREAD_EXIT();
	#endif
	}
	/* ---------------- SUBCYCLE 2 OF 4 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif

#ifdef RTM
TM_THREAD_EXIT();
#endif
	}
	/* ---------------- SUBCYCLE 3 OF 4 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif
#ifdef RTM
TM_THREAD_EXIT();
#endif
	}
	/* ---------------- SUBCYCLE 4 OF 4 ----------------- */
#ifdef RESET_MODE
	/* Try resetting the speculation mode before every openMP loop to measure overhead */
	reset_speculation_mode();
#endif

	/* Calculate deposit for this subcycle based on last subcycle's residue */
	deposit = calc_deposit ();

	/* Scan through zones and add appropriate deposit to each zone */
#pragma omp parallel
	{
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for private (pidx) schedule(static)
	for (pidx = 0; pidx < CLOMP_numParts; pidx++)
	update_part_ptr (partArray[pidx], deposit);
#ifdef STM
#elif NOTM
#else
  #ifdef HTM_REPORT
	tm_print_stats();
  #endif
#endif
	#ifdef RTM
	TM_THREAD_EXIT();
	#endif
	}
}

/*
 * --------------------------------------------------------------------
 * Variation: Static OpenMP with Small TM region
 *            Uses generic static OpenMP module for heavy lifting
 * --------------------------------------------------------------------
 */
/* Do one cycle (10 subcycles) using "omp parallel for schedule(static)" */
void small_tm_cycle()
{
	/* Emulate calls to 4 different packages, do 10 subcycles total */
	generic_module1(update_part_small_TM);
	generic_module2(update_part_small_TM);
	generic_module3(update_part_small_TM);
	generic_module4(update_part_small_TM);
}

/* Do all the cycles (10 subcycles/cycle) using
 * "omp parallel for schedule(static)"
 */
void do_small_tm_version()
{
	long iteration;

	/* Do the specified number of iterations */
	for (iteration = 0; iteration < CLOMP_num_iterations; iteration ++)
	{
	small_tm_cycle();
	}
}


/*
 * --------------------------------------------------------------------
 * Variation: Static OpenMP with Big TM region
 *            Uses generic static OpenMP module for heavy lifting
 * --------------------------------------------------------------------
 */
/* Do one cycle (10 subcycles) using "omp parallel for schedule(static)" */
void large_tm_cycle()
{
	/* Emulate calls to 4 different packages, do 10 subcycles total */
	generic_module1(update_part_large_TM);
	generic_module2(update_part_large_TM);
	generic_module3(update_part_large_TM);
	generic_module4(update_part_large_TM);
}

/* Do all the cycles (10 subcycles/cycle) using
 * "omp parallel for schedule(static)"
 */
void do_large_tm_version()
{
	long iteration;

	/* Do the specified number of iterations */
	for (iteration = 0; iteration < CLOMP_num_iterations; iteration ++)
	{
	large_tm_cycle();
	}
}

/* Helper, add part passed in to the partArray at partId and initialize
 * it.  The separate routine is used to make it easy to allocate parts
 * with various strategies (such as each thread allocating it's parts).
 * The partArray has to be allocated by one thread but it is not
 * modified during the run.
 */
void addPart (Part *part, long partId)
{
	/* Sanity check, make sure partId valid */
	if ((partId < 0) || (partId >= CLOMP_numParts))
	{
	fprintf (stderr, "addPart error: partId (%i) out of bounds!\n", (int)partId);
	exit (1);
	}


	/* Sanity check, make sure part not already added! */
	if (partArray[partId] != NULL)
	{
	fprintf (stderr, "addPart error: partId (%i) already initialized!\n",
		 (int) partId);
	exit (1);
	}

	/* Put part pointer in array */
	partArray[partId] = part;

	/* Set partId */
	part->partId = partId;

	/* Set zone count for part (now fixed, used to be variable */
	part->zoneCount = CLOMP_zonesPerPart;


	/* Updated June 2010 by John Gyllenhaal to pick a deposit ratio
	 * for this part that keeps the math from underflowing and
	 * makes it possible to come up with a sane error bounds.
	 * This math was picked experimentally to come up with
	 * relatively large error bounds for the 'interesting testcase' inputs.
	 *
	 * This is part of an effort to separate rounding error due
	 * to inlining update_part() (and optimizing different ways) from
	 * incorrect results due to races or bad hardware.
	 *
	 * The previous deposit_ratio only really worked well for 100 zones.
	 */
	 part->deposit_ratio=((double)((1.5*(double)CLOMP_numParts)+partId))/
	   ((double)(CLOMP_zonesPerPart*CLOMP_numParts*
		 CLOMP_scatter*CLOMP_flopScale));


	/* Initially no residue from previous passes */
	part->residue = 0.0;

	/* Initially, no zones attached to part */
	part->firstZone = NULL;
	part->lastZone = NULL;

	/* Initially, don't know expected values (used for checking */
	part->expected_first_value = -1.0;
	part->expected_residue = -1.0;

	/* Initially, no zone array allocated */
	part->zoneArray = NULL;
}

/* Appends zone to the part identified by partId.   Done in separate routine
 * to facilitate the zones being allocated in various ways (such as by
 * different threads, randomly, etc.)
 */
void addZone (Part *part, Zone *zone, Zone **scatterZones)
{
	int i;

	/* Sanity check, make sure not NULL */
	if (part == NULL)
	{
	fprintf (stderr, "addZone error: part NULL!\n");
	exit (1);
	}

	/* Sanity check, make sure zone not NULL */
	if (zone == NULL)
	{
	fprintf (stderr, "addZone error: zone NULL!\n");
	exit (1);
	}

	/* Sanity check, make sure scatterZones not NULL */
	if (scatterZones == NULL)
	{
	fprintf (stderr, "addZone error: scatterZones NULL!\n");
	exit (1);
	}

	/* Touch/initialize all of zone to force all memory to be really
	 * allocated (CLOMP_zoneSize is often bigger than the portion of the
	 * zone we use)
	 */
	memset (zone, 0xFF, CLOMP_zoneSize);


	/* If not existing zones, place at head of list */
	if (part->lastZone == NULL)
	{
	/* Give first zone a zoneId of 0! (was 1 -JCG 12/20/11) */
	zone->zoneId = 0;

	/* First and last zone */
	part->firstZone = zone;
	part->lastZone = zone;

	}

	/* Otherwise, put after last zone */
	else
	{
	/* Give this zone the last Zone's id + 1 */
	zone->zoneId = part->lastZone->zoneId + 1;

	part->lastZone->nextZone = zone;
	part->lastZone = zone;
	}

	/* Always placed at end */
	zone->nextZone = NULL;

	/* Point zone's scatterZones array to this array */
	zone->scatterZones = scatterZones;

	/* Initialize scatterZones to NULL */
	for (i=0; i < CLOMP_scatter; i++)
	scatterZones[i] = NULL;

	/* Initalize the rest of the zone fields */
	zone->partId = part->partId;
	zone->value = 0.0;

	for (i=0; i < CLOMP_numExtraValues; i++)
	zone->extraValues[i] = 0.0;

}

/* Returns a Zone pointer from the zoneArray passed in.
 * We are using variable sized zones (set by CLOMP_zoneSize)
 * so we cannot use simplely zoneArray[zoneId]. -JCG 4Mar2011
 */
Zone *arrayToZone(Zone *zoneArray, int zoneId)
{
	Zone *zone;

	/* Sanity check, must be between 0 and CLOMP_zonesPerPart */
	if (zoneId < 0)
	{
	printf ("*** arrayToZone: zoneId (%i) < 0\n", zoneId);
	exit (1);
	}
	if (zoneId >= CLOMP_zonesPerPart)
	{
	printf ("*** arrayToZone: zoneId (%i) > %li (zonesPerPart)\n", zoneId,
		CLOMP_zonesPerPart);
	exit (1);
	}

	/* Get the zone from the array */
	zone = (Zone *)(((char *)zoneArray) + (zoneId * CLOMP_zoneSize));

	return (zone);
}

/* Calculate target zone for NONE scatter mode
 * Just update the currentZone repeatedly each time.
 * No races, so atomic updates not needed.
 */
Zone *scatterNONEzone(long partId, long zoneId, long scatterId, long partIdMod)
{
	Zone *targetZone;
	long usePartId = partId;

	/* Apply PartIdMod if needed */
	if (usePartId >= partIdMod)
	usePartId %= partIdMod;

	/* Get target zone */
	targetZone = arrayToZone(partArray[usePartId]->zoneArray,zoneId);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}


/* Calculate target zone for ADJACENT scatter mode
 * This will cause race conditions except for scatter == 1.
 * Thus, will need atomic updates to guarentee correct answer.
 */
Zone *scatterADJACENTzone(long partId, long zoneId, long scatterId,
			  long partIdMod)
{
	Zone *targetZone;

	/* Use zones at same zoneId from partIds adjacent to
	 * the current part (including the current part).
	 * For scatterId == 0, uses parts  above current part
	 * and moving down as scatterId increases.
	 */
	int usePartId = (partId - (CLOMP_scatter/2)) + scatterId;

	/* If have gone negative in our calculations, wrap around.
	 * Using while loop to support huge scatter values
	 */
	while (usePartId < 0)
	usePartId += CLOMP_numParts;

	/* If go off end in our calculations, wrap around */
	while (usePartId >= CLOMP_numParts)
	usePartId -= CLOMP_numParts;

	/* Apply PartIdMod if needed */
	if (usePartId >= partIdMod)
	usePartId %= partIdMod;

	/* Get target zone */
	targetZone = arrayToZone(partArray[usePartId]->zoneArray,zoneId);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}

/* Calculate target zone for RANDOM scatter mode
 * For 'Random', use a deterministic pseudo
 * random number generator to pick a "random" zone each time.
 * The "random" order is designed to be the same on
 * every machine (for the same part/zone configuration).
 * This should defeat stride-based prefetch engines.
 * Thus, will need atomic updates to guarentee correct answer.
 */
Zone *scatterRANDOMzone(long partId, long zoneId, long scatterId,
			long partIdMod)
{
	Zone *targetZone;
	long randPartId, randZoneId;

	/* Pick a "random" partId*/
	randPartId = rand_small_int(CLOMP_numParts-1);

	/* Apply PartIdMod if needed */
	if (randPartId >= partIdMod)
	randPartId %= partIdMod;

	/* Pick a "random" zoneId */
	randZoneId = rand_small_int(CLOMP_zonesPerPart-1);

#if 0
	printf ("Part %2i Zone %2i Update %i: Part %2i Zone %2i\n",
	    partId, zoneId, scatterId, randPartId, randZoneId);
#endif

	/* Get the randomly selected zone */
	targetZone = arrayToZone(partArray[randPartId]->zoneArray,
			     randZoneId);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}

/* Calculate target zone for INPART (no races) scatter mode
 * For 'InPart', use a deterministic pseudo
 * random number generator to pick a "random" zone
 * in the current Part each time.
 * The "random" order is designed to be the same on
 * every machine (for the same part/zone configuration).
 * This should defeat stride-based prefetch engines.
 * This should approach should have no races and thus
 * give correct answers without atomic updates.
 */
Zone *scatterINPARTzone(long partId, long zoneId, long scatterId,
			long partIdMod)
{
	Zone *targetZone;
	long  randZoneId;
	long usePartId = partId;

	/* Pick a "random" zoneId */
	randZoneId = rand_small_int(CLOMP_zonesPerPart-1);

#if 0
	printf ("Part %2i Zone %2i Update %i: Part %2i Zone %2i\n",
	    partId, zoneId, scatterId, partId, randZoneId);
#endif

	/* Apply PartIdMod if needed */
	if (usePartId >= partIdMod)
	usePartId %= partIdMod;

	/* Get the randomly selected zone */
	targetZone =
	arrayToZone(partArray[usePartId]->zoneArray,randZoneId);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}


/* Calculate target zone for FIRSTPARTS (forces races) scatter mode
 * Picks zone with same ZoneId from partId scatterId.
 * Thus all parts will be updating the same few parts (hopefully
 * causing a lot of races).
 * Thus, will need atomic updates to guarentee correct answer.
 */
Zone *scatterFIRSTPARTSzone(long partId, long zoneId, long scatterId,
			    long partIdMod)
{
	Zone *targetZone;

	/* Start in Part 'scatterId'*/
	int usePartId = scatterId;

	/* If go off end in our calculations, wrap around
	 * Using while loop to support huge scatter values
	 */
	while (usePartId >= CLOMP_numParts)
	usePartId -= CLOMP_numParts;

	/* Apply PartIdMod if needed */
	if (usePartId >= partIdMod)
	usePartId %= partIdMod;

	targetZone = arrayToZone(partArray[usePartId]->zoneArray,zoneId);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}

/* Calculate target zone for STRIDE1 scatter mode
 * Linearly scan zones in part with stride 1.
 * The same zone's scatter zones will be
 * adjacent zones in the same part.
 */
Zone *scatterSTRIDE1zone(long partId, long zoneId, long scatterId,
			 long partIdMod)
{
	Zone *targetZone;
	long targetZoneId;
	long usePartId = partId;

	/* Use scatterCount to get stride1 pattern */
	targetZoneId = (zoneId * CLOMP_scatter) + scatterId;

	/* If go off the end in our calculations, wrap around */
	while (targetZoneId >= CLOMP_zonesPerPart)
	targetZoneId -= CLOMP_zonesPerPart;

	/* Apply PartIdMod if needed */
	if (usePartId >= partIdMod)
	usePartId %= partIdMod;

	/* Get target zone */
	targetZone = arrayToZone(partArray[usePartId]->zoneArray,targetZoneId);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}


/* Calculate target zone for FIRSTZONE scatter mode
 * Just update the first zone in part repeatedly each time.
 * No races, so atomic updates not needed.
 */
Zone *scatterFIRSTZONEzone(long partId, long zoneId, long scatterId,
			   long partIdMod)
{
	Zone *targetZone;
	long usePartId = partId;

	/* Apply PartIdMod if needed */
	if (usePartId >= partIdMod)
	usePartId %= partIdMod;

	/* Get target zone */
	targetZone = arrayToZone(partArray[usePartId]->zoneArray,0);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}

/* Calculate target zone for RANDFIRSTZONE scatter mode
 * Uses a deterministic pseudo random number generator to
 * pick a "random" part's first zone each time.
 * The "random" order is designed to be the same on
 * every machine.
 * Will need atomic updates to guarentee correct answer.
 */
Zone *scatterRANDFIRSTZONEzone(long partId, long zoneId, long scatterId,
				 long partIdMod)
{
	Zone *targetZone;
	long randPartId;

	/* Pick a "random" partId*/
	randPartId = rand_small_int(CLOMP_numParts-1);

	/* Apply PartIdMod if needed */
	if (randPartId >= partIdMod)
	randPartId %= partIdMod;

	/* Get the randomly selected part's first zone */
	targetZone = arrayToZone(partArray[randPartId]->zoneArray, 0);

	/* Return the target Zone for this scatter mode */
	return (targetZone);
}

/* Case independent string comparision that returns 1 if the match
 * and 0 if they don't
 */
int modeNamesMatch(const char *name1, const char *name2)
{
	if (strcasecmp(name1, name2) == 0)
	return (1);
	else
	return (0);
}

/* Define function pointer for scatterMode functions*/
typedef Zone*(*ScatterModeFuncPtr)(long partId, long zoneId, long scatterId,
				   long partIdMod);

/* Return function pointer to appropriate scatterMode function.
 * Case insenstive string compares used.
 */
ScatterModeFuncPtr getScatterModeFuncPtr(const char *desc,
					 const char *scatterMode)
{
	/* Use case-insensitive compares to find scatter mode */
	if (modeNamesMatch(scatterMode, "NONE"))
	return (scatterNONEzone);
	if (modeNamesMatch(scatterMode, "ADJACENT"))
	return (scatterADJACENTzone);
	if (modeNamesMatch(scatterMode, "RANDOM"))
	return (scatterRANDOMzone);
	if (modeNamesMatch(scatterMode, "INPART"))
	return (scatterINPARTzone);
	if (modeNamesMatch(scatterMode, "FIRSTPARTS"))
	return (scatterFIRSTPARTSzone);
	if (modeNamesMatch(scatterMode, "STRIDE1"))
	return (scatterSTRIDE1zone);
	if (modeNamesMatch(scatterMode, "FIRSTZONE"))
	return (scatterFIRSTZONEzone);
	if (modeNamesMatch(scatterMode, "RANDFIRSTZONE"))
	return (scatterRANDFIRSTZONEzone);

	/* Punt if don't recognize the name */
	fprintf (stderr, "Error: Unknown %s '%s'!\n", desc, scatterMode);
	print_usage();
	exit (1);
}



/*
 * --------------------------------------------------------------------
 * Main driver
 * --------------------------------------------------------------------
 */

int main(int argc, char *argv[]) {
	char hostname[200];
	char startdate[50];  /* Must be > 26 characters */
	long partId;
	double totalZoneCount;
	double deposit, percent_residue;
	double part_deposit_bound, deposit_diff_bound;
	double diterations;

	int aidx;
	ScatterModeFuncPtr scatterModeFuncPtr = NULL;
	ScatterModeFuncPtr altScatterModeFuncPtr = NULL;
	long initialRandomSeed, finalRandomSeed;
	long rawNumThreads;

	/* Get executable name by pointing to argv[0] */
	CLOMP_exe_name = argv[0];
	printf ("CLOMP_TM Version 1.60 (9Nov2012).");


	/* Print usage if not 11 arguments */
	if (argc != 13) {
		if (argc > 1) {
			fprintf (stderr, "Error: Passed %i command line arguments, expect 11!\n", argc -1);
		}
		print_usage();
		exit (1);
	}

	/* Get hostname running on */
	if (gethostname (hostname, sizeof(hostname)) != 0)
		strcpy (hostname, "(Unknown host)");

	/* Read in command line args (all must be positive ints) */
	rawNumThreads = convert_to_positive_long ("numThreads", argv[1]);

	/* Set CLOMP_numThreads to system default if -1 */
	if (rawNumThreads == -1) {
		CLOMP_numThreads = omp_get_max_threads();
	}
	/* Otherwise, used passed in thread count */
	else {
		CLOMP_numThreads = rawNumThreads;
	}

	CLOMP_allocThreads = convert_to_positive_long ("allocThreads", argv[2]);
	/* Allow x1, x2, etc. to scale numParts with numThreads -JCG 6Jan2012*/
	if (argv[3][0] == 'x')
	{
		long xN = convert_to_positive_long ("xN for numParts", &argv[3][1]);
		CLOMP_numParts = CLOMP_numThreads * xN;
	}
	else
		CLOMP_numParts = convert_to_positive_long ("numParts", argv[3]);

	/* Allow dM to set zonesPerPart to M/numParts -JCG 6Jan2012*/
	if (argv[4][0] == 'd') {
		long M = convert_to_positive_long ("dM for zonesPerPart", &argv[4][1]);
		CLOMP_zonesPerPart = M / CLOMP_numParts;

		/* Must have at least one zone per part*/
		if (CLOMP_zonesPerPart < 1)
			CLOMP_zonesPerPart = 1;
	}
	else {
		CLOMP_zonesPerPart = convert_to_positive_long ("zonesPerPart", argv[4]);
	}

	CLOMP_zoneSize = convert_to_positive_long ("zoneSize", argv[5]);
	CLOMP_scatterModeUnparsed = strdup(argv[6]);
	CLOMP_scatterMode = strdup(CLOMP_scatterModeUnparsed);
	CLOMP_scatter = convert_to_positive_long ("scatter", argv[7]);
	CLOMP_flopScale = convert_to_positive_long ("flopScale", argv[8]);
	CLOMP_randomSeedOffset = convert_to_positive_long ("randomSeed", argv[9]);
	CLOMP_scrubRate = convert_to_positive_long ("scrubRate", argv[10]);
	CLOMP_timeScale = convert_to_positive_long ("timeScale", argv[11]);
	long CLOMP_totalIteration = convert_to_positive_long ("totalIteration", argv[12]);

	/* Default scatterPartMod and altScatterPartMod to numParts
	 * (i.e. no effect of partId's selected by the scatterMod)
	 */
	CLOMP_scatterPartMod = CLOMP_numParts;
	CLOMP_altScatterPartMod = CLOMP_numParts;

	/* Use randomSeed as offset to initial value and then, if offset not 0,
	 * generate two random numbers with it to create random starting point.
	 * -JCG 19DEC2011
	 */
	if (CLOMP_randomSeedOffset != 0)
	{
		CLOMP_randomSeed += CLOMP_randomSeedOffset;
		rand_small_int(64);
		rand_small_int(64);
	}

	/* Parse optional extension to scatterMode of
	 * "scatterMode%partMod,altScatterCount,altScatterMode%altPartMod"
	 * Will change ',' and '%' into string terminators and
	 * use pointers into string to read in extra parameters
	 * -JCG 12/19/11  '%' support added -JCG 1/4/12
	 */
	{
	char *ptr = CLOMP_scatterMode;
	char ch;
	char *count_ptr = NULL;
	char *alt_ptr = NULL;
	char *partMod_ptr = NULL;
	char *altPartMod_ptr = NULL;
	while ((ch = *ptr) != 0)
	{
	    if (ch == '%')
	    {

		/* If haven't seen count yet, it is partMod_ptr */
		if (count_ptr == NULL)
		    partMod_ptr = ptr+1;

		/* Otherwise, must have alt pointer already for altMod_Ptr */
		else if (alt_ptr != NULL)
		    altPartMod_ptr = ptr+1;
		else
		{
		    fprintf(stderr,
			    "Error: Unexpected %% in scatterMode '%s' here '%s'\n", CLOMP_scatterModeUnparsed, ptr);
		    print_usage();
		    exit(1);
		}
		/* Change percent sign into string terminator */
		*ptr = 0;
	    }

	    if (ch == ',')
	    {
		/* Change comma into string terminator */
		*ptr = 0;

		/* The next parameter starts after the ',' */
		if (count_ptr == NULL)
		    count_ptr = ptr+1;
		else if (alt_ptr == NULL)
		    alt_ptr = ptr+1;
		else
		{
		    fprintf(stderr,
			    "Error: Too many args found in scatterMode '%s'\n", CLOMP_scatterModeUnparsed);
		    print_usage();
		    exit(1);
		}
	    }
	    ptr++;
	}

	/* Convert mod values, if exist */
	if (partMod_ptr != NULL)
	{
	    /* If /D, set Mod to numParts/D -JCG 6Jan2012*/
	    if (partMod_ptr[0] == '/')
	    {
		long D = convert_to_positive_long ("/D for %mainMod",
						   &partMod_ptr[1]);
		CLOMP_scatterPartMod = CLOMP_numParts/D;
		/* Must be at least 1 */
		if (CLOMP_scatterPartMod < 1)
		    CLOMP_scatterPartMod = 1;
	    }
	    else
	    {
		CLOMP_scatterPartMod = convert_to_positive_long ("%mainMod",
								 partMod_ptr);
	    }
	}
	if (altPartMod_ptr != NULL)
	{
	    /* If /D, set altMod to numParts/D -JCG 6Jan2012*/
	    if (altPartMod_ptr[0] == '/')
	    {
		long D = convert_to_positive_long ("/D for %altMod",
						   &altPartMod_ptr[1]);
		CLOMP_altScatterPartMod = CLOMP_numParts/D;
		/* Must be at least 1 */
		if (CLOMP_altScatterPartMod < 1)
		    CLOMP_altScatterPartMod = 1;
	    }
	    else
	    {
		CLOMP_altScatterPartMod =
		    convert_to_positive_long ("%altMod", altPartMod_ptr);
	    }
	}

	/* If found one comma, expect both */
	if (count_ptr != NULL)
	{
			long maxScatterCount;

	    if (alt_ptr == NULL)
	    {
		fprintf(stderr,
			"Error: Expect one or three scatterMode args in '%s'\n", CLOMP_scatterModeUnparsed);
		print_usage();
		exit(1);
	    }
	    CLOMP_altScatterCount = convert_to_positive_long ("altScatterCount", count_ptr);
	    CLOMP_altScatterMode = strdup (alt_ptr);

			/* Sanity check, prevent infinite loops and very long
			 * setup times by limiting to 90% of possible slots
			 */
			maxScatterCount = 0.90 * (double)CLOMP_numParts *
			   (double)CLOMP_zonesPerPart * (double)CLOMP_scatter;
			if (CLOMP_altScatterCount > maxScatterCount)
			{
		fprintf(stderr,
			"Error: altScatterCount (%ld) exceeds 90%% of total slots (%ld)\n"
	                "       You can fix by swapping scatter mode order!\n",
						 CLOMP_altScatterCount, maxScatterCount);
		print_usage();
		exit(1);
			}
	}

	}

	/* Get function pointer to main scatterMode being used */
	scatterModeFuncPtr = getScatterModeFuncPtr("scatterMode", CLOMP_scatterMode);

	/* If have alternative scattermode, get pointer to it */
	if (CLOMP_altScatterMode != NULL)
	altScatterModeFuncPtr = getScatterModeFuncPtr("altScatterMode", CLOMP_altScatterMode);

	/* Zone size cannot be less than sizeof(Zone), force to be valid */
	if (CLOMP_zoneSize < sizeof (Zone))
	{
	printf ("*** Forcing zoneSize (%ld specified) to minimum zone size %ld\n\n",
		CLOMP_zoneSize, (long) sizeof (Zone));
	CLOMP_zoneSize = sizeof(Zone);
	}

	/* Calculate how many extra values fit at the end of the zone */
	{
	Zone testZone;
	long firstOffset, bytesRemaining;
	firstOffset = (char*)(&testZone.extraValues[0]) -
	    ((char *)(&testZone));
	bytesRemaining = CLOMP_zoneSize - firstOffset;

	CLOMP_numExtraValues = bytesRemaining/sizeof(double);

	/* Turn off use of extraValues if zoneSize < 64 bytes */
	if (CLOMP_zoneSize < 64)
	{
	    printf ("*** Turning off extraValue reads, zone size < 64 bytes\n");
	    CLOMP_numExtraValues = 0;
	}

	}

	/* Force zoneSize to be power of 2 (for alignment purposes).
	 * Intentionally did below CLOMP_numExtraValues calculation
	 * to still allow it to be turned off. -JCG 4Mar2011
	 */
	{
	/* Find power of two <= to zoneSize */
	int powOf2=32;
	while ((powOf2 < CLOMP_zoneSize) && (powOf2 < 1000000))
	{
	    powOf2 *= 2;
	}
	/* If closest powOf2 is not the same as zoneSize, force zoneSize
	 * to that size
	 */
	if (powOf2 != CLOMP_zoneSize)
	{
	    printf ("*** Forcing zoneSize to Power of 2 (%li -> %i) ***\n",
		    CLOMP_zoneSize, powOf2);
	    CLOMP_zoneSize = powOf2;
	}
	}


	/* Print out command line arguments as passed in */
	printf ("       Invocation:");
	for (aidx = 0; aidx < argc; aidx ++)
	{
	printf (" %s", argv[aidx]);
	}
	printf ("\n");


	/* Print out command line arguments read in */
	printf ("         Hostname: %s\n", hostname);
	printf ("       Start time: %s", startdate); /* startdate has newline */
	printf ("       Executable: %s\n", CLOMP_exe_name);
	if (rawNumThreads == -1)
	{
	/* Print out system default for threads */
	printf ("      numThreads: %ld (using system default)\n",
		CLOMP_numThreads);
	}
	else
	{
	printf ("      numThreads: %ld\n", CLOMP_numThreads);
	}

	/* If -1, use numThreads for alloc threads */
	if (CLOMP_allocThreads == -1)
	{
	/* Use numThreads for alloc threads if -1 */
	CLOMP_allocThreads = CLOMP_numThreads;

	/* Print out number of alloc threads to use */
	printf ("    allocThreads: %ld (using numThreads)\n",
		CLOMP_allocThreads);
	}
	else
	{
	/* Print out number of alloc threads to use */
	printf ("    allocThreads: %ld\n", CLOMP_allocThreads);
	}




	printf ("        numParts: %ld\n", CLOMP_numParts);
	printf ("    zonesPerPart: %ld\n", CLOMP_zonesPerPart);
	printf ("        zoneSize: %ld\n", CLOMP_zoneSize);
	/* Force alignment to be same as zoneSize for now */
	printf ("  zone alignment: %ld\n", CLOMP_zoneSize);
	printf ("     scatterMode: %s\n",  CLOMP_scatterModeUnparsed);
	printf ("         scatter: %ld\n", CLOMP_scatter);
	printf ("       flopScale: %ld\n", CLOMP_flopScale);
	printf ("      randomSeed: %ld\n", CLOMP_randomSeedOffset);
	printf ("       scrubRate: %ld\n", CLOMP_scrubRate);
	printf ("       timeScale: %ld\n", CLOMP_timeScale);


	/* Set the number of threads for the allocate seciton to what user  specified
	 * specified.   If threads are used, it may lay out the memory on
	 * NUMA system better for threaded computation.
	 */
	omp_set_num_threads ((int)CLOMP_allocThreads);
	#ifdef RTM
	TM_STARTUP((int)CLOMP_allocThreads);
	#endif

	/* Allocate part pointer array */
/*    partArray = (Part **) malloc (CLOMP_numParts * sizeof (Part*)); */
	/* Attempt to force zoneSize alignment to avoid false conflicts */
	posix_memalign((void **)&partArray, CLOMP_zoneSize,
		   (CLOMP_numParts * sizeof (Part*)));
	if (partArray == NULL)
	{
	fprintf (stderr, "Out of memory allocating part array\n");
	exit (1);
	}

	/* Make sure every allocation is aligned as expected */
	check_alignment(partArray);

	/* Initialize poitner array to NULL initially */
	for (partId = 0; partId < CLOMP_numParts; partId++)
	{
	partArray[partId] = NULL;
	}


	/* Calculate 1/numParts to prevent divides in algorithm */
	CLOMP_partRatio = 1.0/((double) CLOMP_numParts);


	/* Ininitialize parts (with no zones initially).
	 * Do allocations in thread (allocThreads may be set to 1 for allocate)
	 * to allow potentially better memory layout for threads
	 */
#pragma omp parallel private(partId)
  {
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	#pragma omp for schedule(static)
	for (partId = 0; partId < CLOMP_numParts; partId++)
	{
	Part *part;
/*	if ((part= (Part *) malloc (sizeof (Part))) == NULL) */
	/* Attempt to force zoneSize alignment to avoid false conflicts */
	if (posix_memalign((void**)&part, CLOMP_zoneSize, (sizeof (Part))) != 0)
	{
	    fprintf (stderr, "Out of memory allocating part\n");
	    exit (1);
	}

	/* Make sure every allocation is aligned as expected */
	check_alignment(part);

	/* Call standard part initializer for part just allocated.
	 * Allows parts to be allocated as desired.
	 */
	addPart(part, partId);
	}
  #ifdef RTM
  TM_THREAD_EXIT();
  #endif
  }

#pragma omp parallel private(partId)
  {
	#ifdef RTM
	TM_THREAD_ENTER();
	#endif
	/* Create and add zones to parts.
	 * Do allocations in thread (allocThreads may be set to 1 for allocate)
	 * to allow potentially better memory layout for threads
	 */
	#pragma omp for schedule(static)
	for (partId = 0; partId < CLOMP_numParts; partId++)
	{
	Zone *zoneArray, *zone;
	Zone **scatterZonesArray, **scatterZones;
	long zoneId;

	/* Allocate an array of zones for this part */
/*	zoneArray = (Zone *)malloc (CLOMP_zoneSize * CLOMP_zonesPerPart); */
	/* Attempt to force zoneSize alignment to avoid false conflicts */
	posix_memalign((void **)&zoneArray, CLOMP_zoneSize, (CLOMP_zoneSize * CLOMP_zonesPerPart));
	if (zoneArray == NULL)
	{
	    fprintf (stderr, "Out of memory allocating zone array\n");
	    exit (1);
	}

	/* Make sure every allocation is aligned as expected */
	check_alignment(zoneArray);

	/* Allocate an array of scatterZones arrays for this part.
	 * Do this so update arrays will be easy to prefetch (since continuous)
	 */
/*	scatterZonesArray = (Zone **)malloc (CLOMP_zonesPerPart *
					     CLOMP_scatter *
					     sizeof (Zone *)); */

	/* Attempt to force zoneSize alignment to avoid false conflicts */
	posix_memalign((void **)&scatterZonesArray, CLOMP_zoneSize,
		       (CLOMP_zonesPerPart *
			CLOMP_scatter *
			sizeof (Zone *)));
	if (scatterZonesArray == NULL)
	{
	    fprintf (stderr, "Out of memory allocating scatterZones array\n");
	    exit (1);
	}

	/* Make sure every allocation is aligned as expected */
	check_alignment(scatterZonesArray);

	/* In order to facilate the building of array of scatterZones for
	 * transational memory testing, keep the zoneArray pointer around
	 */
	partArray[partId]->zoneArray = zoneArray;

	/* Put all zones into part's zone linked list */
	for (zoneId = 0; zoneId < CLOMP_zonesPerPart; zoneId++)
	{
	    /* Get the current zone being placed */
	    zone = arrayToZone (zoneArray, zoneId);

	    /* Make sure every zone is aligned as expected */
	    check_alignment (zone);

	    /* Get the scatterZones array for this zone.
	     * Allocate scatter pointers per zone
	     */
	    scatterZones = &scatterZonesArray[zoneId * CLOMP_scatter];

	    /* Add it to the end of the the part */
	    addZone (partArray[partId], zone, scatterZones);
	}
	}
  #ifdef RTM
  TM_THREAD_EXIT();
  #endif
  }

	/* In order to get predictable behavior when using
	 * altScatterMode to randomly fill in the scatterArray
	 * with a different scatterMode, use the same
	 * starting CLOMP_randomSeed for all three passes
	 * that use random numbers.   This should allow
	 * changing scatterMode,scatterCount, altScatterMode
	 * independently without changing random distrbutions.
	 *
	 * Thats the hope anyway. :) -JCG 12/20/11
	 */
	initialRandomSeed = CLOMP_randomSeed;


	/* Fill in scatterZones in each part/zone, based on the ScatterMode.
	 * Need to do after all zones created, so we can point to them.
	 * I don't believe this needs to be threaded but I could be wrong.
	 *
	 * Now done with function pointers to pick scatter mode -JCG 12/20/11
	 */
	for (partId = 0; partId < CLOMP_numParts; partId++)
	{
	Zone *currentZone;
	Zone **scatterZones;
	long zoneId, scatterId;

	/* For every zone in part, fill in the scatterZones array */
	for (zoneId = 0; zoneId < CLOMP_zonesPerPart; zoneId++)
	{
	    /* Get the currentZone to update */
	    currentZone = arrayToZone(partArray[partId]->zoneArray, zoneId);

	    /* Get the zonestoUpdate array for the current zone */
	    scatterZones = currentZone->scatterZones;

	    /* for every pointer in zone's scatterZones, fill in using scatterMode */
	    for (scatterId = 0; scatterId < CLOMP_scatter; scatterId++)
	    {
		/* Get the zone targetted by the scatterMode function pointer */
		Zone *targetZone =
		    scatterModeFuncPtr(partId, zoneId, scatterId,
				       CLOMP_scatterPartMod);

		/* Fill in scatter zone array with target */
		scatterZones[scatterId] = targetZone;

		/* Sanity check that CLOMP_scatterPartMod used properly */
		if (targetZone->partId >= CLOMP_scatterPartMod)
		{
		    fprintf (stderr,
			     "Algorithm error: partId (%ld) >= scatterPartMod "
			     "(%ld) for %s\n", targetZone->partId,
			     CLOMP_scatterPartMod, CLOMP_scatterMode);
		    exit (1);
		}
	    }

	}
	}

	/* Did user specify use of altScatterMode? */
	if (CLOMP_altScatterMode != NULL)
	{
	long replaceId;

	/* Restore initial random seed, so have predicable randomness for selecting
	 * zones to replace
	 */
	CLOMP_randomSeed = initialRandomSeed;

	/* Randomly pick CLOMP_altScatterCount scatterArray entries
	 * and mark them with 1 to indicate they should
	 * be replaced with the altScatterMode pointers.
	 */
	for (replaceId=0; replaceId < CLOMP_altScatterCount; replaceId++)
	{
	    long randPartId, randZoneId, randScatterId;
	    Zone *targetZone=(Zone *)-1;

	    /* Randomly search until find a non-marked (-1) targetZone */
	    while (1)
	    {
		/* Pick a "random" partId*/
		randPartId = rand_small_int(CLOMP_numParts-1);

		/* Pick a "random" zoneId */
		randZoneId = rand_small_int(CLOMP_zonesPerPart-1);

		/* Pick a "random" scatterId */
		randScatterId = rand_small_int(CLOMP_scatter-1);

		/* Get the randomly selected scatterZone targetZone */
		targetZone = arrayToZone(partArray[randPartId]->zoneArray,
					 randZoneId)->scatterZones[randScatterId];

		/* Stop search, found target not yet marked */
		if (targetZone != (Zone *)-1)
		    break;
	    }

	    /* Mark with -1 to replace with altScatterMode target later */
	    arrayToZone(partArray[randPartId]->zoneArray,
			randZoneId)->scatterZones[randScatterId] = (Zone *)-1;
	}

	/* Restore initial random seed, so have predicable randomness when
	 * running altScatterMode algorithm.   Should return same results
	 * for each scatterArray target independent of the altScatterCount
	 * and main scatterMode.
	 */
	CLOMP_randomSeed = initialRandomSeed;


	/* For scatterZone targets marked with -1 above,
	 * Fill in based on the altScatterMode.
	 *
	 * Intentionally scan entire array so get same mapping for
	 * random scatter modes independant of how many targets are marked.
	 *
	 * I am hoping this predictable randomness will help in our
	 * parameter studies where we slowly add random targets. -JCG 12/20/11
	 */
	for (partId = 0; partId < CLOMP_numParts; partId++)
	{
	    Zone *currentZone;
	    Zone **scatterZones;
	    long zoneId, scatterId;

	    /* For every zone in part, fill in the scatterZones array */
	    for (zoneId = 0; zoneId < CLOMP_zonesPerPart; zoneId++)
	    {
		/* Get the currentZone to update */
		currentZone = arrayToZone(partArray[partId]->zoneArray, zoneId);

		/* Get the zonestoUpdate array for the current zone */
		scatterZones = currentZone->scatterZones;

		/* for every pointer in zone's scatterZones, fill in using scatterMode */
		for (scatterId = 0; scatterId < CLOMP_scatter; scatterId++)
		{
		    /* Get the zone targetted by the altScatterMode function pointer */
		    Zone *targetZone =
			altScatterModeFuncPtr(partId, zoneId, scatterId,
					      CLOMP_altScatterPartMod);

		    /* Sanity check that CLOMP_altScatterPartMod used properly*/
		    if (targetZone->partId >= CLOMP_altScatterPartMod)
		    {
			fprintf (stderr,
				 "Algorithm error: partId (%ld) >= altScatterPartMod "
				 "(%ld) for %s\n", targetZone->partId,
				 CLOMP_scatterPartMod, CLOMP_altScatterMode);
			exit (1);
		    }

		    /* Fill in scatter zone array with target if marked with -1*/
		    if (scatterZones[scatterId] == (Zone *)-1)
		    {
			scatterZones[scatterId] = targetZone;
		    }
		}
	    }
	}
	}


	/* Calculate the total number of zones */
	totalZoneCount = (double)CLOMP_numParts * (double)CLOMP_zonesPerPart;



	printf ("   Zones per Part: %.0f\n",   (double)CLOMP_zonesPerPart);
	printf ("      Total Zones: %.0f\n", (double)totalZoneCount);

	/* Calculate how many extra updates there will be */
	{
	int extra_update_count=0;
	int extra_index;
	/* March backwards from end of array.   Now record the earliest
	 * index touched with the current parameters. This will allow
	 * a forward going access pattern in update_part. -JCG 3/18/11
	 *
	 * Assume SIMD wants address aligned on 4 double boundary,
	 * so march back by 4 doubles from end to find start point. -JCG 4/1/11
	 */
	for (extra_index = CLOMP_numExtraValues; extra_index >= 0;
	     extra_index -= 4)
	{
	    /* Keep updating start index until hit end condition */
	    CLOMP_startExtraValues = extra_index;
	}

	/* Count the number of updates this start value and stride gives you
	 * -JCG 4/1/11
	 */
	for (extra_index = CLOMP_startExtraValues;
	     extra_index < CLOMP_numExtraValues;
	     extra_index += CALC_STRIDE)
	{
	    extra_update_count++;
	}
	printf ("Extra Zone Values: %ld\n", CLOMP_numExtraValues);
	printf (" Calc Start Index: %0li\n", CLOMP_startExtraValues);
	printf (" Zone Calc Stride: %0i\n", (int) CALC_STRIDE);
	printf (" Extra Zone Calcs: %0i\n", extra_update_count);
	printf ("   Zone Calc Flag: " CALCID "\n");
	printf ("Zone Calc Formula: " CALCZEROSTRING "\n");
#ifdef ENABLE_SIMD
	printf ("     SIMD Enabled: Yes (-DENABLE_SIMD)\n");
#else
	printf ("     SIMD Enabled: No (calc's dependent)\n");
#endif
	printf ("SelfScrub SpecIds: " SCRUBID "\n");

	/* Make sure start extra value is aligned to 32 byte boundary */
	check_SIMD_alignment (&partArray[0]->firstZone->extraValues[CLOMP_startExtraValues]);

	}

	printf ("Memory (in bytes): %.0f\n", (double)(totalZoneCount*CLOMP_zoneSize) + (double)(sizeof(Part) * CLOMP_numParts));


	/* Calculate a number of iterations that experimentally appears to
	 * give between 0.05 and 1.3 seconds of work on a 2GHz Opteron.
	 * The 0.05 for small inputs that fit in cache, 1.3 for others.
	 * The minimum iterations is 1, so once 1 million zones is hit,
	 * the iterations cannot drop any more to compensate.
	 * Also factor in flop_scaling now so setting flop_scaling to 100
	 * doesn't make benchmark run 100X longer.
	 * Also factor in scatter now so setting scatter to 3 doesn't make
	 * benchmark run 3X longer.
	 */
	/* JCG Scaled down for simulator runs 24Feb2010 */
	diterations = ceil((((double)10000) * ((double)CLOMP_timeScale))/
		       ((double)totalZoneCount * (double)CLOMP_flopScale *
			(double)CLOMP_scatter));

	/* Sanity check for very small zone counts */
	if (diterations > 2000000000.0)
	{
	printf ("*** Forcing iterations from (%g) to 2 billion\n",
		diterations);
	diterations = 2000000000.0;
	}

	/* Convert double iterations to int for use */
	CLOMP_num_iterations = (long) diterations;


	/* Give Small TM update counts to simplify calculations
	 * of other overheads that we don't calculate.
	 */
	printf (" Small TM Updates: %.0f\n",
	    (double) CLOMP_num_iterations * (double) 10.0 *
	    (double)CLOMP_scatter * (double)totalZoneCount);

	/* Give Large TM update counts to simplify calculations
	 * of other overheads that we don't calculate.
	 */
	printf (" Large TM Updates: %.0f\n",
	    (double) CLOMP_num_iterations * (double) 10.0 *
	    (double)totalZoneCount);


	/* Calculate serially the percent residue left after one pass */
	percent_residue = 0.0;

	/* If we deposit 1.0/numParts, sum of residues becomes percent residue */
	deposit = CLOMP_partRatio;

	/* In order to have sane rounding error bounds, determine the minimum
	 * amount deposited in any zone.   Initialize to deposit as a starting
	 * point for the min calculation below.
	 */
	CLOMP_error_bound = deposit;
	CLOMP_tightest_error_bound = deposit;

	/* Scan through zones and add deposit to each zone */
	for (partId = 0; partId < CLOMP_numParts; partId++)
	{
	/* Do serial calculation of percent residue
	 * Don't use transactional memory since serial*/
	update_part_no_TM (partArray[partId], deposit);
	percent_residue += partArray[partId]->residue;

		/* The 'next' deposit is smaller than any actual deposit made in
		 * this part.  So it is a good lower bound on deposit size.
		 */
		part_deposit_bound =
			partArray[partId]->residue * partArray[partId]->deposit_ratio;

		/* If we use the smallest bound on any part, we should have the
		 * biggest error bound that will not give false negatives on
		 * bad computation.
		 */
		if (CLOMP_error_bound > part_deposit_bound)
		{
			CLOMP_error_bound = part_deposit_bound;
		}

		deposit_diff_bound = part_deposit_bound * partArray[partId]->deposit_ratio;
		if (CLOMP_tightest_error_bound > deposit_diff_bound)
		{
			CLOMP_tightest_error_bound = deposit_diff_bound;
		}
	}
	printf ("Iteration Residue: %.6f%%\n", percent_residue*100.0);
	printf ("  Max Error bound: %-8.8g\n", CLOMP_error_bound);
	printf ("Tight Error bound: %-8.8g\n", CLOMP_tightest_error_bound);

	/* Calculate the expected converged residue (for infinite iterations)
	 * If Y is the percent residue after one pass, then the converged
	 * expected converged residue = (added_each_cycle * Y)/ (1-Y)
	 */
	CLOMP_max_residue = (1.0*percent_residue)/(1-percent_residue);
	printf ("      Max Residue: %-8.8g\n", CLOMP_max_residue);


	/* Set the number of threads for the computation seciton to what user
	 * specified. Because we are using alloc threads also, have to explicitly
	 * set even if using system default
	 */
	omp_set_num_threads ((int)CLOMP_numThreads);

	/* --------- Start Serial Ref benchmark measurement --------- */
	    /* Reinitialize parts and warm up cache by doing dummy update */
	    reinitialize_parts();

	    /* Do the serial version of calculation and measure time*/
	    print_pseudocode ("Serial Ref", "------ Start Serial Ref Pseudocode ------");
	    print_pseudocode ("Serial Ref", "/* Measure serial reference performance */");
	    print_pseudocode ("Serial Ref", "deposit = calc_deposit ();");
	    print_pseudocode ("Serial Ref", "for (pidx = 0; pidx < numParts; pidx++)");
	    print_pseudocode ("Serial Ref", "  update_part_no_TM (partArray[pidx], deposit);");
	    print_pseudocode ("Serial Ref", "------- End Serial Ref Pseudocode -------");

	    do_serial_ref_version();

	    /* Check data for consistency and print out data stats*/
	    print_data_stats ("Serial Ref");
	long i;
	for(i=0; i < CLOMP_totalIteration; i++){
#ifdef SMALL_TM
	/* --------- Start Small TM benchmark measurement --------- */
	/* Reinitialize parts and warm up cache by doing dummy update */
	reinitialize_parts();

	/* Do the OMP Small TM version of calculation and measure time*/
	print_pseudocode ("Small TM", "------ Start Small TM Pseudocode ------");
	print_pseudocode ("Small TM", "/* Use OpenMP parallel for schedule(static) on original loop. */");
	print_pseudocode ("Small TM", "deposit = calc_deposit ();");
	print_pseudocode ("Small TM", "#pragma omp parallel for private (pidx) schedule(static)");
	print_pseudocode ("Small TM", "for (pidx = 0; pidx < numParts; pidx++)");
	print_pseudocode ("Small TM", "  update_part_small_TM (partArray[pidx], deposit);");
	print_pseudocode ("Small TM", "------- End Small TM Pseudocode -------");

	do_small_tm_version();

	/* Check data for consistency and print out data stats*/
	print_data_stats ("Small TM");

#endif // SMALL_TM

#ifdef LARGE_TM
	/* --------- Start Large TM benchmark measurement --------- */
	/* Reinitialize parts and warm up cache by doing dummy update */
	reinitialize_parts();

	/* Do the OMP Large TM version of calculation and measure time*/
	print_pseudocode ("Large TM", "------ Start Large TM Pseudocode ------");
	print_pseudocode ("Large TM", "/* Use OpenMP parallel for schedule(static) on original loop. */");
	print_pseudocode ("Large TM", "deposit = calc_deposit ();");
	print_pseudocode ("Large TM", "#pragma omp parallel for private (pidx) schedule(static)");
	print_pseudocode ("Large TM", "for (pidx = 0; pidx < numParts; pidx++)");
	print_pseudocode ("Large TM", "  update_part_large_TM (partArray[pidx], deposit);");
	print_pseudocode ("Large TM", "------- End Large TM Pseudocode -------");

	do_large_tm_version();

	/* Check data for consistency and print out data stats*/
	print_data_stats ("Large TM");
#endif //LARGE_TM
	}
	#ifdef RTM
	TM_SHUTDOWN();
	#endif
	return (0);
}
