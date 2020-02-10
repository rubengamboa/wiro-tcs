/*  ntrack.c 
Code to track the WIRO 2.3m telescope under Linux.          

This program keeps time, reads the encoders, and computes and   
outputs drive rates to the servo motors.  It also contains     
the shared memory array.                                       

History: 

Original  Masscomp code by Dan Dumbrill 1986-1988 

Port to PC by Bob Howell                1991      
TRACK              August 4, 1991  03:45    Robert R. howell    

Port to LynxOS/UNIX by Earl Spillar     1991-1992 
TRACK taken for the new system by earl  3 Sept.  91            
LynxOS version first debugged on the mountain OCT 24 1991  

kibbutzing throughout by Gary Grasdalen           

Howell  Aug. 15, 1994    Added code for nod
Howell  Nov. 24, 1995    Set default servo gains to 10000
                         to compensate for hardware changes

Howell  Mar. 06, 1996    This is a special version modified to handle the
                         dome encoder which gives 256 counts every 
						 quarter turn.
Howell  July 01, 1996    Modified check of D80 configuration so that newer
			 rev boards DO NOT result in the false alarm about
			 failure to configure.

Howell  July 07, 1996    Implementing in-control-room guide panel.
		Only required to changed:
		1) In do_paddle_rates  check inside values
		2) In main code always read "HIFI"

Howell  Sep. 09, 1996    Removed the debug output which was started
                        when the HIFI / Control Room Paddle was
                        installed.
                        Added code (in Paddle section) to read focus value
                        which also comes in on HIFI input, and store
                        value in FOCUS shared memory variable.  The
                        calibration used to reproduce focus panel values
                        may be off slightly.  Need to add filtering to
		focus a-to-d converter input, becuase it jitters
	    by about 5 units.  That is also size of error over
	    full range, compared to panel.

Weger  Dec. 30, 1996    Removed the patch installed by Howell on March
			6, 1996 which works with the dome encoder that
			gave 256 counts every quarter turn.  Replaced
			that encoder with one which gives 256 counts every
			complete turn, like the busted, original encoder.

Spillar  April 1999 	Moving to Linux
Weger    Fall  1999     Various changes
RRH      991220         Trying to fix Planet.  Delete timeb references.
RRH      Jan. 14 2000   Added copy of time to CURRENT in read_ut -- it had been
                         lost in conversion and was making planet trk fail.
RRH	 Jan. 14 2000   Added rollover fix for seconds_on_object at 0 UT
Weger    Jan. 19 2000   Activated log_entry declaration & statement for logging.
Weger    Jan. 25 2000   Activated "store" (state of system) thread.
Weger    Mar. 29 2000   Modified do_screens to allow creation of a tracking
                         screen while track is running: do the procedure
                         tscreen_add() if the value ADD_SCREEN was set by the
                         command trackscreen.  Made with version 17 of wiro.h.
Weger    Mar. 30 2000   Activated the diagnostic thread.
Weger    Jun. 01 2000   Remove "WARNING" re clock for demos while UT updates
                         busted.
Weger    Feb. 27 2000   Installed remote focusing.  Reinstalled "WARNING".
*/

/* Operational Parameters */

#define SCREENTIME 300000 	/* Time between screen updates in microsecs */
							/* Warning! NO TIMES OVER 1 SEC! */
#define SCREENCYCLES 10000   /* Times between refreshes */
#define SLEWRATE 10.         /* 10. corresponds to the old slew rate */
							/* Larger Numbers mean Slower Rates */
#define MAX_DOME_ERROR 4    /* Maximum dome error before motion -  		
							   *256/360 degrees */


/****** Various useful defines ******************************************/
/* Convert 8 bits from BCD to binary */
#define BCD_DEC(x) (  ( 10 * ( ( (x)>>4 )&0xf) ) | ((x)&0xF) )
#define DPR ( 57.29577951 ) /* Degrees per radian */

/***** includes **********************************************************/

#include "wirotypes.h"   /* Does wiro typedefs and system dependencies */
#include <stdlib.h>      /* Standard libraries */
#include <stdio.h>
#include <strings.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>


#include "worm_corr.h"    /* contains worm error numbers           */
#include "track.h"        /* global function prototypes */
#include "GPIBports.h"    /* contains GPIB interface info          */
#include "wiro.h"         /* contains all the global variable defs.*/
#include "parameters.h"   /* hardware constants */
#include <pthread.h>      /*  POSIX threads package. */
/* Prototypes of tracking functions */

void do_diagnostic         (void);  /* The interactive diagnostic thread */
void save_diagnostic_state (void);
void get_position_from_diag(void);
void get_velocity_from_diag(void);
void set_zero_velocity     (void);
void simulate_telescope    (void);
short encode_ha_rate	   (double Velocity);
short encode_dec_rate	   (double Velocity);

void init           (void);  /* The master initialization routine */
void read_ut        (void);  /* Read in the universal time, set variables */
void lst            (void);  /* Set the local sideral time */
void do_paddle(void);        /* read in the hand paddle and move telescope */
void do_times       (void);  /* Update the times do to another timer tick */
void do_angles      (void);  /* Calculate all the positions angles, like HA*/
void do_corrections (void); 
			/* Include flexture, parralax, worm errors, refraction. */
void do_tracking    (void);  /* Calculate DES_RA including proper motions */
void do_store       (void);  /* Store the current state */
void do_rates       (void);  /* Calculate rates for motors given  errors */
void do_turns       (void);  
		/* Watch to see if we need to increment the turns counter */
void do_screens(void);       /* This function displays the tracking screens */
void run_dome       (void);  /* Check dome encoder, move if necessary */
void tinfo_init     (void);  /* Read in initial state */
void trackloop      (void);  
		/* The loop of tracking commands executed over and over */
void track_init     (void);    /* Initializes clocks and drives. */
void read_hifi      (void);    /* Read hifi on hifi D80 channel 2 */
void write_focus    (void);    /* Command focus motor via hifi D80 ch 2, pt 5 */
void write_hifi     (void);    /* Write hifi on hifi D80 channel 1 */
void log_entry      (char *comment);  /* Adds a comment into the log */
                /* which is located at /home/observer/wiro/logging/log */


/* Test and hardware enable flags - default states*/
int paddle_present  =1;  /* Set to 1 to enable paddle input. */
int drive_present   =1;  /* Set to 1 to enable drive access. */
int encoder_present =1;  /* Set to 1 to enable encoders */
int hifi_present    =1;  /* Set to 1 to enable hifi */

int PRINTGPIB = 0;       /* print all GPIB i/o to screen */

