/* Nod:  A program which lets you nod the telescope.   */
/*       NOD A  moves to the A beam.                   */
/*       NOD B  moves to the B beam.                   */
/*       NOD    alternates beams.                      */
/*       NOD    help  prints a help screen.            */
/* Robert R. Howell   08/16/94  */

/* JSW 20Jan00  Port to Linux */

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

void help(void);

struct  wiro_memory *tinfo, *get_tinfo();

main(int argc, char *argv[])
{

char s1[98];

tinfo = get_tinfo();	/* Attach shared memory */

if (argc == 2) {
	if (strcmp(argv[1], "a") == 0 || strcmp(argv[1], "A") == 0) {
		NODDED &= ~2;		/* Clear the software nod flag. */
		if (NODDED & 1) 	/* Has hardware commanded a nod? */
			printf("\007 WARNING:  The hardware switch is forcing beam B.\n");
		if( NODDED )
			log_entry("nod is b");
		else
			log_entry("nod is a");
		return;
		}
	if (strcmp(argv[1], "b") == 0 || strcmp(argv[1], "B") == 0) {
		NODDED |=  2;		/* Set the software nod flag.  */
					/* Don't need to check hardware, because */
					/* either can command B beam.			*/
		if( NODDED )
			log_entry("nod is b");
		else
			log_entry("nod is a");
		return;
		}
	/* If not "a" or "b" print help message */
	help();
	}

if (argc == 1) {
	NODDED ^= 2;	/* Flip the software nod bit */
	if ( (NODDED & 2) == 0 && (NODDED & 1) == 1)
		printf("\007 WARNING:  The hardware switch is forcing beam B.\n");
	if( NODDED )
		log_entry("nod is b");
	else
		log_entry("nod is a");
	return;
	}
/* If none of the above arguments was found, print the help screen */
help();
}

void help(void) {
	printf("nod       alternates beams");
	printf("\nnod a     moves to the a beam");
	printf("\nnod b     moves to the b beam");
	printf("\nThe b beam will be commanded if EITHER the hardware");
	printf("\nswitch or the software switch commands \"b\".\n");
	exit(1);
}
