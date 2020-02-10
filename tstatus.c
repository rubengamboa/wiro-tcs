/* WIRO Routine to return telescope status over the serial port */
/* LynxOS version */
/* Robert R. Howell   95/07/04  20:55 */
/* RRH    96/09/08  Added real focus output */
/* RRH    99/05/23  Discovered only current epoch RA DEC is stored.*/
/*                  Output that and set EPHOCH = 0 to signify that */
/* RRH    99/12/20  Trying to create linux version.
		Sort of works but altitude, object name, filter are wrong. */

#define	TS_VERSION 1.2	/* Version 1.2 is same as 1.2 but on lixus */

#include <math.h>
#include <time.h>
#include <stdio.h>
/* #include <smem.h> */
#include <stdio.h>
#include "/home/observer/wiro/track/wirotypes.h"
#include "/home/observer/wiro/track/wiro.h"

struct  wiro_memory *tinfo, *get_tinfo();

main()
{
	int csent, i;

	tinfo = get_tinfo();  /* Attach shared memory */
   	printf("%15.5f\n", TS_VERSION);
   	printf("%15.5f\n", DES_RA);
   	printf("%15.5f\n", DES_DEC);
   	printf("%15.5f\n", HA);
   	printf("%15.5f\n", 0.); /* EPOCH); */
   	printf("%15.5f\n", 3600.0* OFFSET_HA);
   	printf("%15.5f\n", 3600.0* OFFSET_DEC);
   	printf("%15.5f\n", 3600.0* OFFSET_2_HA);
   	printf("%15.5f\n", 3600.0* OFFSET_2_DEC);
	printf("%15d\n",   NODDED);
	printf("%15.5f\n", 3600.0* NOD_HA);
   	printf("%15.5f\n", 3600.0* NOD_DEC);
	printf("%15.5f\n", ALT);
/*  To avoid overflow if focus not available, */
/*  check and if out of range print -9999. */
	if( FOCUS < -2000 | FOCUS > 2000)
		printf("%15.5f\n", -9999.);
	else
		printf("%15.5lf\n", FOCUS);
	printf("%15d\n",   YEAR);
	printf("%15d\n",   MONTH);
	printf("%15d\n",   DAY);
	printf("%15.5f\n", UT_TIME);
	printf("%s\n",     OBJECT_NAME);
	printf("%s\n",     FILTER);
	printf("END\n");

	/* To make the receiving program simpler, */
	/*  pad the output so that exactly 512 cxharacters are sent. */
	csent = 18 * 17;
	csent += strlen(OBJECT_NAME) + 2;
	csent += strlen(FILTER) + 2;
	csent += strlen("END") + 2;
	for (i=0; i<512-csent; i++)
		putchar(' ');
}