int elapsed_ticks;       /* Ticks since track started */

unsigned short haraw, decraw;   

struct wiro_memory tinfodata;  /* Allocate space for structure */
struct wiro_memory *tinfo;     /* Create far pointer to it.    */
FILE *core,*debugfile;

int debug      =0;      /* Set to 1 to enable debug output to lower screen */
int nosigblock =0;      /* Set to 1 to allow sigalrm to kill threads. */

extern int DiagnosticMode; /* See manual on diagnostics */
						   
extern number_terminals;       /* The number of tracking terminals */
extern FILE* screen_stream[];  /* The names of these screens */

unsigned short  last_ha_enc,  last_dec_enc;  /* Save last encoder values */

/* Here are some structures to read in some data */
struct d80data drives, drives2;
struct d80data encoders, encoders2;
struct d80data1 paddle;

double  sin_ha,   /* Common angles that are used by many routines.  */
cos_ha,
sin_dec,
cos_dec,
sin_lat,
cos_lat,
d_dec = 0.0,
d_ha = 0.0;


/*************************************************************************/
/*************************************************************************/
/* The main program */
/*************************************************************************/
/*************************************************************************/

void main(int argc, char *argv[])

{       
	int i,j;
	pthread_t thTrack,thScreen,thStore,thDiagnostic;
	pthread_attr_t thatTrack, thatScreen, thatStore, thatDiagnostic;
	long *totask;
	void **stat;
	long tmp;

	/*	setprio(getpid(),MAINPRIORITY);  */
	/* First, grab shared memory */
	tinfo = get_tinfo();

	if (argc == 1) {   /* If not enough arguments, */
		/* Assume that the executer wants to know the options! */
		printf("track moves the telescope and displays tracking screens.\n");
		printf("Some options:\n");
		printf(" printgpib forces the program to print all gpib traffic on screen\n");
		printf(" nopaddle turns the dome handpaddle off\n");
		printf(" noencoder turns off the encoders \n");
		printf(" nodrive turns off the drive \n");
		printf(" nohifi turns off the hifi control \n");
		printf("     This also disables the control room paddle.\n");
		printf(" port (terminal name) puts a tracker screen on (terminal name)\n");
		printf(" test turns off drive  paddle encoder and hifi\n");
		printf(" debug types out encoders on tracking screen\n");
		printf("     and writes data to a file debugfile.\n");
		printf("     Warning: the data piles up quickly\n");
		printf(" nosigblock lets termination signals kill the task\n");
		printf("diagnostic enables interactive diagnostics\n");
		exit(0);
	}


	for (i = 1; i<argc; i++)  /* collect instructions from command line */
	{

		/* Go down the argument list one at a time and check flags */

		printf("Scanning argument %s\n",argv[i]);
		if ( !strcmp(argv[i],"printgpib") )  PRINTGPIB = 1;   /* print flag */
		/* if test flag...*/
		else if ( !strcmp(argv[i], "nopaddle") ) {           /* paddle flag */
			paddle_present = 0;
			printf("Paddle not present \n");
		}
			
		else if ( !strcmp(argv[i], "nodrive") ) {           
			drive_present = 0;
			printf("drive not present\n");
		}
			
		else if ( !strcmp(argv[i], "noencoder") ) {          
			encoder_present = 0;
			printf("Encoder not present \n");
		}
		
		else if ( !strcmp(argv[i], "nohifi") ) {          
			hifi_present = 0;
			printf("HIFI not present \n");
		}

		else if ( !strcmp(argv[i], "nosigblock") ) {
			nosigblock=1;
			printf("Allowing alarm signals through to threads\n");
		} 
		
		else if ( !strcmp(argv[i], "test"    ) )                   
		  {/* no paddle, no drive! */
			encoder_present = 0;
			paddle_present = 0;                      
			drive_present  = 0;
			hifi_present   =0;
			printf(" No paddle, No drive, noncoder assumed present. \n");
		}

		else if ( !strcmp(argv[i], "port" )) {  
				/* If port, open a tracker screen */ 
				/* Open the terminal file and add the name to the list */
			screen_stream[number_terminals++] = fopen(argv[++i],"w");
			printf("Opening port %s for tracking screen\n",argv[i]);
			if (screen_stream[number_terminals-1] == NULL) {
				number_terminals--;
				printf("->Unable to open terminal %s\n",argv[i]);
			}
		}
		else if ( !strcmp(argv[i],"debug") ) {
			debug=1;
			debugfile=fopen("debugfile","w");
		}

		else if ( !strcmp(argv[i], "diagnostic") ) {
			debug=1;
			DiagnosticMode = POSITION_MODE;
			printf("Diagnostic thread will be started.\n");
		} 
		
		else {
			printf("I don't understand the command %s\n", argv[i]);
			exit(-1);
		}
	}

	if (drive_present | encoder_present | paddle_present )
	            /* If we are going to use the drives, */
		gpibport_setup();       
	/* This opens up the appropriate channel */
				/* so that the other commands can use it */
				/* so that the other commands can use it */
				/* to communicate with the IEEE 488.     */
				/* Set up encoder port in high speed */

	tinfo_init();               /* Initialize telescope information */

	for (i=0; i<8; i++)         /* Clear the user strings */
	{
		for (j=0; j<80; j++) USER_STR[i] [j] = '\0';
	}

	track_init();    /* Call standard wiro initializations */
	tscreen_init();  /* Tracking screen initializations */

	pthread_attr_init(&thatStore);
	pthread_create( &thStore, &thatStore, (void *)do_store, NULL);

	pthread_attr_init(&thatScreen);
	pthread_attr_setschedpolicy(&thatScreen, SCHED_OTHER);
	/* I need to put in some scheduling priorityies here! */
	pthread_create( &thScreen, &thatScreen, (void *)do_screens , NULL);

	pthread_attr_init(&thatTrack);
	pthread_attr_setschedpolicy(&thatTrack, SCHED_OTHER);
	/* I need to put in some scheduling priorityies here! */
	pthread_create( &thTrack, &thatTrack, (void *)trackloop , NULL);


	if (DiagnosticMode != NULL)
	  {
		pthread_attr_init(&thatDiagnostic);
		pthread_create(&thDiagnostic, &thatDiagnostic, (void *)do_diagnostic, &totask);
	  }
	log_entry ( "track" );
	pthread_join(thStore, &tmp );
	pthread_join(thScreen,  &tmp );
	pthread_join(thTrack,  &tmp); 

	while(tinfo->keep_tracking != TRACK_STOP) { 
		usleep( (unsigned long) 1000000);	  
	};

	printf("Exiting track\n");

	fflush(stdout);

}

