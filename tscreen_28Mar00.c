/* tscreen.c                July 29, 1991      Robert R. Howell  */

/* Modified to run in a NeXT window; 6 Sept 91   esp                */
/* July 12, 1994  R. Howell
	  Eliminated output of Dec Dial.  It is not meaningful.    */
/* RRH:  08/15/94  Added nod indicator */

/* I need to avoid excess output, and only have it output variables */
/* which have changed.  To do this, I will keep a copy of each      */
/* output variable, and examine it for a change before outputing    */
/* it.  Note that I'm not going to spend the time checking rounding */
/* before doing this.  Therefore, any change, even if it does not   */
/* result in a change in visible characters, will result in output. */

typedef char  * lpchar;      /* long pointer to character */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "wirotypes.h"
#include "track.h"
#include "wtrack.h"         /* This must come before wiro */
#include "wiro.h"

extern struct wiro_memory *tinfo;     /* Pointer to shared tracking info */
int  scount=0;               /* Count calls to screen routine.  */

int number_terminals=0;      /* the number of active tracker screens */
FILE *screen_stream[10];     /* up to 20 active tracker screens are possible */

/* Prototypes of the local functions */


void clear_screen(void);
void temit(char c);
void ttype(char *cptr);
void cursor(int row, int column);
void get_cursor(int *row, int *column);
void dblout(int row, int column, int fld, int dpl, double x);
void hmsout(int row, int column, int fld, int dpl, double x);
void intout(int row, int column, int fld, int i);
void chrout(int row, int column, lpchar cptr);

/* Initialize serial port       */
int register_port(char* name);         /* open a stream to write screen data too */
void   serial_snd(unsigned char);         /* Send serial data             */
int  serial_ocount(void);               /* # of chr. waiting for output */
unsigned char serial_rcv(void);           /* Get serial data              */

unsigned int  tsrsize;
int ibusy = 0;              /* True if interrupt routine is busy */
int orow, ocolumn;          /* Cursor position when routine starts.    */
int   tmp;
char  serial_buffer[2048];    /* Buffer for serial port data.  */


void serial_snd(unsigned char a) 
{
  int i;
  for (i=0; i<number_terminals; i++) putc(a,screen_stream[i]); 
  

}


/***** FINALLY, THESE ARE THE SUBROUTINES WHICH DO ALL THE WORK ***********/

/*  First comes some code which helps check whether a number is changed   */
/*     and needs to be printed.                                           */
/*  Second comes a set of routines used to output various types of        */
/*     numbers in standard formats.                                       */
/*  Third  comes a screen initialization routine which clears the         */
/*     tracker screen then outputs fixed "label" information.             */
/*  Fourth is the main screen output routine.                             */
/*  Fifth is a set of DOS specific routines used for hardware dependent   */
/*     functions such as cursor control and actual character output.      */

/****************** Does a number need to be printed?   ****************/
/*  To avoid outputing numbers which haven't changed, each double      */
/*  and integer value which is output by hmsout, dblout, or intout     */
/*  is stored in an array.  The number is output only if it has        */
/*  changed since the last time it was output, or if the "printall"    */
/*  variable is nonzero.  Two arrays hold the old values, and two      */
/*  indices are used to find which variable is being output.  The      */
/*  indices are zeroed at the start of the main print routine, and     */
/*  and are incremented each time a number is output.  Note that this  */
/*  requires that the different quantities always be output in         */
/*  the same order.                                                    */
/*  The first time through the loop printall is set to 1.              */

int    printall;        /* Print all variable numbers if this is nonzero */

int    dindx;           /* Index into the old double array.             */
double dval[100];       /* An array which holds the numbers output.     */

int    iindx;           /* Index into the old integer array.            */
int  ival[100];       /* An array which holds the numbers output.     */

int    strindx;		    /* Index into the old string array.            */
char  strval[10][80];   /* An array to hold user string output.        */
/******************** Output routines for various formats. *****************/
/*                                                                     */
/* The following routines output various format numbers, at the        */
/* specified location on the tracker screen.  The other parameters are */
/* fld :     field width                                               */
/* dpl :     number of decimal places.                                 */

/* Double precision floating point output */
void dblout(int row, int column, int fld, int dpl, double x)
{
	char format[80];
	char outstr[80];
	if ( !printall &&  (dval[dindx] == x) )     /* Need to reprint? */
	{ 
		dindx++;
		return;
	}
	dval[dindx++] = x;
	cursor(row, column);
	sprintf(format, "%c%u.%ulf", '%', fld, dpl);
	sprintf(outstr, format, x);
	ttype(outstr);
}

