/**********************************************************
header grabs stuff from the shared memory area and puts 
it into a fits header which it writes to stdout. 
It scans the command line for info: the first argument
is writen as the picture serial #, the second as the integration time,
and any third argument as the camera name
A log message is written to the logfile.
both picture and log file are in PICDIR.
NOR 12/20/95 now does NAXIS3
NOR 09/25/96 Now includes the FOCUS information.  Has to do a "few"
	loops to average the info from shared memory.  This will be obsolete
	when the high pass filter is put inline by the staff.
**********************************************************/


#include <smem.h>
#include <stdio.h>
#include "/usr/local/wiro/tracking/wirotypes.h"
#include "/usr/local/wiro/tracking/track.h"
#include "/usr/local/wiro/tracking/wiro.h"

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
	double sum_focus;
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
	int coadd;
	int hours, minutes,seconds;
	double danhours;
	char instrument[60];
	int rows, cols;
	int naxis=3,naxis3,bitpix=16;

    if ( NULL == ( tinfo = (struct wiro_memory *) 
    	smem_get("WIRO_MEMORY",sizeof(*tinfo), SM_READ | SM_WRITE ) ) ) {
              fprintf(stderr,"Unable to open shared memory \n");
              exit(-1);
    }

	if (argc < 4) {
		printf("fheader creates a fits header \n");
		printf("Usages: fheader rows cols naxis3 instrument\n");
		exit(0);
	}
	/*sscanf(argv[1],"%d",&rows);
	sscanf(argv[2],"%d",&cols);
	sscanf(argv[3],"%d",&naxis3);
/*	sscanf(argv[4],"%lf",&inttime);
	sscanf(argv[5],"%d",&serial);
	sscanf(argv[6],"%d",&coadd);*/
	sscanf(argv[4],"%s",instrument);
	strcpy(filter,FILTER);	

	sum_focus=0.0;
	for(i=0;i<10000000;i++) sum_focus+=(double)FOCUS;
	sum_focus=sum_focus/10000000;

/* Write picture # to tracker screen */
    sprintf(USER_STR[1], "Integration Time = %6d   #Coadd = %3d   Naxis3 = %3d  Filter = %2s\0",  (int) inttime,coadd,naxis3,FILTER);
	sprintf(USER_STR[2], "Picture id       = %6d   Focus  = %3.3f",serial,sum_focus);
	

/* write out the fits header  */
	sprintf(filename,"%s%d.fit",PICDIR,serial);
	/* output=fopen(filename,"w+"); */
	output=stdout;
	if (output==NULL) {
		exit(1);
	}
/*	basicheader(output,rows,cols);*/
	fitsout(output,"SIMPLE",'l','T');
	fitsout(output,"BITPIX",'i',&bitpix);
	fitsout(output,"NAXIS",'i',&naxis);
	fitsout(output,"NAXIS1",'i',&rows);
	fitsout(output,"NAXIS2",'i',&cols);
	fitsout(output,"NAXIS3",'i',&naxis3);
	fitsout(output,"SERIAL",'i',&serial);
	fitsout(output,"INTTIME",'f',&inttime);
	fitsout(output,"#COADD",'i',&coadd);
	fitsout(output,"OBJECT",'h',getobj());
	fitsout(output,"FILTER",'h',filter);	
    fitsout(output,"FOCUS",'f',&sum_focus);  	
	ra=(double) ORIG_RA;
	fitsout(output,"RA",'f',&ra);
	dec=(double) ORIG_DEC;	
	fitsout(output,"DEC",'f',&dec);
	fitsout(output,"EPOCH",'f',&EPOCH);
	raoff=3600.0*(double) OFFSET_HA;
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
/*	fitsout(output,"ORIGIN",'h',"WIRO");
	fitsout(output,"INSTRUME",'h',instrument);
	fitsout(output,"BZERO",'f',&dzero);
	fitsout(output,"BSCALE",'f',&done);
	fitsout(output,"BUNIT",'h',"ADU");
*/	fitsout(output,"END",'e',output);

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