/***********************************************************************/
/********************* A thread to do the track screens  ***************/
/***********************************************************************/

void do_screens(void ) {

	void *v;
	long int loopnumber=0;

	while(tinfo->keep_tracking != TRACK_STOP) { 

		if ( ( loopnumber%SCREENCYCLES) == 0 ) tscreen_init();  
		if (tinfo->keep_tracking == REMOTE_REFRESH) {
			tscreen_init();
			tinfo->keep_tracking = TRACK_GO;
			} 
		/* Initialize if not allready, and do so periodically anyway */
		if (tinfo->keep_tracking == ADD_SCREEN) {
			tscreen_add();
			tinfo->keep_tracking = TRACK_GO;
			}
		/* Add a tracking screen if asked to do so. */
		tminn(); 
 			 /* do the tracking screen */
		loopnumber++;
		usleep( (unsigned long) SCREENTIME);
	}
	printf("New exiting the screens thread \n");
	pthread_exit(0);
}


/*************************************************************************/
/************************* Main tracking loop ***************************/
/*************************************************************************/

void trackloop(void)
{

	void *v;

	printf("Entering trackloop\n");
	while(tinfo->keep_tracking != TRACK_STOP)  {
	  
		usleep( (unsigned long) 50000 );
		tinfo->status++;         /* update the loop counter */

		switch(DiagnosticMode)
		  {
		  /* IDLE diagnostic mode: 
		     Send zero velocity commands. */
		  case IDLE_MODE:	
		    do_encoders();          
		    do_times( );            
		    do_angles();
		    set_zero_velocity();
		    break;

		  /* POSITION diagnostic mode:
		     Set desired HA and DEC directly, then track normally. */
		  case POSITION_MODE:	
		    get_position_from_diag();
		    do_encoders();          
		    do_angles();
		    do_rates();
		    save_diagnostic_state();
		    break;

		  /* VELOCITY diagnostic mode:
		     Set desired rates directly. */
		  case VELOCITY_MODE:	
		    do_encoders();          
		    do_angles();
		    get_velocity_from_diag();
		    save_diagnostic_state();
		    break;

		  /* NORMAL and PASSIVE observing modes:
		     Track normally, optionally monitoring performance. */
		  case PASSIVE_MODE:	
		  case NULL:		
		  default:
	
			/* In temporary modification for Ames, */
                        /* the hifi is always read
                        if (hifi_present) read_hifi(); */

                        /* HIFI[0] bit 3 tells if focus is being remotely */
                        /* adjusted in setfocus.  Bit 1 is set after a read */
                        /* and cleared after a write:                       */
			if (hifi_present)
                        {
                            if ((HIFI[0] & 0x04) == 0x00) read_hifi();
                            else if ((HIFI[0] & 0x01) == 0x00) read_hifi();
                            else  write_focus();
                        }
			do_paddle();

		    do_encoders();          /* read telescope encoders */
		    do_times( );            /* read and set time variables */
		    do_angles( );           /* Calculate telescope angles */
		    do_corrections( );      /* Correct for flexture etc. */
		    do_tracking( );         /* Calculate object position */
		    do_rates( );            /* calcuate what rates we want */

		    if (DiagnosticMode == PASSIVE_MODE) save_diagnostic_state();
		  } /* switch */

		if ( !tinfo->no_motion )
		{	
#ifdef REVERSED
			swab(   (lpchar) &drives.dec, (lpchar) &drives2.dec, 4);
			memcpy( (lpchar) &drives.dec, (lpchar) &drives2.dec, 4);
#endif  /* Reverse bytes if intel! */
			if (drive_present) {
				gpib_wr(drive_adr, (lpchar) &(drives.dome), 5); 
			}
		 } 
		/* WARNING  WE ARE RELYING ON THE DEADMAN CIRCUITS TO STOP */
		/* TRACKING WHEN no_motion IS TRUE.  THIS SEEMS DANGEROUS.  I MAY  */
		/* CHANGE CODE TO WRITE "STOP" COMMANDS ONCE, THEN RELY ON DEADMAN'S.*/
		/* YOU PROBABLY DON'T WANT TO keep SENDIND 0 MOTION COMMANDS.      */
	}
	printf("Now exiting trackloop\n");
	pthread_exit(0);
}

/*************************************************************************/
/************** store the current tracking state once in a while *********/
/*************************************************************************/
void do_store( void )
{
	while( tinfo->keep_tracking != TRACK_STOP) {
		fwrite(tinfo, sizeof(*tinfo),1,core);
		rewind(core);
		sleep(3);
	};
}
		
/*************************************************************************/
/*********************** Initialize clocks and drives ********************/
/*************************************************************************/

void track_init( )
{
	init( );		/* Init initializes I/O ports and reads data once */
	NODDED = 0;		/* Clear any software commanded NOD */
	read_ut( );		/* Read UT clock and set internal counters */
	timecopy(CURRENT,&START);
		
	ELAPSED =  UT_TIME;      /* Set the counter at the beggining */
	lst( );                  /* Set the local sidereal time */

	drives.dome =      0;    /* Set drive command velocities  to 0 */
	drives.dec  = 0xc7ff;
	drives.ha   = 0xc7ff;
}

/*************************************************************************/
/********************* read hifi   ***************************************/
/*************************************************************************/

/* In read_hifi() HIFI[1] through HIFI[4] are filled with data from ports
5 through 2 respectively of the second channel (hifi_in_adr) of the HIFI D80.
Here, the msb of HIFI[1] is the msb of the entire 40 bit channel as well as the
msb of port 5 (8 bits).  The Control Room Hand Paddle is read into the computer
at HIFI[1] via port 5.

The focus readout is contained in the twelve bits which are returned in all of
HIFI[3] and the least significant 4 bits of HIFI[2].  The fourth-least-
significant bit of HIFI[2] is the msb of the focus readout.  The lsb of HIFI[3]
is the lsb of the focus readout. */

void read_hifi(void) 
{
		gpib_rd(hifi_in_adr, HIFI+1 , 4);  
		HIFI[0] |= 0x01;
}

/*************************************************************************/
/********************* write focus ***************************************/
/*************************************************************************/

/* While adjusting the focus remotely in setfocus, alternate trackloops pass
between reading the focus and writing to the focus motor.  Read_hifi and
write_focus accomplish this by setting and clearing HIFI[0] bit 1.  The eight
bits in HIFI[0] contain no I/O data from/to the HIFI D80; they are reserved for
flags.

In write_focus the three least-significant bits of HIFI[5] are output via
port 1 of the second channel of the  HIFI D80 to the dedicated Focus
Control box which is located in the Control Room Rack.  The least-significant
bit of HIFI[5] toggles Fast/Slow.  The 2nd-lsb commands Out.  The 3rd-lsb of
HIFI[5] commands In. */


