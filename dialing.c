/* Dialing is used to observer a pair of stars, one near the          */
/*  celestial equator and another at Dec ~ +60.  From the offsets     */
/*  recorded at these two stars, the program recalculates the         */
/*  dialing and coll corrections.                                     */
/*  Dialing ?   or calling dialing in the wrong sequence will         */
/*  cause a help screen to be printed.                                */
/* Robert R. Howell   02/26/91  00:32                                 */
/* Modifications:						      */
/* 96_07_01   Howell:   Modified data file so that it always lives    */
/*                      in the "/data/" directory.  This avoids       */
/*                      problems when the first dialing is run in     */
/*                      one directory and the second or third is run  */
/*                      in a different one.                           */
/*			Also modified log output to record new        */
/*			coll and dial values.                         */
/* 00_01_24   Weger:    Port to Linux.  Data file placed at           */
/*                      "/home/observer/wiro/catalogs/dialing.dat"    */

/* Format is:  index#  HA  DEC  HA_offset  DEC_offset  Object_Name    */
/* The index number recorded is just 0.  You can change it later.     */
/* The HA is decimal hours, the DEC is decimal degrees, and the       */
/* offsets are true decimal arcseconds.  + means the telescope is     */
/* North and West of the originally set position.                     */


/* dirflag is an int.  which tells what parameter was specified.     */
/* 0 Means no flag was specified.         */
/* 1 means the S or s flag was specified. */
/* 2 means the N or n flag was specified. */

#include <stdlib.h>
#include <stdio.h>
#include <termio.h>
#include <math.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct 
   wiro_memory *tinfo;



void gotostar();
void findstars();

char fname[200]; /* ="/usr/local/catalogs/dialing.dat";*/ /*  Temporary data file to use.         */
FILE *datfile;               /* Pointer to input file               */
int  dirflag = 0;            /* 1/2 means do S/N star first         */
int  seqflag = 0;            /* Keeps track of sequence of calls:   */
                             /*  0  Go to 1st star                  */
                             /*  1  Record 1st offset & go to 2nd star. */
                             /*  2  Use 2nd offset to update pointing.  */
char nstar[80],sstar[80];    /*  Names of N and S stars.            */

main(argc, argv )
int  argc;
char *argv[];

{

  strcpy(fname, CATALOGS);
  strcat(fname, "dialing.dat");
	
/* First check for various illegal parameters. */

if (argc>2) {
    help();
    exit(-1);
    }

if  (argc==2) {
    if ( (strcmp(argv[1],"s")==0) || (strcmp(argv[1],"S")==0) ) dirflag = 1;
    if ( (strcmp(argv[1],"n")==0) || (strcmp(argv[1],"N")==0) ) dirflag = 2;
    }

if (argc==2 && dirflag==0) {          /* Illegal parameter */
    help();
    exit(-1);
    }

init();         /* Get ready to talk to WIRO */

if (dirflag>0)  star_1();
else {
    datfile = fopen(fname, "r");
    if (datfile==NULL) {
        help();
        exit(-1);
    }
    fscanf(datfile,"%d %d %20s %20s",&seqflag,&dirflag,nstar,sstar);
    if (seqflag==0) star_2();
    else if (seqflag==1) star_3();
         else {
              help();
              exit(-1);
              }
    }
}

/************* Use this routine for the first call. ***************/
/*** It intializes the data file and goes to the first star.  *****/
star_1()
{
  int i;
  /* Select dialing catalog           */
  if (i=(int) system("catalog dialing.cat")) {
    printf("system call returns %d\n",i);
    printf("Unable to select \"dialing.cat\" catalog.");
    exit(-1);
  }
  else {
    findstars();                 /* Find which pair of stars to use. */
  }

  if   (dirflag==1) gotostar(sstar);
  else              gotostar(nstar);

  seqflag = 0;
  datfile = fopen(fname,"w");   /* Now initialize data file.        */
  fprintf(datfile,"%5d %5d %20s %20s\n",seqflag,dirflag,nstar,sstar);
  fclose(datfile);
}

