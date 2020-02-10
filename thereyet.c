
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

struct 
   wiro_memory *tinfo;


#define DELAY 300000
/* 300 millisecond delay between trials */
#define TRIES 3
/* times error must be small before OK */
#define TOLERENCE 1.0
/* tolerence in arcsec */

int thereyet()
{	double errorra, errordec,error,fudge; 
	int i=TRIES;

	tinfo =get_tinfo();

	do {
		usleep(DELAY);	

        errorra = 3600.0*(DES_HA /15.0 - HA / 15.0);

		errordec = 3600.0* (DES_DEC - DEC);
		error=errorra*errorra+ errordec*errordec;

		if (error < TOLERENCE*TOLERENCE) i--;
		else i=TRIES;
	} while (i>0);
}