/* Double precision floating point, printed as hh:mm:ss.ss */
void hmsout(int row, int column, int fld, int dpl, double x)
{
	char format[80];
	char hmsstr[81];
	if ( !printall &&  (dval[dindx] == x) )     /* Need to reprint? */
	{ 
		dindx++;
		return;
	}
	dval[dindx++] = x;
	if (fld > 80) fld = 80;
	cursor(row, column);
	strncpy(hmsstr, hr_hms(x, fld, dpl), fld+1);
	ttype(hmsstr);
}

/* 32 bit integer output */
void intout(int row, int column, int fld, int i)
{
	char format[80];
	char outstr[80];
	if ( !printall &&  (ival[iindx] == i) )     /* Need to reprint? */
	{ 
		iindx++;
		return;
	}
	ival[iindx++] = i;
	cursor(row, column);
	sprintf(format, "%c%uld", '%', fld);
	sprintf(outstr, format, i);
	ttype(outstr);
}

/* string output */
void cchrout(int row, int column, lpchar cptr)
{
	if ( !printall && (0==strcmp(cptr,strval[strindx]))) /* Need to reprint? */
	{ 
		strindx++;
		return;
	}
	strcpy(strval[strindx++],cptr);
	cursor(row, column);
	ttype(cptr);
}
/* character output */
void chrout(int row, int column, lpchar cptr)
{
	cursor(row, column);
	ttype(cptr);
}



/****************** Set up the tracker screen *****************************/
void tscreen_init( void )
{
	int i;
	clear_screen();
	printall = 1;                   /* Print everything */

	chrout( 1,  1, "OBJECT");    /* Output labels    */
	chrout( 1, 60, "UT");
	chrout( 1, 40, "LST");
	chrout( 1, 73, "DATE");

	chrout( 4, 26, "R.A.");
	chrout( 4, 39, "DEC");
	chrout( 4, 49, "H.A.");
	chrout( 4, 60, "DOME");
	chrout( 4, 70, "AIRMASS");

	chrout( 8, 16, "H.A.");
	chrout( 8, 29, "DEC");

	chrout( 9, 60, "TELESCOPE:");
	chrout(10, 60, "DOME:");

	chrout( 5,  1, "ACTUAL    J(date)");
	chrout( 6,  1, "DESIRED   J(date)");
	chrout( 7,  1, "Astrometric J1950"); 
	chrout( 9,  1, "OFFSET");
	chrout(10,  1, "   NOD");
	chrout(11,  1, "   COL");
	chrout(12,  1, "  DIAL");
	chrout(13,  1, " RATES");
	chrout(14,  1, "ERRORS");
}

/****************** Update the tracker screen ******************************/