/************* Use this routine for the second call. ***************/
/*** It records the offsets on the first star, and goes to the second. */
star_2()
{
    double ha0, dec0;            /* Position of star 0 */
    double ha1, dec1;            /* Position of star 1 */
    double haoff0, decoff0;      /* Offsets for star 0 */
    double haoff1, decoff1;      /* Offsets for star 1 */



    seqflag = 1;

    ha0  = DES_HA /15. ;         /* Desired HA  for star 0, in hours    */
    dec0 = DES_DEC;              /* Desired DEC for star 0, in degrees  */
    haoff0  = 3600.*OFFSET_HA;   /* HA Offset,  in arcseconds */
    decoff0 = 3600.*OFFSET_DEC;  /* DEC Offset, in arcseconds */

    fclose(datfile);
    /* Rewrite the file */
    datfile = fopen(fname,"w");
    fprintf(datfile,"%5d %5d %20s %20s\n",seqflag,dirflag,nstar,sstar);
    fprintf(datfile, "%10.4lf %10.4lf %6.1lf   %6.1lf\n",
                        ha0,   dec0,   haoff0, decoff0);
    fclose(datfile);

/* Now go to the next star */
    if (dirflag==2) gotostar(sstar);
    else            gotostar(nstar);
}

/************* Use this routine for the third call. ***************/
/*** It records the offsets on the second star then computes corrections. */
star_3()
{
    double ha0, dec0;            /* Position of star 0 */
    double ha1, dec1;            /* Position of star 1 */
    double haoff0, decoff0;      /* Offsets for star 0 */
    double haoff1, decoff1;      /* Offsets for star 1 */
    double cosd0, cosd1;         /* Cos. of dec for both stars */
    double deccoll, hacoll;      /* New coll values.           */
    double dial;                 /* New dial values            */
    double odeccoll, ohacoll;    /* Old coll values            */
    double odial;                /* Old dial values            */
    char   response;             /* Response to prompt.        */
	char   log_message[98];	 /* Message to place in log file */


    seqflag = 2;
    ha1  = DES_HA / 15. ;        /* Desired HA  for star 1, in hours    */
    dec1 = DES_DEC;              /* Desired DEC for star 1, in degrees  */
    haoff1  = 3600.*OFFSET_HA;   /* HA Offset,  in arcseconds */
    decoff1 = 3600.*OFFSET_DEC;  /* DEC Offset, in arcseconds */

    ohacoll  = 3600. * COL_HA;
    odeccoll = 3600. * COL_DEC;
    odial    = 3600. * tinfo -> dial_ha;

    /* Read the previous offset information */
    fscanf(datfile, "%lf %lf %lf %lf",
                       &ha0,   &dec0,   &haoff0, &decoff0);
    fclose(datfile);

    /* Now compute the new coll and dial values.   Note I am not
       certain about the sign the current program uses, or the units. */

    deccoll = (decoff0 + decoff1)/2.   + odeccoll;   /* Use average coll. */

    cosd0 = cos(dec0 / 57.29578);
    cosd1 = cos(dec1 / 57.29578);

    dial    = (haoff1 - haoff0) / ( cosd1 - cosd0);
    hacoll  =  haoff0 - dial * cosd0;
    dial   += odial;
    hacoll += ohacoll;

    /* Now check to be sure these should actually be used.  */

    printf("\nDial     changes from %6.1lf  to: %6.1lf", odial,    dial);
    printf("\nHA  Coll changes from %6.1lf  to: %6.1lf", ohacoll,  hacoll);
    printf("\nDEC Coll changes from %6.1lf  to: %6.1lf", odeccoll, deccoll);
    printf("\nDo you want to use these new values?  (Y/N): ");
    scanf("%1c",&response);

/*  if (    (strcmp(response,"y")==0)
         || (strcmp(response,"Y")==0) ) { */
    if (response=='Y' || response=='y') {
         COL_HA  = hacoll  / 3600.;              /* Set the new values */
         COL_DEC = deccoll / 3600.;
         tinfo -> dial_ha = dial / 3600.;
         OFFSET_HA  = 0.;
         OFFSET_DEC = 0.;
         printf("New values for coll & dial accepted.");
         sprintf( log_message, 
				  "dialing:  HA Coll: %10.2lf  DEC Coll:  %10.2lf  Dial: %10.2lf",
				  COL_HA * 3600.0, COL_DEC * 3600.0 , tinfo -> dial_ha * 3600.0);
	 	 log_entry(log_message);
         }
    else {
         printf("New values for coll & dial discarded.");
         }
     printf("\n\nYou are now in the \"dialing\" catalog.  You probably want");
     printf(  "\nto call \"catalog\" to reselect your own.\n");

    /* Rewrite all of the information in the file */
    datfile = fopen(fname,"w");
    fprintf(datfile,"%5d %5d %20s %20s\n",seqflag,dirflag,nstar,sstar);
    fprintf(datfile, "%10.4lf %10.4lf %6.1lf   %6.1lf\n",
                        ha0,   dec0,   haoff0, decoff0);
    fprintf(datfile, "%10.4lf %10.4lf %6.1lf   %6.1lf\n",
                        ha1,   dec1,   haoff1, decoff1);
    fclose(datfile);
}