void write_focus(void)
{
                gpib_wr(hifi_in_adr, HIFI+5 , 1);
                HIFI[0] &= 0xfe;

} 

/*************************************************************************/
/********************* write hifi  ***************************************/
/*************************************************************************/
void write_hifi(void) 
{
		gpib_wr(hifi_out_adr, HIFI+1, 5); 
}

/*************************************************************************/
/********************* check and execute paddle instructions ****************/
/*************************************************************************/
void do_paddle(void ) {
	double  padrate;            /* Paddle velocity, in "/second.       */
	if (paddle_present) {
		gpib_rd(paddle_adr,  (lpchar) &paddle.port1, 5);  
	}
	/* Read paddle mux */
	else {
		memset( (lpchar) &paddle.port1, 0, 5); 
		paddle.port1 |= 0x80;
		/* Clear all except the nod bit if we are */
		/* ignoring the paddle. */
		}

	/* Now decide if we should be in the B beam */
	if (paddle.port1 & 0x80)
		NODDED &= ~1;	/* High input means A beam.   */
						/* Clear hardware command bit */
	else 
		NODDED |=  1;	/* Low input means B beam.    */ 
						/* Set  hardware command bit. */

	/* Now calculate paddle rates.   */
	switch ( SPEED_BITS & paddle.port5)  /* First get the speed */
	{
	case GUIDE : 
		padrate = GUIDE_RATE;
		break;
	case SET   : 
		padrate = SET_RATE;
		break;
	case FAST  : 
		padrate = FAST_RATE;
		break;
	default    : 
		padrate = GUIDE_RATE;
	}
	PAD_V_DEC = 0.;         /* Set the default paddle rates to zero */
	PAD_V_RA  = 0.;
	/* Add a rate in the appropriate direction */
	if (  (NORTH & paddle.port5) && !(SOUTH & paddle.port5) )
		PAD_V_DEC =  padrate;
	if ( !(NORTH & paddle.port5) &&  (SOUTH & paddle.port5) )
		PAD_V_DEC = -padrate;
	if (  (WEST  & paddle.port5) && !(EAST  & paddle.port5) )
		PAD_V_RA  =  padrate;
	if ( !(WEST  & paddle.port5) &&  (EAST  & paddle.port5) )
		PAD_V_RA  = -padrate;

	/* Addition for control room paddle */
#define	CTRL_ROOM_NORTH 1
#define CTRL_ROOM_SOUTH 2
#define CTRL_ROOM_EAST  4
#define CTRL_ROOM_WEST  8
	if (hifi_present) {
		/* For some reason polarity is reversed from standard paddle code */
		if (  (CTRL_ROOM_NORTH & HIFI[1]) && !(CTRL_ROOM_SOUTH & HIFI[1]) )
			PAD_V_DEC = -padrate;
		if ( !(CTRL_ROOM_NORTH & HIFI[1]) &&  (CTRL_ROOM_SOUTH & HIFI[1]) )
			PAD_V_DEC = padrate;
		if (  (CTRL_ROOM_WEST  & HIFI[1]) && !(CTRL_ROOM_EAST  & HIFI[1]) )
			PAD_V_RA  = -padrate;
		if ( !(CTRL_ROOM_WEST  & HIFI[1]) &&  (CTRL_ROOM_EAST  & HIFI[1]) )
			PAD_V_RA  = padrate;
	
		/* Now read the focus, which also comes in on the HIFI line. */
		/* The code should eventually be reorganized to put this in */
		/* a better place, with general HIFI input for many terms. */
/*		FOCUS = -4.8763E-4 * ( 
								(double) ( (0x0F & HIFI[2])<<8 | HIFI[3])
					       		- 2030.4
					         ); */
		FOCUS = (       ((double) (1 - 2 * ((0x20 & HIFI[2]) >> 5)))
		          * (   ((double) (1000  * ((0x10 & HIFI[2]) >> 4)))
		              + ((double) (100   *  (0x0F & HIFI[2])))
		              + ((double) (10    * ((0xF0 & HIFI[3]) >> 4)))
		              + ((double) (1     *  (0x0F & HIFI[3])))
		            )
		        );
	sprintf(USER_STR[4], " %2x %2x %2x %2x %2x", HIFI[1], HIFI[2], HIFI[3], HIFI[4], HIFI[5]);
	}
	/* end of additions for control room paddle */
}

/*************************************************************************/
/**************** Read the encoders  *************************************/
/*************************************************************************/
/* This is supposed to contain all the magic to read and interpret all   */
/* the encoders each pass through the tracking code.                     */
/*************************************************************************/

