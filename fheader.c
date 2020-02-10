/**********************************************************
header grabs stuff from the shared memory area and puts 
it into a fits header which it writes to stdout. 
It scans the command line for info: the first argument
is writen as the picture serial #, the second as the integration time,
and any third argument as the camera name
A log message is written to the logfile.
both picture and log file are in PICDIR.
**********************************************************/



#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

struct  wiro_memory *tinfo;


#define pi 3.141592654
#define PICDIR "/data/new/"

#include <math.h>
#include <time.h>
#include <stdio.h>

main(argc,argv)
int argc;
char *argv[];
{
	int serial,i;
	double inttime=1.0;
	char co[30],observer[30],c1[60],filter[10],*object;
	char *getfilt(),*getobj(),*getobjtype();
	double getinttime(),dzero=0.0,done=1.0;
	FILE *state,*output,*logfile;
	double dec,ra,ha;
	double raoff,decoff,airmass;
	struct tm *gmt;
	char date[80],thetime[80],objtype[80],filename[100];
	int getobjinfo();
	int hours, minutes,seconds;
	double danhours;
	char instrument[60];
	int rows, cols;

	tinfo = get_tinfo();

	if (argc < 5) {
		printf("fheader creates a fits header \n");
		printf("Usages: fheader rows cols inttime serial instrument\n");
		exit(0);
	}
	sscanf(argv[1],"%d",&rows);
	sscanf(argv[2],"%d",&cols);
	sscanf(argv[3],"%lf",&inttime);
	sscanf(argv[4],"%d",&serial);
	sscanf(argv[5],"%s",instrument);
	strcpy(filter,FILTER);	

/* Write picture # to tracker screen */
    sprintf(USER_STR[1], "Integration Time = %.2f                             Filter = %s\0", tinfo -> integration_time,FILTER);
	sprintf(USER_STR[2],"Picture id = %6d",serial);
	

/* write out the fits header  */
	sprintf(filename,"%s%d.fit",PICDIR,serial);
	/* output=fopen(filename,"w+"); */
	output=stdout;
	if (output==NULL) {
		exit(1);
	}
	basicheader(output,rows,cols);
	fitsout(output,"SERIAL",'i',&serial);
	fitsout(output,"INTTIME",'f',&inttime);
	fitsout(output,"OBJECT",'h',getobj());
	fitsout(output,"FILTER",'h',filter);	
	ra=(double) ORIG_RA;
	fitsout(output,"RA",'f',&ra);
	dec=(double) ORIG_DEC;	
	fitsout(output,"DEC",'f',&dec);
	fitsout(output,"EPOCH",'f',&EPOCH);
	raoff=-3600.0*(double) OFFSET_HA;
	fitsout(output,"OFFRA",'f',&raoff);
	decoff=3600.0*(double) OFFSET_DEC;
	fitsout(output,"OFFDEC",'f',&decoff);
	airmass= ( (double) 1.0 )/sin( (double) ALT*pi/180.0);
	fitsout(output,"AIRMASS",'f',&airmass);
  	ha=(double) HA;
	fitsout(output,"HA",'f',&ha);
	sprintf(date,"%02d/%02d/%02d",MONTH,DAY,YEAR);
	fitsout(output,"DATE-OBS",'h',date);
	danhours=UT_TIME;
	hours=(int) danhours;
	danhours=60.0 * (danhours -(double) hours);
	minutes=(int) danhours;
	danhours=60.0 * (danhours -(double) minutes);
	seconds=(int) danhours;
	sprintf(thetime,"%02d:%02d:%02d",hours,minutes,seconds);
	danhours=60.0 * (danhours -(double) minutes);
	fitsout(output,"TIME",'h',thetime);
/*	fitsout(output,"OBSERVER",'h',observer);
 	fitsout(output,"OBJTYPE",'h',objtype);  */
	fitsout(output,"ORIGIN",'h',"WIRO");
	fitsout(output,"INSTRUME",'h',instrument);
	fitsout(output,"BZERO",'f',&dzero);
	fitsout(output,"BSCALE",'f',&done);
	fitsout(output,"BUNIT",'h',"ADU");
	fitsout(output,"COMMENT",'c',"Version 2. header");
	fitsout(output,"COMMENT",'c',
	"reports RA offset instead of HA offset for keyword OFFRA"); 
	fitsout(output,"END",'e',output);

/* open the log file if desired */
		logfile=fopen("/data/new/log.dat","a");
		if (logfile == NULL) {
			fprintf(stderr,"Cannot open /data/new/log.dat - exiting\n");
			exit(1);
		}
		fprintf(logfile,
			"*1 %-19s | %6d | %-10s| %7.3lf | %-8s | %8s |\n",	
			getobj(),serial,filter,inttime, thetime, date);
		fprintf(logfile,"*2        %-10.7lf | %-10.7lf | %4.3lf |\n",	
			 ra,dec,airmass);
		fprintf(logfile,"*3        %-5.1lf      | %-5.1lf      | \n\n",
			raoff,decoff);
	fclose(logfile);
}


/*****************************************/
/* basicheader prints out a basic header */
/*****************************************/
basicheader(outfile,rows,cols)
FILE *outfile;
int rows,cols;
{		char true='T';
		int two=2,sixteen=16;

		fitsout(outfile,"SIMPLE",'l',&true);
		fitsout(outfile,"BITPIX",'i',&sixteen);
		fitsout(outfile,"NAXIS",'i',&two);
		fitsout(outfile,"NAXIS1",'i',&rows);
		fitsout(outfile,"NAXIS2",'i',&cols);
}


/*********************************************/
/* getobj returns a pointer to the name of  */
/* the current object                        */
/*********************************************/

char* getobj()
{
	static char name[90];

	strcpy(name,OBJECT_NAME);
	name[19]=0;
	return(name);
}



/*******************************************/
/*   frontandback deletes leading and      */
/*   trailing blanks                       */
/*******************************************/

char *frontandback(fil)
char *fil;
{
	int i;
	if (strlen(fil) ==0)
		return(fil);
	for (i=strlen(fil)-1 ; fil[i] == ' ';i--) fil[i]='\0';
	while(fil[0] == ' ') fil++;
	return(fil);
}
