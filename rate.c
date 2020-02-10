/* rate.c  sets rates for objects */
/* Jan. 14, 2000  R. Howell   Added fix for timeonobj rollover at 0 UT */
/* Jan. 21, 2000  J. Weger    Reinstalled logging for Linux. */

#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"
#include <math.h>
#include <sys/time.h>
#include <sys/timeb.h>

void log_entry( char *comment );

#define PI 3.141592654

struct  wiro_memory *tinfo;
main( argc, argv )
int argc;
char *argv[ ]; 
{
struct timeval timeonobj;
double seconds_on_object;
double vra, vdec;
char s1[ 98 ];

tinfo = get_tinfo();

if ( argc == 2 ) {
   vdec=V_DEC *3600. ;
   vra = V_RA * 3600 * 15 * cos(PI * DES_DEC / 180.);
   if ( tinfo->motion_type == NON_SOLAR) {
	printf(" Co-ordinate rates of object relative to stars\n");
     	printf(" (arcsec/sec)  ra: %10.4f  dec %10.4f\n", vra, vdec );
   	exit( 0 );
   }
   else {
	printf(" Co-ordinate rates of object relative to stars\n");
     	printf(" (sec/hr)  ra: %10.4f  dec %10.4f\n", vra, vdec );
	exit(0);
   }
} 

if ( argc != 4 ) {
   printf("Usage:\n"   );
   printf("  rate q reports the current rate of the object wrt the stars\n");
   printf("  input rates are given in arcsecs in ra and dec per second\n");
   printf("  For solar system objects, the units are per hour\n");
   printf("  rate a #1 #2  sets the rates, removing any current rates\n");
   printf("  rate i #1 #2  sets the rates, incrementing the rates \n");
   exit ( -1 );
}

sscanf( argv[2], "%lf", &vra);
vra /= 15. * cos( PI * DES_DEC / 180.); 

sscanf( argv[3], "%lf", &vdec);


if (tinfo -> motion_type == NON_SOLAR) {

	if ( strcmp( argv[1], "a" ) == 0 ) {
   		V_RA = vra / 3600. ; 
   		V_DEC = vdec / 3600. ;
	}

	else if ( strcmp( argv[1], "i" ) == 0 ) {
        	V_RA += vra / 3600.0 ; 
        	V_DEC += vdec / 3600.0 ;
	}
}

if ( tinfo -> motion_type == SOLAR ) {
	timesub(CURRENT,TONOBJ, &timeonobj);
	seconds_on_object = timesize(timeonobj);
	while (seconds_on_object < 0.) seconds_on_object += 86400.;
	ORIG_RA  += seconds_on_object * V_RA/3600.;
      	ORIG_DEC += seconds_on_object * V_DEC/3600.;
	timecopy(CURRENT,&TONOBJ);

        if (strcmp( argv[1], "i") == 0 ) {
		V_DEC += vdec /= 3600. ;
        	V_RA  +=  vra /=  ( 3600. ) ; 
	}

        if ( strcmp( argv[1], "a") == 0 ) {
		V_DEC = vdec /=  3600. ;
        	V_RA  =  vra /=  ( 3600.  ) ; 
	}
}
sprintf( s1, "rate %s %s %s", argv[ 1 ], argv[ 2 ], argv[ 3 ] );
log_entry( s1 );
}
