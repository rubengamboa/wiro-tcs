/* This program dumps a corefile with reasonable values */ 
/* Howell  Aug. 15, 1994   Added nod constants.  */
/* Howell  Aug. 17, 1994   Added zero of dial constants. */
/* Spillar April 5 1999    Switch to Linux shared memory */

#include "wirotypes.h"   /* Does wiro typedefs and system dependencies */
#include <stdio.h>
#include "worm_corr.h"    /* contains worm error numbers           */
#include "track.h"        /* global function prototypes */
#include "GPIBports.h"    /* contains GPIB interface info          */
#include "wiro.h"         /* contains all the global variable defs.*/

struct wiro_memory tinfodata;  /* Allocate space for structure */
struct wiro_memory *tinfo;     /* Create far pointer to it.    */
FILE *core;

main(argc,argv)
{

	printf("Warning: running this program will overwrite the current\n");
	printf("state of the telescope: position, offsets, dial, col \n ");
	printf("write down any numbers you want to use, and check\n");
	printf("to be sure track IS NOT running.\n");
	printf("Do you wish to continue (y)?\n");

	if ( 'y' != getc(stdin)) exit(1); 

	tinfo = get_tinfo();

	DES_HA  = 0.;
	DES_RA  = 0.;
	DES_DEC = 0.;

	COL_DEC = 0.;                /* from file? */
	COL_HA  = 0.;
	
	tinfo->dial_dec = 0.;
	tinfo->dial_ha  = 0.;

	OFFSET_DEC   = 0.;
	OFFSET_HA    = 0.;
	OFFSET_2_DEC = 0.;
	OFFSET_2_HA  = 0.;
	NOD_DEC   = 20./3600.;
	NOD_HA    =  0.;

	NODDED = 0;

	ORIG_DEC = 0.;
	ORIG_RA  = 0.;

	ABER_NUT_RA  = 0.;
	ABER_NUT_DEC = 0.;
	HP           = 0.;

	EPOCH = 1950.;

	tinfo->ha_enc  = 0;
	tinfo->dec_enc = 0;

	tinfo->ha_turns  = 0;
	tinfo->dec_turns = 0;

	tinfo->ha_rate  = 0;
	tinfo->dec_rate = 0;

	tinfo->ha.whole  = 0;
	tinfo->dec.whole = 0;

	tinfo->paddle_rate = 0.0;
	tinfo->status = 0;

	TEMPERATURE = 0.0;
	PRESSURE    = 26.0;
	
	V_RA  = 0.0;
	V_DEC = 0.0;

	HA_SERVO=7500.; /* Rate = 1/2 max when encoder error = this */
	DEC_SERVO = 7500.;

	PAD_V_RA = 0.0;
	PAD_V_DEC = 0.0;

	tinfo->keep_tracking = TRACK_GO;
	tinfo->motion_type = NO_MOTION;
	tinfo->no_motion = 1;

	tinfo->dome_enc = 0;
	tinfo->dome_offset = 0;

	
	strcpy( tinfo->object_name, "Spillar");
	log_entry( "makecorefile" );
}


