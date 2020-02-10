/* hr_hms      Converts the double precision hr to a character string */
/*             hh:mm:ss.ss   where the field with to use is fld, and  */
/*             the number of decimal places is dpl.                   */
/*             Return a pointer to the null terminated character array*/
/*             If you give a field width of 0, it returns a string    */
/*             with no leading blanks.  The largest fld you can give  */
/*             is 80 characters.                                      */
/*             The largest dpl it will use is 20.                     */

/*              Robert R. Howell    May 13, 1991                      */
/*                                  Aug  1, 1991  Fixed round to 60's bug */
#include <math.h>

char    hms[81];               /* hms string to return.                 */

char *hr_hms(hr, fld, dpl)
double hr;
int fld; 
int dpl;
{
	char    frmt[20];
	int     hours;
	int     minutes;
	double  seconds;
	double  pdpl;                  /* Power used to shift dpl places.   */
	int     negflag;               /* 1 means hr is negative.           */
	int     hrfld;
	int     secfld;

	
	if (fld >80) {
		printf("Error:  HMS string can be at most 80 characters.");
		exit(-1);
	}

	if (hr < 0)                 /* Following needs positive numbers. */
	{                       /* Find sign, then take abs.         */
		negflag = 1;
		hr = fabs(hr);
	}
	else
		negflag = 0;

	/* Round to correct number of places, to force output to */
	/* round to 1:00 rather than 0:60, etc.                  */

	/*  Find power to convert to ", then shift dpl places.    */
	if (dpl > 0) {
		pdpl = 3600. * pow((double) 10.0, (double) dpl);
	}
	else
		pdpl = 3600.;
	if (pdpl < 3600.) {
		printf( "pdpl was too small %lf - dpl was %d\n",pdpl,dpl);
		pdpl = 3600.0; exit(1);
	}
	hr = floor( hr * pdpl + 0.5) / pdpl;  /* Shift, round, shift back. */


	/* Now separate into separate hours, minutes, seconds, etc. */
	hours    = (int) hr;
	hr      -= (double) hours;
	hr      *= 60.;
	minutes  = (int) hr;
	hr      -= (double) minutes;
	hr      *= 60.;
	seconds = hr;

	if (negflag) hours = -hours;

	/* Create the format string.  Need to enter field size and # of dpl's */
	/* for the hours and seconds.                                         */

	/* Limit # of decimal places to 20 */
	if (dpl >20) dpl = 20;

	if (dpl > 0) {
		secfld = (int) (dpl + 3);
		hrfld  = (int) (fld - dpl - 7);
	}
	else {
		secfld = 2;
		hrfld  = (int) (fld - 6);
	}

	/* If number is not negative, or hours are not 0 you can just print */
	/* the hours field.  Otherwise, you must force it to print -0:      */
	if ( !negflag || hours )
	{
		if (hrfld<0) hrfld = 0;
/*		sprintf(frmt, "%%%dd:%%02d:%%%02d.%ldlf", hrfld, secfld, dpl); */
		sprintf(frmt, "%%%dd:%%02d:%%%02d.%ldlf", hrfld, secfld, dpl); 
		sprintf(hms, frmt, hours, minutes, seconds);
	}
	else
	{
		hrfld -= 2;   /* Print -0 manually, space over this much */
		sprintf(frmt, "%%%ds-0:%%02d:%%%02d.%ldlf", hrfld, secfld, dpl);
		sprintf(hms, frmt, " ", minutes, seconds);
	}
	return hms;

error:  
	printf("\nError in HMS to HR conversion.");
	exit(1);
}
