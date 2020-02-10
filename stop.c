/*****
 *
 * This routine will stop or start the telecsope drive strobe
 * or index the telescope
 *
 *****/

/* JSW 20Jan00  Port to Linux */

#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct 
   wiro_memory *tinfo;

main( argc, argv )

int argc;

char *argv[ ];

{
  char inchar;
  tinfo = get_tinfo();

if ( strcmp( argv[ 0 ], "g" ) == 0 ) {
   tinfo -> no_motion = 0;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "s" ) == 0 ) {
   tinfo -> no_motion = 1;
   log_entry( argv[ 0 ] );
}
   
else if ( strcmp( argv[ 0 ], "index" ) == 0 ) {
   tinfo -> dec_turns = 82;
   tinfo -> ha_turns = 3;
   tinfo -> motion_type = NO_MOTION;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "turn+" ) == 0) {
   tinfo -> ha_turns += 1;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "turn-" ) == 0) {
   tinfo -> ha_turns -= 1;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "turn+ra" ) == 0) {
   tinfo -> ha_turns += 1;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "turn-ra" ) == 0) {
   tinfo -> ha_turns -= 1;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "turn+dec" ) == 0) {
   tinfo -> dec_turns += 1;
   log_entry( argv[ 0 ] );
}

else if ( strcmp( argv[ 0 ], "turn-dec" ) == 0) {
   tinfo -> dec_turns -= 1;
   log_entry( argv[ 0 ] );
}

/* ends here */

else if ( strcmp( argv[ 0 ], "quit" ) == 0) {
   printf("Do you really want to quit tracking the telescope?\n");
   printf("Type  Y to stop tracking\n");
   scanf("%c",&inchar);
   if (( inchar == 'Y') || (inchar == 'y')) {
   	tinfo->keep_tracking = TRACK_STOP;
   	printf(" tracking has been stopped, and the track program is exiting \n");
        log_entry( argv[ 0 ] );
   }
   else {
   	 printf("tracking HAS NOT been stopped- \n");
   }
}

else
   printf( "Unknown motion command: %s\n", argv[ 0 ] );

/* end telescope */ }
