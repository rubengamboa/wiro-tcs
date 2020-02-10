
/* This guy handles all the dome stuff */

/* JSW 20Jan00  Port to Linux */

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
int pos;
char s1[ 100 ];

tinfo = get_tinfo();

 if (argc <2) {
   printf("No dome command given \n");
   exit(1);
 }

if ( strcmp( "on", argv[ 1 ] ) == 0 ) {
   tinfo -> dome_on = 1;
   log_entry( "dome on" );
}
else if ( strcmp( "off", argv[ 1 ] ) == 0 ) {
   tinfo -> dome_on = 0;
   log_entry( "dome off" );
}
else if ( strcmp( "init", argv[ 1 ] ) == 0 ) {
   pos = ( int ) ( ( 256.0 / 360.0 ) * tinfo -> tel_azi );
   tinfo -> dome_offset = tinfo -> dome_enc - pos;
   if ( tinfo -> dome_offset < 0 ) 
      tinfo -> dome_offset += 256;
   log_entry( "dome init" );
}
else if ( strcmp( "right", argv[ 1 ] ) == 0 )  {
   tinfo -> dome_offset += 64;
   printf("dome offset now %d \n",tinfo->dome_offset);
   while ( tinfo -> dome_offset > 255 ) {
      tinfo -> dome_offset -= 256;
     printf("adjusting: offset now %d \n",tinfo->dome_offset);
   }
   log_entry( "dome right" );
} 
else if ( strcmp( "left", argv[ 1 ] ) == 0 )   {
   tinfo -> dome_offset -= 64;
   printf("dome offset now %d \n",tinfo->dome_offset);   
   while ( tinfo -> dome_offset < 0 ) {
      tinfo -> dome_offset += 256;
     printf("adjusting: offset now %d \n",tinfo->dome_offset);
   }
   log_entry( "dome left" );
}
else
   printf( "Unknown dome command: %s\n.",argv[1]);

/* end main */ }   
