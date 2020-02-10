/* SetNod:  A program which lets you set nod offset values. */
/* Robert R. Howell   02/11/92  23:57    LynxOS version */
/* Port to Linux.  24Jan00 JSW */

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct  wiro_memory *tinfo, *get_tinfo();

main(int argc, char *argv[])
{

double new_ha, new_dec;
char s1[129];

tinfo = get_tinfo();	/* Attach shared memory */

if (argc <3) {
	printf("\nSETNOD  HAvalue.vvv DECvalue.vvv  sets the distance");
	printf(" to the B beam.");
	printf("\n   The values are in arcseconds, and W and N are positive.");
	printf("\n   The current values are %8.2lf %8.2lf\n", 
	             3600. * NOD_HA, 3600. * NOD_DEC);
	exit(1);
	}

printf("%s %s\n", argv[1], argv[2]);

sscanf(argv[1],"%lf", &new_ha);
sscanf(argv[2],"%lf", &new_dec);

new_ha /= 3600.0;
new_dec /= 3600.0;

if (fabs(new_ha) < 2.0001 && fabs(new_dec) < 2.0001) {
	NOD_HA  = new_ha;
	NOD_DEC = new_dec;
	sprintf(s1, "setnod %s %s", argv[1], argv[2]);
	log_entry(s1);
	}
else
	{
	printf("\nThe maximum value allowed is 2 degrees.");
	exit(1);
	}
}