/**************************************************************************/
init()
{
tinfo = get_tinfo();
}

/***************************************************************************/
help()
{ printf("\nYou didn't initialize dialing properly.\n" );
  printf("\nDIALING:  is a program which computes new coll and dial ");
  printf("\n          corrections based on observations of one star near");
  printf("\n          the equator and another star near Dec = +65.");
  printf("\n");
  printf("\n          The first time you call dialing, follow it with N or S");
  printf("\n          to select which star to do first and to tell it you are");
  printf("\n          starting a new sequence.  It will move the telescope to");
  printf("\n          the star.  Center the star, then call dialing again to ");
  printf("\n          record the offsets. It will go to the other star.  After"); 
  printf("\n          centering that star, call dialing a third time.  It will ");
  printf("\n          compute the new coll and dial coefficients.\n" );
}

/***************************************************************************/
/* Go to the selected star, after requesting confirmation */
void gotostar(sname)
char sname[];
{
    char syscmd[80];
    char response;
    while (1) {
        printf("OK to go to star %s?  (Y/N): ",sname);
        scanf("%1c", &response);
        if (response=='Y' || response=='y') break;
        if (response=='N' || response=='n') {
            printf("Dialing aborted.\n");
            exit(-1);
            }
        }
                                  /* Compose "follow starname"        */
    strcpy(syscmd, "follow ");    
    strcat(syscmd, sname);
    system(syscmd);               /* Seek to star.                    */
}


/***************************************************************************/
/*   Find names of N & S stars, based on LST */
void findstars()
{

int spair;
char nstars[24][9];
char sstars[24][9];

strcpy(nstars[ 0], "10978");
strcpy(nstars[ 1], "11482");
strcpy(nstars[ 2], "12031");
strcpy(nstars[ 3], "12704");
strcpy(nstars[ 4], "12969");
strcpy(nstars[ 5], "13351");
strcpy(nstars[ 6], "13788");
strcpy(nstars[ 7], "13986");
strcpy(nstars[ 8], "14456");
strcpy(nstars[ 9], "14796");
strcpy(nstars[10], "15135");
strcpy(nstars[11], "15384");
strcpy(nstars[12], "15606");
strcpy(nstars[13], "15941");
strcpy(nstars[14], "16273");
strcpy(nstars[15], "16558");
strcpy(nstars[16], "16962");
strcpy(nstars[17], "17365");
strcpy(nstars[18], "17828");
strcpy(nstars[19], "18222");
strcpy(nstars[20], "18676");
strcpy(nstars[21], "19019");
strcpy(nstars[22], "19827");
strcpy(nstars[23], "20268");

strcpy(sstars[ 0], "128513");
strcpy(sstars[ 1], "109627");
strcpy(sstars[ 2], "110291");
strcpy(sstars[ 3], "110920");
strcpy(sstars[ 4], "111579");
strcpy(sstars[ 5], "112142");
strcpy(sstars[ 6], "113271");
strcpy(sstars[ 7], "115456");
strcpy(sstars[ 8], "116569");
strcpy(sstars[ 9], "117264");
strcpy(sstars[10], "118044");
strcpy(sstars[11], "118648");
strcpy(sstars[12], "119213");
strcpy(sstars[13], "119674");
strcpy(sstars[14], "120238");
strcpy(sstars[15], "120809");
strcpy(sstars[16], "121218");
strcpy(sstars[17], "121962");
strcpy(sstars[18], "123013");
strcpy(sstars[19], "124068");
strcpy(sstars[20], "125235");
strcpy(sstars[21], "126643");
strcpy(sstars[22], "127340");
strcpy(sstars[23], "127934");


spair = (int) (LST+0.5);
if (spair >= 24) spair =  0;

strcpy(nstar, nstars[spair]);   /* Get name & coord. for N star */
strcpy(sstar, sstars[spair]);   /* Get name & coord. for S star */
}