do_encoders()
{
	int16 tmpgray;     /* Temporarily holds encoder gray code */
	char msg[81];

	if (encoder_present) 
	  { /* Read encoders. */
      gpib_rd(encoder_adr, (lpchar) &encoders.dome,  5);  

#ifdef REVERSED	 /*    Swap bytes for Intel chips */	
	  swab(   (char *) &encoders.dec, (char *) &encoders2.dec, 4);
	  memcpy( (char *) &encoders.dec, (char *) &encoders2.dec, 4);
#endif

	  if (debug){
		sprintf(USER_STR[0],"DOME: %x DEC: %x HA: %x",
			(int) encoders.dome,(int)encoders.dec,(int)encoders.ha);
		}

	  
	  /* Now decode  the encoder values.                             */
	  /* The dome encoder reads true 8 bit binary.                   */
	  /* The HA and DEC encoders use gray code in the upper 14 bits. */
	  /*   The low order two bits must be masked off.  Furthurmore,  */
	  /*   for the gray>binary conversion to work, you must shift    */
	  /*   the real bits down to the bottom.  To keeps the rest of   */
	  /*   the program the same, I shift them back up after the      */
	  /*   conversion.                                               */

	  /* Temp. fix for 4X Dome encoder */
	  /* It counts 256 turns every quarter turn.  Compensate here */

	  /* raw_dome_enc = encoders.dome & 0xff; */
	      /* Has encoder rolled over a quarter turn? */
	  /* if ( (raw_dome_enc <  64) && (last_raw_dome_enc > 192) ) */
		/* dome_qturn++; */
	  /* if (( raw_dome_enc > 192) && (last_raw_dome_enc <  64) ) */
		/* dome_qturn--; */
	  /* last_raw_dome_enc = raw_dome_enc; */

	  /* Has dome itself done a full turn? */
	  /* if (dome_qturn >= 4) dome_qturn = 0; */
	  /* if (dome_qturn <  0) dome_qturn = 3; */
	  
	  
	  /* sim_dome_enc = 64 * dome_qturn + ( raw_dome_enc / 4); */
	  /* tinfo->dome_enc = sim_dome_enc; */
	
	  tinfo->dome_enc = encoders.dome & 0xff;
	  /* End of 4X Dome encoder fix */

	  haraw = encoders.ha;
	  tmpgray = encoders.ha;
	  tmpgray = 0x3FFF & (tmpgray>>2);
	  tinfo->ha_enc = gray2bin( tmpgray ) << 2;

	  decraw = encoders.dec;
	  tmpgray = encoders.dec;
	  tmpgray = 0x3FFF & (tmpgray>>2);
	  tinfo->dec_enc = gray2bin( tmpgray ) << 2;
	  }
	else 
	  { /* Clear array if we are ignoring the dome */
	  memset( (lpchar) &encoders.dome, 0, 5);            
	  encoders.dome =0;
	  encoders.dec =0;
	  encoders.ha=0;

	  simulate_telescope();
	  }

	if ( tinfo->dome_on )
		run_dome( );
	else
		drives.dome = 0;

	do_turns( );
	/* Combine turns count with the encoder (fraction of turn) */

#ifdef REVERSED    
	tinfo->ha.halves[  1 ] = tinfo->ha_turns;
	tinfo->ha.halves[  0 ] = tinfo->ha_enc;
	tinfo->dec.halves[ 0 ] = tinfo->dec_enc;
	tinfo->dec.halves[ 1 ] = tinfo->dec_turns;
#else
	tinfo->ha.halves[  1 ] = tinfo->ha_enc;
	tinfo->ha.halves[  0 ] = tinfo->ha_turns;
	tinfo->dec.halves[ 1 ] = tinfo->dec_enc;
	tinfo->dec.halves[ 0 ] = tinfo->dec_turns;
#endif
}

/**********************  do_turns() ****************************************/
/* Check encoders to see if either worm has moved past the                 */
/*   0 - 65536 turnover point.  If so, update turns counter.               */         
/***************************************************************************/
void do_turns( void )
{
	if ( (tinfo->ha_enc  < 16384) && (last_ha_enc  > 49152) )
		tinfo->ha_turns++;
	if ( (tinfo->ha_enc  > 49152) && (last_ha_enc  < 16384) )
		tinfo->ha_turns--;

	if ( (tinfo->dec_enc < 16384) && (last_dec_enc > 49152) )
		tinfo->dec_turns++;
	if ( (tinfo->dec_enc > 49152) && (last_dec_enc < 16384) )
		tinfo->dec_turns--;


	last_ha_enc  = tinfo->ha_enc;     /* record last HA and Dec */
	last_dec_enc = tinfo->dec_enc;
}

/******************************* do_rates() ********************************/
/*  Calculate rates to send to servoes, based on errors in ha and dec.     */
/***************************************************************************/
void do_rates( void )
{
/* See parameters.h for lots of important hardware constants used below.

   The parameters: double HA_SERVO & double DEC_SERVO	
   define the commanded velocity vs. error curve.  
   The computed rate is 1/2 its max at this encoder error.
   They are set in tinfoinit.

   When tracking an object, the siderial rate is added to the computed
   rate to keep the telescope from lagging behind.
*/
	double  ha_err, dec_err;	/* Error, in encoder units. */
	double  ha_rt,  dec_rt; 	/* Velocity commanded by servo. */

	/* Get error in units of encoder */
	ha_err  = (DES_HA  - HA ) * ENCODERS_PER_DEGREE;   
	dec_err = (DES_DEC - DEC) * ENCODERS_PER_DEGREE;

	if (debug)  
	  sprintf(USER_STR[3], "Errors: %8.1lf  %8.1lf", ha_err, dec_err); 
	if (debugfile != NULL)
	  fprintf(debugfile, "%8.1lf  %8.1lf ", ha_err, dec_err); 

/* NOTE:  This is a curve which has slope (MAX_SLEW_SPEED/HA_SERVO) at 
   the origin, reaches (MAX_SLEW_SPEED/2) at HA_SERVO, then goes 
   asymptotically to MAX_SLEW_SPEED. */

	ha_rt  = MAX_SLEW_SPEED * ha_err  / ( fabs(ha_err ) + HA_SERVO);
	dec_rt = MAX_SLEW_SPEED * dec_err / ( fabs(dec_err) + DEC_SERVO);

	if ( tinfo->motion_type != NO_MOTION ) {
		/* Add in siderial rate and check for overflow. */
		ha_rt = ha_rt + SIDERIAL_RATE;    
		if ( ha_rt > MAX_SLEW_SPEED) ha_rt = MAX_SLEW_SPEED;  
	}

	tinfo->ha_rate  = encode_ha_rate (  ha_rt );
	tinfo->dec_rate = encode_dec_rate( dec_rt );

	drives.ha  = tinfo->ha_rate ;    /* load the rates into working area */
	drives.dec = tinfo->dec_rate;

	if (debug)  {
	  sprintf(USER_STR[1], "Rates: %8.0f  %8.0f", ha_rt, dec_rt);
	  sprintf(USER_STR[2], "Cmds: %8d  %8d", 
	  			tinfo ->ha_rate, tinfo ->dec_rate);
	}
	if (debugfile != NULL)
	  fprintf(debugfile, " %8.0f  %8.0f\n", ha_rt, dec_rt);
}

/**************************************************************************/
/* Calculate the angles need for the telescope                            */
/**************************************************************************/

double t0 = -1;        /* UT_TIME for previous iteration.  (HOURS) */

