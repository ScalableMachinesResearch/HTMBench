/* Simple Zone data structure */
/* Reorder fields and add pad to make similar sized Zone on 32-bit machines
 * and 64-bit machines, since affects work done. -JCG 4/5/11
 */
typedef struct _Zone
{
	long zoneId;
	double value; /* Force 8 byte alignment here -JCG 4/5/11 */
	long partId;
	double pad1;  /* Force 8 byte alignment and pad here -JCG 4/5/11 */
	struct _Zone *nextZone;
	double pad2;  /* Force 8 byte alignment and pad here -JCG 4/5/11 */
	struct _Zone **scatterZones;  /*Array of zones to update (creates races)*/
#ifdef PAD_ZONE
	/* JCG 4/4/11 */
	double pad3;  /* Make extraValues start on 64 byte boundary on 64-bit */
#endif
	double extraValues[1];  /* Hack to access rest of Zone data as doubles*/
} Zone;

/* Part data structure */
typedef struct _Part
{
	long partId;
	long zoneCount;
	long update_count;
	Zone *firstZone;
	Zone *lastZone;
	double deposit_ratio;
	double residue;
	double expected_first_value; /* Used to check results */
	double expected_residue;     /* Used to check results */
	Zone *zoneArray;             /* Used to build up mesh for TM */
} Part;


/* Allow a configurable amount of calculation to be done in a zone update.
 * Leverage the array of 0.0 values that get added to the deposit.
 * A floating point 0.0 will be passed in.  Can do any math you want
 * as long as 0.0 comes out at the end.  Defaults to just passing back
 * the 0.0 (i.e., -DNO_CALC).
 */
#if defined(COMPLEX_CALC)
#define CALCZERO(x) (log10(sqrt(((1.0/(x+2.0))+9999.5)))-2.0)
#define CALCID "-DCOMPLEX_CALC"

#elif defined(DIVIDE_CALC)
#define CALCZERO(x) ((1.0/(x+2.0))-0.5)
#define CALCID "-DDIVIDE_CALC"

#elif defined(MANYDIVIDE_CALC)
#define CALCZERO(x) ((((((42.0/(x+3.0))+1.0)/(x+5.0))-2.0)/(x+2.0))-0.5)
#define CALCID "-DMANYDIVIDE_CALC"

#elif defined(NO_CALC)
#define CALCZERO(x) (x)
#define CALCID "-DNO_CALC"

#else
#define CALCZERO(x) (x)
#define CALCID "-DNO_CALC (default)"

#endif


/* Pick how the stride for the extra calcs (4 -> 32 bytes, 8 -> 64 bytes).
 */
#ifndef CALC_STRIDE
/*#define CALC_STRIDE 8 */
#define CALC_STRIDE 4
/*#define CALC_STRIDE 1 */
#endif

/* Allow us to print out the formula above using C preprocessor tricks */

#define CLOMP_XSTR(s) CLOMP_STR(s)
#define CLOMP_STR(s) #s
#define CALCZEROSTRING CLOMP_XSTR(CALCZERO(x))


#ifdef SELF_SCRUB
#define SCRUBID "Enabled (-DSELF_SCRUB)"

/* Self scrub specIds for small TM regions, if compiled with -DSELF_SCRUB */
#define do_small_self_scrub() \
{\
	/* Force read to written TM value to clear specIDs */\
	*(volatile double *)&scatter_zone->value;\
}

/* Self scrub specIds for large TM regions, if compiled with -DSELF_SCRUB */
#define do_large_self_scrub() \
/* Self Scrub routine */ \
{\
	/* Need to scrub all scatter zones */\
	for (scatter_count = 0; scatter_count < CLOMP_scatter;\
	 scatter_count ++)\
	{\
		/* Use minimum stride of 32 to minimize overhead -JCG 4/21/11*/\
		int scrub_stride = CALC_STRIDE;\
		if (scrub_stride < 32) scrub_stride = 32;\
		\
	scatter_zone = zone->scatterZones[scatter_count];\
	\
	/* Write 0.0 to all read extraValues to clear specIDs tied\
	 * up by those reads.   Of course, this may cause a lot\
	 * more TM conflicts for these reads\
	 */\
	for (extra_index = CLOMP_startExtraValues; \
	     extra_index < CLOMP_numExtraValues;\
	     extra_index += scrub_stride)\
	{\
	    scatter_zone->extraValues[extra_index] = 0.0;\
	}\
	\
	/* Force read to written TM value to clear specIDs */\
	*(volatile double *)&scatter_zone->value;\
	}\
}
#else
/* The default, no self scrub of specIds for small/large TM regions */
#define SCRUBID "Disabled (default)"
#define do_small_self_scrub() {}
#define do_large_self_scrub() {}
#endif