void tminn( void )
{
	int i;                                        /* loop index.     */
	float ftmp;

	iindx = 0;                /* Indices for reprint? tests */
	dindx = 0;
	strindx =0;

	hmsout( 5, 20, 13, 2, RA);                    /* actual position */
	hmsout( 5, 33, 12, 1, DEC);
	hmsout( 5, 46, 11, 1, HA / 15. );

	hmsout( 9, 10, 12, 1, OFFSET_HA  );  /* Offsets in " */
	hmsout( 9, 23, 12, 1, OFFSET_DEC );

	if (NODDED) { 						/* Has telescope been nodded? */
		hmsout(10, 10, 12, 1, NOD_HA  );
		hmsout(10, 23, 12, 1, NOD_DEC );
		cchrout(10, 45, "B Beam");
		}
	else{
		hmsout(10, 10, 12, 1, 0.      );
		hmsout(10, 23, 12, 1, 0.      );
		cchrout(10, 45, "A Beam");
		}

	hmsout(14, 10, 12, 1, DES_HA - HA   );        /* Errors */
	hmsout(14, 23, 12, 1, DEC - DES_DEC );        /* USE DD:MM:SS for both */


	if ( fabs(DES_HA - HA) > 5.0 / 3600. )  {
		cchrout( 9, 36, "XXXXXXXX");
		cchrout(10, 36, "XXXXXXXX");
	}
	else if ( fabs(DES_HA - HA) > 1.0 / 3600. )  {
		cchrout( 9, 36,  "........");
		cchrout(10, 36, "........");
	}
	else {	
		cchrout( 9, 36,  "        ");
		cchrout(10, 36, "        ");
	}

	if ( fabs(DES_DEC - DEC) > 5.0 / 3600. )  {
		cchrout(12, 36, "XXXXXXXX");
		cchrout(13, 36, "XXXXXXXX");
	}
	else if ( fabs(DES_DEC - DEC ) > 1.0 / 3600. )  {
		cchrout(12, 36, "........");
		cchrout(13, 36, "........");
	}
	else { 
		cchrout(12, 36, "        ");	
		cchrout(13, 36, "        ");	
	}
		
	hmsout( 2, 34, 11, 1, LST);
	hmsout( 2, 54, 11, 1, UT_TIME);

	intout( 2, 69, 4, YEAR);
	intout( 2, 74, 2, MONTH);
	intout( 2, 77, 2, DAY);
	
	cchrout(16,  1, USER_STR[0]);
	cchrout(17,  1, USER_STR[1]);
	cchrout(18,  1, USER_STR[2]);
	cchrout(19,  1, USER_STR[3]);
	cchrout(20,  1, USER_STR[4]);
	cchrout(21,  1, USER_STR[5]);
	cchrout(22,  1, USER_STR[6]);


/*	for (i=0; i<20; i++) 
		if ( OBJECT_NAME[i] == NULL ) OBJECT_NAME[i]=' '; */
	strcat(OBJECT_NAME,"                ");	
	OBJECT_NAME[20] = 0;
	cchrout( 2,  6, OBJECT_NAME);     

	/* for(i=strlen(OBJECT_NAME); i<20; i++) temit(' ');  */

	/* Now compute and output actual dome position */
	tmp = (int)  ((int) tinfo->dome_enc - (int) tinfo->dome_offset);
	if      (tmp > 255)  tmp -= 256;
	else if (tmp <   0)  tmp += 256;
	tmp = (int) ( tmp * 360.0 / 256.0 );
	intout( 5, 60,  4,  tmp);

	hmsout( 6, 20, 13, 2, DES_RA);                /* wanted */
	hmsout( 6, 33, 12, 1, DES_DEC);
	hmsout( 6, 46, 11, 1, DES_HA / 15.);
	intout( 6, 60,  4, (int) (AZI+0.5) ) ;
	ftmp = sin( RAD( ALT ) );
	if (ftmp == 0) ftmp = 0.001;
	ftmp = 1/ftmp;	
	dblout( 6, 72,  5, 2, ( ftmp  ) );  /* more elaborate later? */

	hmsout(11, 10, 12, 1, COL_HA );               /* Col */
	hmsout(11, 23, 12, 1, COL_DEC);

	hmsout(12, 10, 12, 1, tinfo->dial_ha);        /* Dial */
/*	hmsout(12, 23, 12, 1, tinfo->dial_dec); */

	/* Print rates as arcseconds/second.  The RA rates are really only */
	/* this at DEC=0.  The printed numbers are actually 15 * (s/s)     */
	hmsout(13, 10, 12, 1, ( V_RA * cos( rad( DEC ) ) * 15./3600. ) );
	hmsout(13, 23, 12, 1, ( V_DEC / 3600. ) );

	if (tinfo->no_motion)                         /* Drives enabled? */
		cchrout( 9, 72, "OFF");         /* Logic is opposite of dome_on */
	else
		cchrout( 9, 72, "ON ");

	if (tinfo->dome_on)                          /* Dome enabled */
		cchrout(10, 72, "ON ");
	else
		cchrout(10, 72, "OFF");

	printall = 0;
}




/**************** Clear the screen ******************/
void clear_screen()
{
	/* Erase ANSI terminal   */
	{
		serial_snd( 27);
		serial_snd('[');
		serial_snd('2');
		serial_snd('J');
	}
}



/*************** Emit a character to the tracker screen ******************/
/*                  either the console or the terminal.            *******/

void temit(char chr)
{
  int i;
  for (i=0; i<number_terminals; i++) putc(chr,screen_stream[i]); 


}

/*************** Emit a string to the tracker screen ********************/
void ttype(char *cptr)
{
        int i;
	while (*cptr != '\0') temit( *(cptr++) );
	for(i=0; i<number_terminals; i++) fflush(screen_stream[i]);


}

/*************** Position the cursor at row 1-25, column 1-80 ************/
void cursor(int row, int column)
{
	char ctemp[80];
	int  i;

	{
	        serial_snd( 27);
		sprintf(ctemp, "[%d;%dH\000", row, column);
		i=0;
		while ( ctemp[i] != '\0')    serial_snd ( ctemp[i++] );
	}

}





