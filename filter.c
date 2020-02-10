
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

struct 
   wiro_memory *tinfo;

main(int argc, char *argv[]) {
char *line[100];

tinfo =get_tinfo(); 

	if ( argc != 2) {
		printf("filter sets the filter string on the tracking screen\n");
		printf("Usage: filter string\n");
		exit(1);
	}
	strcpy( FILTER, argv[1]);
	sprintf( USER_STR[1], "Integration Time = %.3f       Filter=%s",
		tinfo->integration_time,FILTER);	
}
