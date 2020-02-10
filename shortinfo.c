/*------- info prints out the current position ----------------*/
#include <math.h>
#include <time.h>
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"
#define pi 3.141592654


struct  wiro_memory *tinfo, *get_tinfo();

main(argc, argv)
int argc;
char *argv[];
{
	double raoff, decoff, airmass, danhours;
	int hours, minutes, seconds;
	int flag=1;
/* attach shared tracking memory */
	
	tinfo = get_tinfo();

	airmass = ( (double) 1.0) / sin( (double) ALT*pi /180.0);
	printf("%4.3lf ",airmass);

	danhours=ORIG_RA;
	hours=(int) danhours;
	danhours = 60.0 * (danhours - (double) hours);
	minutes = (int) danhours;
	danhours = 60.0 * (danhours - (double) minutes);
	seconds=(int) danhours;

	printf(" %02d:%02d:%05.2lf",hours,minutes,danhours);

	danhours= ORIG_DEC;
	if (danhours < 0){
		danhours = -danhours; 
		flag = -1;
	}
	hours = flag * (int) danhours;
	danhours = 60.0 * (danhours - (double) hours);
	minutes = (int) danhours;
	danhours = 60.0 * (danhours - (double) minutes);
	if (danhours <= 0)  danhours = - danhours;
	seconds=(int) danhours;
	if (hours >0 )
		printf(" +%2d:%02d:%05.2lf ",hours,minutes,danhours);
	else
		printf(" %3d:%02d:%05.2lf ",hours,minutes,danhours);

	decoff = 3600.0 * (double) OFFSET_DEC;
	raoff = 3600.0 * (double) OFFSET_HA;
	printf("%5.1lf%5.1lf",raoff,decoff);
	printf(" %lf",JULIAN);
	printf(" %s\n ",OBJECT_NAME);
}

/***********Front and back deletes leading and trailing blanks *********/

char *frontandback(fil)
char *fil;
{
	int i;
	if (strlen(fil) == 0 )
		return(fil);
	for( i = strlen(fil)-1; fil[i] == ' ';i--) fil[i]='\0';
	while(fil[0] == ' ') fil++;
	return(fil);
}