void do_angles( void )
{
	double  delta_t;       /* Time since last iteration. (SECONDS) */

	DEC = ( double ) ( tinfo->dec.whole / ( 2.0 * 65536.0 ) );
	HA  = ( double ) ( tinfo->ha.whole  / ( 2.0 * 65536.0 ) );
	

	cos_ha  = cos( rad( DES_HA  ) );
	sin_ha  = sin( rad( DES_HA  ) );
	cos_dec = cos( rad( DES_DEC ) );
	sin_dec = sin( rad( DES_DEC ) );

	/* For paddle motions, the position is specified as a constant */
	/* plus a rate.  t0 is the time of the last pass through.  delta_t is  */
	/* the elapsed time. */

	if (t0 == -1.) t0 = UT_TIME;
	delta_t = (UT_TIME - t0) * 3600.;          /* Delta in second. */
	t0 = UT_TIME;
	if (delta_t < 0.) delta_t += 24. * 3600.;  /* next day?        */

	/* Include Hand Paddle Motions into offsets */
	OFFSET_HA  += PAD_V_RA  * delta_t / 3600. ;
	OFFSET_DEC += PAD_V_DEC * delta_t / 3600. ; 
	/* deg     +=    "/s    *    s    /   "/deg  */

	/* Update offsets by paddle velocity times  */
	/* interval since last update.              */
	/* UNITS:  OFFSETS are in degrees of arc    */
	/*         delta_t  is in seconds of time   */
	/*         PAD_V_RA is in arcseconds/second */


	/* Include non-solar system object motions */
	if ( tinfo->motion_type == NON_SOLAR )
	{
		OFFSET_HA  += V_RA  * delta_t * 15.0 * cos_dec ;
		OFFSET_DEC += V_DEC * delta_t;
	} 

	/* Combine all the corrections and get an RA and DEC */
	DEC += ( DIAL_DEC - tinfo->dial_dec ) -
	    COL_ZERO_DEC - COL_DEC - OFFSET_DEC - OFFSET_2_DEC - d_dec; 

	HA += ( DIAL_HA - tinfo->dial_ha ) -
	    ( COL_ZERO_HA + COL_HA + OFFSET_HA + OFFSET_2_HA ) / cos_dec - d_ha ; 

	/* Add in the nod motions if needed */
	if (NODDED) {
		DEC -= (NOD_DEC);
		HA  -= (NOD_HA ) / cos_dec;
		}
	if ( tinfo->motion_type == NON_SOLAR )
	{
		DEC -= ABER_NUT_DEC;     /* Put in aberation and nutation */
		HA  += ABER_NUT_RA;      
	}

	RA = LST - HA / 15.0;
	if      (RA <   0.0) RA += 24.0;
	else if (RA >= 24.0) RA -= 24.0;

	/* Calculate an azimuth and altitude */

	AZI = DPR * atan2( sin_ha, cos_ha * sin_lat - tan( rad(DES_DEC) ) * cos_lat );
	AZI += 180.0;
	if (AZI > 360.0) AZI -= 360.0;


	ALT = DPR * asin( sin_lat * sin_dec + cos_lat * cos_dec * cos_ha );

	if ( ALT < 12. )   /* Check for limits, turn off if too low */
	{
		tinfo->no_motion = 1;
	}
}

/*************************************************************************/
/* Make correction for the telescope pointing model                      */
/* See Bob Howell's write-up for the meaning of the terms                */
/*************************************************************************/

double  last_ut;

void do_corrections( void )
{
	double  clsh,
	sin_alt,
	lhp,
	ladd,
	c4hh,
	chch;

	sin_alt = sin( rad( ALT ) );
	if ( sin_alt < 0.05 ) sin_alt = 0.05;

	if ( tinfo->motion_type != SOLAR )
		lhp = 0;
	else
		lhp = HP * ( 1. + rad( HP ) * sin_alt );  
	/* HP = Degrees/Radian * (DE/DO)    where                 */
	/* DE = distance to center of earth   and                 */
	/* DO = distance to object.                               */
	/* IF HP and Parallax were measured in radians then       */
	/* Parallax = ATAN{ HP * cos(alt) / [1 - HP * sin(alt)] } */
	/*          ~     { HP * cos(alt) * [1 + HP * sin(alt)] } */
	/* and  lhp =     { HP            * [1 + HP * sin(alt)] } */

	ladd = (sin_lat - sin_alt * sin_dec) / cos_dec;    /* for ALT -> DEC */
	chch  = cos_ha * cos_ha;
	c4hh  = 8. * (chch*chch - chch) + 1.0;

	d_ha =
	    ( - Refr_H * sin_ha * cos_lat / sin_alt/* RH   Refraction     */
	+ TFlx_H * sin_ha * cos_lat                /* TFH  Tube flexure   */
	+ lhp    * sin_ha * cos_lat                /* parallax            */

	- MAz_H  * cos_ha * sin_dec   /*RRH SIGN*/ /* MA   Polar axle AZ. */
	+ MEl_H  * sin_ha * sin_dec                /* MEH  Polar axle EL. */
	- NPerp  *          sin_dec                /* SDCH Non-perp. ax.  */

	- Emprc1 * cos_ha                          /* CH   Empirical Term1*/
	- Emprc2 * c4hh                            /* C4H  Empirical Term2*/

	- Col_H                                    /* CO   Collimation    */
	) / cos_dec
	    -   Dial_H;                            /* DIAL                */

	d_dec =
	    (   Refr_D * ladd / sin_alt             /* R  Refraction       */
	- TFlx_D * ladd                            /* TF Tube flexure     */
	- lhp    * ladd                            /* Parallax            */

	+ MAz_D  * sin_ha                          /* MA Polar axle AZ.   */
	+ MEl_D  * cos_ha                          /* ME Polar axle EL.   */
	)
		- Dial_D;                              /* Dial (encoder)      */


	/* Now do the HA worm gear correction, based on the HA encoder.       */

	worm_indx = (unsigned long) tinfo->ha_enc;
	worm_indx = worm_indx % 65536;
	worm_indx = (worm_indx * worm_n) / 65536;
	d_ha += worm_corr[worm_indx] / 3600. ;
}

/***************************************************************************/
/* Determine and update for whether we are at a fixed position, tracking a */
/* solar system object or object outside our solar system.                 */
/**************************************************************************/

void do_tracking( void )
{	struct timeval timeonobj;
	double	seconds_on_object;

	switch ( tinfo->motion_type )
	{
	case NO_MOTION :                
		break;
	case SOLAR :  
			/*  solar system objects require a correction for velocity */
		    /* Note that the ORIG constants are set by planet      */
		{  
			timesub(CURRENT,TONOBJ, &timeonobj);
			seconds_on_object = timesize(timeonobj);
			while (seconds_on_object < 0.) seconds_on_object += 86400.;
			/*  V_RA and V_DEC give the hourly rates in RA/DEC units */
			DES_RA =  ORIG_RA + V_RA  *(seconds_on_object/3600.0);
			DES_DEC = ORIG_DEC+ V_DEC *(seconds_on_object/3600.0);
			DES_HA = 15.0 * ( LST - DES_RA);
			if  (DES_HA  < -180.0 )
				DES_HA  += 360.;
			else
				if  ( DES_HA >  180.0 )
					DES_HA -= 360.0;
			break;
		}  /* done with solar */
	case NON_SOLAR :  	
			/* Just old fashioned hour angles if NON_SOLAR */
		{
			DES_HA = 15.0 * ( LST - DES_RA );
			if  (DES_HA  < -180.0 )          /* move to -180 to 180 */
				DES_HA  += 360.;
			else
				if ( DES_HA  >  180.0 )
					DES_HA  -= 360.0;
			break;
		}  /* done with non-solar */
	}
}

