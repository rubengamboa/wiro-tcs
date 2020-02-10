/***************************************************************************/
/* fixed.c                                                                 */
/*  Linked to the command "zenith" which makes the desired coordinates     */
/*    0 hours and 41.0 degrees north, near the actual zenith.              */
/*  Linked to the command "service" which makes the desired coordinates    */
/*    0 hours and -35.0 degrees south, over the service platform.          */
/*  "fixed <HA> <Dec>" commands movement to the desired position and then  */
/*    remains at that position without sidereal rate tracking.             */
/*  "fixed <RA> <Dec> follow" commands movement to the desired sky         */
/*    position and then tracks that sky position at the sidereal rate.     */
/***************************************************************************/

/* Port to Linux.  21Jan00 JSW                                             */


#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"
#include <math.h>

void log_entry( char *comment );

struct  wiro_memory *tinfo;

main( argc, argv )
int argc;
char *argv[ ];
{
double  ra_val, ha_val, dec_val;
char    s1[ 100 ];

tinfo = get_tinfo();

if ( argc == 1 ) {
   if ( strcmp( argv[ 0 ], "zenith" ) == 0 ) {
      DES_DEC = 41.0;
      DES_HA = 0.0;
      strcpy( tinfo -> object_name, "ZENITH" );
      tinfo -> motion_type = NO_MOTION;
      log_entry( "zenith" );
   }
   else if ( strcmp( argv[ 0 ], "service" ) == 0 ) {
      DES_DEC = -35.0;
      DES_HA = 0.0;
      strcpy( tinfo -> object_name, "SERVICE" );
      tinfo -> motion_type = NO_MOTION;
      log_entry( "service" );
   }
   else {
      printf("\nUsage: \"fixed <HA> <Dec>]\"");
      printf("\n    or \"fixed <RA> <Dec> follow\"\n");
      exit( -1 );
   }
}
else if ( ( argc == 3 ) && ( strcmp( argv[ 0 ], "fixed" ) == 0 ) ) {
   sscanf( argv[ 1 ], "%lf", &ha_val );
   sscanf( argv[ 2 ], "%lf", &dec_val );
   if ( fabs( ha_val ) > 12.0 ) {
      printf( "Illegal HA value: %s\n", argv[ 1 ] );
      exit( -2 );
   }
   else if ( ( dec_val > 90.0 ) || ( dec_val < -40.0 ) ) {
      printf( "Illegal Dec value: %s\n", argv[ 2 ] );
      exit( -3 );
   }
   else {
      tinfo -> motion_type = NO_MOTION;
      DES_HA = ha_val * 15.0; 
      DES_DEC = dec_val;
      strcpy( tinfo -> object_name, "FIXED" );
      strcpy( s1, "fixed " );
      strcat( s1, argv[ 1 ] );
      strcat( s1, " " );
      strcat( s1, argv[ 2 ] );
      log_entry( s1 );
   }
}
else if ( ( argc == 4 ) && ( strcmp( argv[ 0 ], "fixed" ) == 0 )
                        && ( strcmp( argv[ 3 ], "follow" ) == 0 ) ) {
   sscanf( argv[ 1 ], "%lf", &ra_val );
   sscanf( argv[ 2 ], "%lf", &dec_val );
   if ( ( ra_val < 0.0 ) || ( ra_val > 24.0 ) ) {
      printf( "Illegal RA value: %s\n", argv[ 1 ] );
      exit( -4 );
   }
   else if ( ( dec_val > 90.0 ) || ( dec_val < -40.0 ) ) {
      printf( "Illegal Dec value: %s\n", argv[ 2 ] );
      exit( -5 );
   }
   else {
      tinfo -> motion_type = NON_SOLAR;
      DES_RA = ra_val;
      DES_DEC = dec_val;
      strcpy( tinfo -> object_name, "FOLLOW" );
      strcpy( s1, "fixed " );
      strcat( s1, argv[ 1 ] );
      strcat( s1, " " );
      strcat( s1, argv[ 2 ] );
      strcat( s1, " follow" );
      log_entry( s1 );
   }
}
else {
   printf("\nUsage: \"fixed <HA> <Dec>]\"");
   printf("\n    or \"fixed <RA> <Dec> follow\"\n");
   exit( -6 );
}
V_RA  = 0.0;
V_DEC = 0.0;
}
