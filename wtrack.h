/* Include file for tracking library functions (trklib) */
#ifndef _wtrack
#define _wtrack

#define PI   3.14159265358979323846
/* #define DPR  57.29577951 */
#define RAD( x ) ( ( x ) * PI / 180.0 )
#define DEG( x ) ( ( x ) / PI * 180.0 )

/***********************************************************************/
unsigned short gray2bin(unsigned short a);

/* Convert gray code (from RA and DEC encoders) to binary.             */
/***********************************************************************/
char *hr_hms(double hr, int fld, int dpl);

/* Convert hr to hms string, with given field width & decimal places.  */
/***********************************************************************/
double hms_hr(char hms[]);

/* Convert hh:mm:ss.ss or hh:mm.mm or hh.hh string to hours            */
/***********************************************************************/


/***************************************************************************/
double  julian( int year, int month, int day, double time );

/* Given year, month, day, UT time in hours, return julian day.   */

/***************************************************************************/
void    planet( char name[], double jd,
				double *ra, double *dec, double *dearth, double *dsun);

/* Given planet name, i.e. "Jupiter", Julian Date+TDT correction,
				return J2000. RA, DEC, and "Apparent" distance to
				earth and sun.  Apparent means as traveled by light.  */

/***************************************************************************/
/* void aplanet(char NAME[], int YEAR, int MONTH, int DAY, double TIME,
				   int apparent,
				   double *ORIG_RA, double *ORIG_DEC,
				   double *V_RA,    double *V_DEC,
				   double *DEARTH,  double *DSUN,      double *HP ) ; */

/* Given above parameters, returns APPARENT position, plus velocities,
		 distances, and the horizontal parallax (in degrees.).          */
/*       Set apparent = 1 for apparent    position of date,             */
/*       Set apparent = 0 for astrometric position of date.             */

/***************************************************************************/
void    trimstr( char name[] );
/* Given a string, trim trailing blank characters from it.                 */

/**************************************************************************/
void    precession( double  js, double  je, double pmra, double pmdec,
					double *ra, double *dec);
/* Given two julian dates, and proper motions and coordinates,  precess   */
/* the coordinates.                                                       */

void    pmatrix( double js, double je, double matrix[] [3] );
void    nutation(double jul, double *ra, double *dec);
void    annabr(double jul, double *deltara, double *deltadec);
void    lonobl(double jul, double *lon, double *obl);
double  oblecl(double jul);
double  kepler(double M, double e);


/************************************************************************/
/* Given planet number (1-9 for THIS routine) and JD,                   */
/* return heliocentric equatorial (x,y,z) in J2000. frame.              */
/* NOTE: YOU should add TDT-UT to JD before calling this routine.       */

void    planetxyz( int pnum, double jd,
				double *x, double *y, double *z);

/* Convert J2000. ecliptic coordinates into J2000. equatorial.          */
void ecl_eqt(double *x,double *y, double *z) ;

#endif