/*************************************************************************/
/* Check dome position & move it if necessary.                           */
/**************************************************************************/

void run_dome( void )
{
	int16 dome_error;
	if ( tinfo->status % 16 == 0 )   /* Execute every 16 passes through */
	{               /* First, set desired position from AZI */
		tinfo->dome_des_pos = ( int16 ) (    ( 256.0 / 360.0 ) * AZI
		    + (double) tinfo->dome_offset );
		      
		if ( tinfo->dome_des_pos  > 255 ) /* Force to range 0-255 */
			tinfo->dome_des_pos -= 256;

		         /* Calculate error- */
		dome_error =  tinfo->dome_enc - tinfo->dome_des_pos; 

		   
		if ( abs(dome_error) > MAX_DOME_ERROR )  						
				 /* If over 4 units off, move the dome */
		{        /* First, set up the range */
			if      ( dome_error >  127 )  dome_error -= 256;  
			else if ( dome_error < -127 )  dome_error += 256;

			if  ( dome_error < 0 )  drives.dome = 1; /* Move the dome ! */
			else                    drives.dome = 2;
		}
		else
			drives.dome = 0;                         /* Stop the dome !*/
	}
}


/***********************************************************************/
/* Read UT time from Clock                                             */
/***********************************************************************/

void read_ut( )                     
{

	struct timeval tp;         /* These are all the UNIX time structures */
	struct timezone tzp;       /* that need to be filled. */
	struct tm *ptm;
	time_t tloc;

	/* call the unix time function */
	if ( 0 != gettimeofday(&tp, &tzp) ) 
		printf("Could not get time of day!? \n");

	timecopy(tp, &CURRENT);	/* Need to update CURRENT time */

	UT_TIME = ((double) (tp.tv_sec%(60*60*24))) +     /* get UT in hours */
	((double)tp.tv_usec )/( 1000000.0 );    /* get micro seconds part */

	UT_TIME /= (3600.0);  /* Convert to HOURS */

	time( &tloc );
	ptm=gmtime( & tloc );   /* collect the other time string */
	YEAR  = ptm->tm_year +1900;  
	MONTH = ptm->tm_mon + 1 ;
	DAY   = ptm->tm_mday;
}

/****************************************************************************
 * Calculate and store the LST for the current julian date
 **********/

void lst( void )
{
	double
	    gast, jul, lon, obl, gmst,
	remjul, t, day_fract, day;


	int32 year, month, a, b;

	day_fract = UT_TIME / 24.0;
	year = YEAR;
	month = MONTH;
	day = DAY;

	if (month <= 2) {
		year--;
		month += 12;
	}

	a = year / 100;
	b = 2 - a + ( int32 ) ( a / 4 );

	jul = (int32) (365.25 * year)
		+ (int32) (30.6001 * ( month + 1 ) )
			+ day + b + 1720994.5;    /* The Julian day at midnight */

	JULIAN = jul + day_fract;
	printf("Julian %lf \n",JULIAN);

	t = (jul - 2415020.0) / 36525.0;   /* t = julian centuries since J1900 */
	t = rem( 0.276919398 + 100.0021359 * t + 0.000001075 * t * t ) * 24.0;
	/* This is Newcomb's theory */

	gmst = t + day_fract * 24.0 * 1.0027379093;  /* greenich mean sideral */
	if (gmst > 24.0) gmst -= 24.0;                   

	/*  Ignore the possible 1 second difference between gmst & gast.   */
	/*  UT-UTC can be this great, and we don't take that into account. */
	/*  The gast-gmst changes slowly, and will just be absorbed into   */
	/*  the dial term.                                                 */
	/*    lonobl(jul, &lon, &obl);     */
	/*    gast = gmst + (lon * cos(rad(oblecl(jul))) / 15.0 );         */

	gast = gmst;

	jul = gast - 7.065101852; /* 105d 58m 35s.5 west - Wiros Longitude */
	if (jul < 0.0)
		jul += 24.0; 

	tinfo->time_zero_lst = jul;  /* Save LST last midnight */
}



/*************************************************************************/
/*             Update the LST  and other times                           */
/*************************************************************************/

double  last_ut;

void do_times( void )
{
	FILE *console;
	int32 year, month, a, b;
	double day_fract,jul,day;
	int i;

	tinfo->clockcount--;  /* decrement the clock counter */
	if ( (tinfo->clockcount < 0 ) && (tinfo->clockcount > -400) &&
		 (tinfo->clockcount %20 == 0)  ) {
		for(i=0; i<8; i++)
			 strcpy(USER_STR[i],"WARNING: the clock has stopped!");
		/*		console=fopen("/dev/atc0","w");
		fprintf(console,"\007");
		fclose(console); This is supposed to beep!  Not working yet on Linux*/
		} 

	elapsed_ticks++; /* increment counter of tries */

	read_ut();    /* Read the system clock */

	day_fract = UT_TIME / 24.0;
	year = YEAR;
	month = MONTH;
	day = DAY;

	if (month <= 2) {
		year--;
		month += 12;
	}

	a = year / 100;
	b = 2 - a + ( int32 ) ( a / 4 );

	jul = (int32) (365.25 * year)
		+ (int32) (30.6001 * ( month + 1 ) )
			+ day + b + 1720994.5;    /* The Julian day at midnight */

	JULIAN = jul + day_fract;


	if (UT_TIME - ELAPSED >= -0.5) 
		LST =   tinfo->time_zero_lst
	   	 	+ ( 22707.0 / 22645.0 )
	    	* ( (double) UT_TIME - ELAPSED);  
	else  
		LST =   tinfo->time_zero_lst
	   	 	+ ( 22707.0 / 22645.0 )
	    	* ( (double) 24.0 + UT_TIME - ELAPSED);  

	while ( LST >= 240000000.0 ) LST -= 240000000.0;
	while ( LST >= 240000.0 ) LST -= 240000.0;
	while ( LST >= 24.0 )  LST -= 24.0;

	while ( LST <= -240000000.0 ) LST += 240000000.0;
	while ( LST <= -240000.0 ) LST += 240000.0;
	while ( LST <= 0.0 )  LST += 24.0;

}

/**************************************************************************/
/* Initialize information in the tracking info structure.                 */
/***********************************************************************/

