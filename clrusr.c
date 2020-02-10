#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

struct 
   wiro_memory *tinfo;

struct wiro_memory
   *tracking_info;
   
main( )
{
int i;

tinfo = get_tinfo();

for( i = 0; i < 7; i++ ) {
   USER_STR[ i ][ 79 ] = '\0';
   memset( USER_STR[ i ], ' ', 79 );
}

sleep(1);


for( i = 0; i < 7; i++ ) 
   USER_STR[ i ][ 0 ] = '\0';

/* end main */ }
