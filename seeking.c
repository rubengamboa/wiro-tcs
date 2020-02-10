/* seeking waits for the telescope to get close to the commanded
position.  It peeks at the errors and waits for them to both
be under an arcsec for 0.2 seconds. */

#include <stdio.h>

main()
{
	thereyet(); /* thereyet is in babe_util.c */
	printf("\nONIT\n");
}