void tinfo_init( void )
{
	if (NULL == (core = fopen(CORE,"r+"	) )) {
		printf("Unable to open corefile %s\n",CORE);
		printf("How can this be? Disk full?\n");
		printf("Altered directory structure? No permission?\n");
	
		DES_HA  = 0.;
		DES_RA  = 0.;
		DES_DEC = 0.;

		COL_DEC = 0.;                
		COL_HA  = 0.;

		OFFSET_DEC   = 0.;
		OFFSET_HA    = 0.;
		OFFSET_2_DEC = 0.;
		OFFSET_2_HA  = 0.;

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
	}

	else {
	  
		fread(tinfo, sizeof(*tinfo), 1, core); /*read the last state  */
	}
/* The next few values are always set! */
	HA_SERVO= 10000.; /* Rate = 1/2 max when encoder error = this */
	DEC_SERVO = 10000.; 

	PAD_V_RA = 0.0;
	PAD_V_DEC = 0.0;

	tinfo->keep_tracking = TRACK_GO;
	tinfo->dome_on = 0;     /* Turn the dome off */
	tinfo->no_motion = 1;
	tinfo->clockcount=250;	
}

/************************************************************************/
/* This converts graycode to binary.                                    */
/************************************************************************/

unsigned short gray2bin( unsigned short gval )
{
	unsigned short
	    bval, temp;
	int32 i;

	gval = ~gval;
	bval = ( 0x8000 & gval ) ? 1 : 0;
	temp = bval;
	for ( i = 14; i >= 0; i-- ) {
		bval <<= 1;
		bval |= ( gval >> i & 0x01 ) ^ temp;
		temp = bval & 0x01;
	}
	return( bval );
}
/*
#include <stdio.h>
#include <math.h>
#include "GPIBports.h"
#include "wtrack.h"

extern int drive_present, encoder_present, paddle_present, hifi_present;
extern double sin_lat, cos_lat;
extern unsigned short last_ha_enc, last_dec_enc;

extern struct d80data encoders, encoders2;
extern struct d80data1 paddle;
extern struct d80data drives, drives2;
*/

/***************************************************************************/
/* Open and initialize everything                                          */
/***************************************************************************/

void init( void )
{
	char msg[81];
	unsigned short tmpgray;      /* RRH  holds shifted gray code value */

	/* Configuration values for the 4 IoTech channels (2 per box).  */

	static char *configs[ 4 ] =                  /* Configuration reported? */
	{
		"C5E0F0G0I000K0L0000M000P0R0Y0", /* Drive   port -- output  */
		"C0E0F0G0I000K0L0000M000P0R0Y0", /* Encoder port -- input   */
		"C0E0F0G0I000K0L0000M000P0R0Y0", /* Paddle  port -- input   */
		"C1E0F0G0I000K0L0000M000P0R0Y0"
	};

	static char *defaults[ 4 ] =             /* Configuration set.      */
	{
		"C5F0G0I000K0L0000M000P0R0Y0XS0X",  /* Drive   port -- output  */
		"C0F0G0I000K0L0000M000P0R0Y0XS0X",  /* Encoder port -- input   */
		"C0F0G0I000K0L0000M000P0R0Y0XS0X",  /* Paddle  port -- input   */
		"C1F0G0I000K0L0000M000P0R0Y0XS0X"
	};

	/* FIRST, open the path to the IB488 and clear all the devices */

	/******************** Init the HIFI channels  *************************/
	if (hifi_present) {
		init_port(hifi_in_adr,  "HIFI in", configs[3], defaults[3]);
		init_port(hifi_out_adr, "HIFI out", configs[0], defaults[0]);
	}	

	/******************** Init the drive channel  **************************/
	if (drive_present)
		init_port(drive_adr, "Drive", configs[0], defaults[0]);

	/******************** Init the encoder channel **********************/
	if (encoder_present) 
		init_port(encoder_adr, "Encoder", configs[1], defaults[1]);
	
	
	/******************** Init the paddle channel  *************************/
	if (paddle_present)
		init_port(paddle_adr, "Paddle", configs[2], defaults[2]);
	
		
	/***************** Now read the encoders one time. *******************/

	printf("Reading encoders once...");	
	if (encoder_present) {
		gpib_rd(encoder_adr, (lpchar) &(encoders.dome), 5);  
	}

	/* Read the encoders */
	else
		memset( (lpchar) &encoders.dome, 0, 5);  /* Clear array */

#ifdef REVERSED
	swab  ( (char *) &encoders.dec, (char *) &encoders2.dec, 4);
	memcpy( (char *) &encoders.dec, (char *) &encoders2.dec, 4);
	/* Swap bytes for intel chips */
#endif

	tmpgray = encoders.ha;         /* See later notes about gray code. */
	tmpgray = 0x3FFF & (tmpgray>>2);
	last_ha_enc = gray2bin( tmpgray ) << 2;

	tmpgray = encoders.dec;
	tmpgray = 0x3FFF & (tmpgray>>2);
	last_dec_enc = gray2bin( tmpgray ) << 2;

	             	                   /* Set up the angles */
	sin_lat = sin( rad( 41.09705) );   /* WIRO = 41:05:49.4 */
	cos_lat = cos( rad( 41.09705) );

	/**************** Read the paddle once **************************/
	if (paddle_present)
		gpib_rd(paddle_adr,  (lpchar) &paddle.port1, 5);
	else
		memset( (lpchar) &paddle.port1, 0, 5);  /* Clear array */
	
	printf("Done Initializing. \n");
}

/*****************************************************************/
/*****************************************************************/

int init_port(address, name, conf, def)
int address;
char *name; 
char *conf; 
char *def;
{		char msg[81];

		printf("Setting %s channel...",name);
		fflush(stdout);
		gpib_clear(address);	
		gpib_wrs(address, "E?X"   );       /* Check for errors */
		gpib_rds(address,  msg, 80);
		if ( msg[ 1 ] != '0' )
			printf( "GPIB %s port error:  %s\n", name, msg );

		gpib_wrs(address, "U0X"   );       /* Check configuration */
		gpib_rds(address,  msg, 80);
		if ( strcmp( msg+3, conf) != 0 )  /* & set it if it is wrong */
										  /* The +3 skips the Rev. # */
										  /* returned by the D80.    */
			{
			gpib_wrs(address, def );
			gpib_wrs(address, "U0X");    /* check one last time */
			gpib_rds(address, msg, 80);
			if ( strcmp( msg, conf ) != 0 )
				printf( "Unable to configure GPIB %s port.\n",name);
		}
		gpib_wrs(address, "F0X");     /* set fast binary mode */
		/* Set to Hex mode for testing */
		printf("Done.\n");
}
