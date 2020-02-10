/* 07 april 1995  R. Howell :  Modified so that it only takes
                               one value.  The dec dial term should
                               always be zero.  Only the RA term is changed.
                               Also removed the useless commands dial and idial.
                               The comments in the original dial code were 
                               inaccurate.  The "default" values for the dial
                               variables should be zero.

  Modifications:
  96-07-01	Howell	Changed logging on col, icol, off2, and ioff2 to log
                        the changed values, not just the fact that the command
                        was executed.

  20Jan00       JSW     Port to Linux.
*/

#include <math.h>
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct  wiro_memory *tinfo;

main( argc, argv )
int argc;
char *argv[ ];
{

double val,
    valha,
    valdec;

char   log_message[98];	 /* Message to place in log file */

tinfo = get_tinfo();

if (argc == 1) {
	if ( strcmp(argv[0],"zero") ==0 ) {
   		OFFSET_DEC = 0.0;
		OFFSET_HA = 0.0;
		log_entry( "zero" );
		exit(0);
	}
   else if ( strcmp( argv[ 0 ], "col" ) == 0 ) {
      COL_DEC = 0.0;
      COL_HA = 0.0;
      sprintf( log_message, 
			   "col:  HA Coll: %10.2lf  DEC Coll: %10.2lf  Dial: %10.2lf",
				COL_HA * 3600.0, COL_DEC * 3600.0 , tinfo -> dial_ha * 3600.0);
      log_entry(log_message);
   } 
   else if ( strcmp( argv[ 0 ], "icol" ) == 0 ) {
      COL_DEC += OFFSET_DEC;
      COL_HA += OFFSET_HA;
      OFFSET_DEC = 0.0;
      OFFSET_HA = 0.0;
	  sprintf( log_message, 
	           "icol:  HA Coll: %10.2lf  DEC Coll: %10.2lf  Dial: %10.2lf",
				COL_HA * 3600.0, COL_DEC * 3600.0 , tinfo -> dial_ha * 3600.0);
	  log_entry(log_message);
   }
   else if ( strcmp( argv[ 0 ], "off2" ) == 0 ) {
      OFFSET_2_DEC = OFFSET_DEC;
      OFFSET_2_HA = OFFSET_HA;
      OFFSET_DEC = 0.0;
      OFFSET_HA = 0.0;
	  sprintf( log_message, 
	           "off2:  HA Offset2: %10.2lf  DEC Offset2: %10.2lf",
				OFFSET_2_HA * 3600.0, OFFSET_2_DEC * 3600.0 );
	  log_entry(log_message);
   } 
   else if ( strcmp( argv[ 0 ], "ioff2" ) == 0 ) {
      OFFSET_2_DEC += OFFSET_DEC;
      OFFSET_2_HA += OFFSET_HA;
      OFFSET_DEC = 0.0;
      OFFSET_HA = 0.0;
	  sprintf( log_message, 
	           "ioff2:  HA Offset2: %10.2lf  DEC Offset2: %10.2lf",
				OFFSET_2_HA * 3600.0, OFFSET_2_DEC * 3600.0 );
	  log_entry(log_message);
   }
   else {
		printf("Unrecognized command %s \n",argv[0]);
		exit(1);
   } 
}

if ( argc == 2) {
    sscanf( argv[ 1 ], "%lf", &val );
	if ( fabs( val ) > 3600.0 ) {
		printf( "Illegal offset value: %s\n", argv[ 1 ] );
		exit( -1 );
    }
	else
		val /= 3600.0;
	if (strcmp(argv[0],"nn") == 0) {
      	OFFSET_DEC += val;
      	sprintf( log_message, "nn %s", argv[ 1 ] );
      	log_entry( log_message );
      	exit(0);
    }	
    else if ( strcmp( argv[ 0 ], "ss" ) == 0 ) {
        OFFSET_DEC -= val;
      	sprintf( log_message, "ss %s", argv[ 1 ] );
      	log_entry( log_message );
        exit(0);
    }
    else if ( strcmp( argv[ 0 ], "ee" ) == 0 )  {
        OFFSET_HA -= val;
      	sprintf( log_message, "ee %s", argv[ 1 ] );
      	log_entry( log_message );
        exit(0);
    }
    else if ( strcmp( argv[ 0 ], "ww" ) == 0 ) {
        OFFSET_HA += val;
      	sprintf( log_message, "ww %s", argv[ 1 ] );
      	log_entry( log_message );
        exit(0);
    }
	else if (strcmp( argv[ 0 ], "setdial") == 0) {
		tinfo->dial_ha = val;
		sprintf( log_message, "setdial %s", argv[ 1 ]);
		log_entry( log_message );
	}
    else {
		printf("Unrecognized command %s \n",argv[0]);
		exit(1);
	}
}
if ( argc == 3) {
	sscanf( argv[ 1 ], "%lf", &valha );
	valha /= 3600.0;
	sscanf( argv[ 2 ], "%lf", &valdec );
	valdec /= 3600.0;
	if ( fabs( valha ) > 1.0 ) {             /* 1 degree or 3600 arcsec max. */
		printf( "Illegal offset value: %s\n", argv[ 1 ] );
		exit( -1 );
    }
	else if ( fabs( valdec ) > 1.0 ) {       /* 1 degree or 3600 arcsec max. */
		printf( "Illegal offset value: %s\n", argv[ 2 ] );
		exit( -1 );
    }
	else if (strcmp( argv[ 0 ], "setcol") == 0) {
		tinfo->col_ha = valha;
		tinfo->col_dec = valdec;
		sprintf( log_message, "setcol %s %s", argv[ 1 ], argv[ 2 ] );
		log_entry( log_message );
	}
    else {
		printf("Unrecognized command %s \n",argv[0]);
		exit(1);
	}
}
}
