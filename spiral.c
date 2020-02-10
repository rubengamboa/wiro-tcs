/**************************************************************************/
/* spiral.c                                                               */
/* Moves telescope in a spiral pattern.                                   */
/*                                                                        */
/* Based on Zeos code.  Originally for the masscomp.                      */
/* For Lynxos.  May 14, 1992  Robert R. Howell                            */
/* Hacked again for Lynxos.  28Oct93 JSW                                  */
/* Port to Linux.  ESp & 21Jan00 JSW                                      */
/**************************************************************************/

#include <ctype.h>
#include <slang/slang.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <math.h>
#include <string.h>

struct wiro_memory *tracking_info;

#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct  wiro_memory *tinfo, *get_tinfo();


double
   home_ha,                       /* Position at start of spiral        */
   home_dec,
   dtime    = 0.5,                /* Dwell time, in seconds per point   */
   pitch    = 5.0,                /* Default pitch, in arcseconds       */
   degstep;                       /* Step size, in degrees.             */

int size    = 1,    /* Length of the  current side, in steps. */
    step    = 0,    /* Position along current side, in steps. */
    for_rev = 1,    /* 1 = forward (expanding)  -1 = reverse (contracting ) */
    dir     = 0;    /* Forward direction of current size */
                    /* 0=W, 1=N, 2=E, 3=S */

int moving    = 0,              /* True if spiral in underway       */
    good_vals = 0,              /* True if parameters are ok.       */
    quit      = 0;              /* True if program should exit.     */

long iwait;


/* Unix variables */
int    tty;
char   command;
char   tbuf[129];

void spiral(  );            /* Main spiral code.                        */
void turn_left(   );        /* Turn a corner going forward              */
void turn_right(  );        /* Turn a corner going backwards.           */
void take_step(  );         /* Move one step in direction specified     */
void do_command(  );        /* Read command from keyboard & execute it. */

void go(  );                /* Start motion if parameters are ok.       */
void home(  );              /* Goto origin of spiral                    */

void set_pitch(  );         /* Prompt & set pitch size          */
void set_dtime(  );         /* Prompt & set dwell time          */
void check_vals(  );        /* Check pitch & dwell time.        */

void help(  );              /* Print help screen.               */

void log_offsets(  );       /* Record the change in offsets.    */

/*******************************************************************/
void main(argc, argv )
int argc;
char *argv[];
{

   tinfo = get_tinfo();

   PADDLE_STATUS = PADDLE_LOCAL;

   home_ha  = OFFSET_HA;          /* Record home position. */
   home_dec = OFFSET_DEC;

   if ( argc == 3 )               /* Check for command line parameters.  */
       {
       sscanf( argv[ 1 ], "%lf", &pitch );
       sscanf( argv[ 2 ], "%lf", &dtime );
       go( );                          /* If they are OK, ready for motion. */
       }
   else                           /* Otherwise, use default values. */
       iwait = ( long ) ( 1000000.0 * dtime );

    good_vals = 1;


    log_offsets( "BEGIN" );
    spiral( );                     /* Do the spiral.        */
    log_offsets( "END" );
}

void slinit()
{
  struct winsize ws;

  ioctl(1,TIOCGWINSZ, &ws);
  SLtt_get_terminfo();
  SLtt_Screen_Rows = ws.ws_row;
  SLtt_Screen_Cols = ws.ws_col;
  SLsmg_init_smg();
  SLang_init_tty(-1,0,1);

}

void slfinish()
{  
  SLsmg_reset_smg();
  SLang_reset_tty();
}

void sldraw(int x, int y, char* tbuf)
{
	SLsmg_gotorc(x,y);
	SLsmg_write_string( tbuf);
	SLsmg_refresh();
}

/* note: both prompt and retstring should be allocated by the caller */
void slgetstr(char * prompt, char *retstring)
{
  int i, j=0, x=0, y=0;  /* starting cursor place */
  char c;

  for (i = 0; i<10; i++) retstring[i] = 0;
  sldraw(x,y,"                                            ");
  sldraw(x,y,prompt);
  y += 2+strlen(prompt);
  while ( !isspace( c = (char) SLang_getkey()))
    {
      retstring[j] = c;
      retstring[++j] = '\0';
      sldraw(x,y, retstring);
    }
  return;
}



/*************************************************************************/
void spiral( )
{
  slinit();
  sldraw(0,0,"Type \"?\" for help, \"g\" to start, \"q\" to quit.");
  do
    {
      if (SLang_input_pending(1))
	/*      if ( keywait(iwait) )  Wait iwait microseconds for a key    */
        do_command();             /*  to be pressed.  If one was pressed  */
      /*  execute the command.  Wait forever. */
      usleep(iwait);
      if ( moving );
      { 
        if (step >= size  &&  for_rev== 1)  turn_left(  );
        if (step <=    0  &&  for_rev==-1)  turn_right( );
        if (moving) take_step();    /* Test again -- in case size in now zero */
	sprintf(tbuf, "\"Diameter/2\" =%8.1lf    OFFSETS:  %8.1lf  %8.1lf  >   ",
		( 0.5 * pitch * (double) size),
		3600. * OFFSET_HA,
		3600. * OFFSET_DEC );
	sldraw(1,0,tbuf);
      }
    }
  while ( !quit );
  slfinish();
}

/********************************************************************/
void turn_left(  )         /* Turn a corner going forward       */
{
    dir++;                      /*   Turn left              */
    if (dir>3) dir=0;           /*     and wrap counter.    */
    if ( dir==0 || dir==2)      /*   At these corners       */
        size++;                 /*     enlarge the size.    */
    step = 0;                   /*   Motion along new side  */
}

/********************************************************************/
void turn_right(  )        /* Turn a corner going backwards     */
{
  if ( dir==0 || dir==2)      /*   At above corners       */
    size--;                 /*     shrink the size.     */
  dir--;                      /*   Turn right.            */
  if (dir<0) dir=3;           /*     and wrap counter.    */
  step = size;

  /* For the shrinking spiral, make sure size hasn't gone to 0. */
  /*  If it has, fix up values and stop motion.                 */
  if (size<=0)
    {
      home();        /* At original location; stop    */
      sldraw(1,0,"Size=0 STOPPED");
    }
}
/************************************************************************/
void take_step(  )          /* Move one step in direction specified */
{                           /* by dir and for_rev                   */
  step += for_rev;

  if (for_rev==1)   degstep =   pitch / 3600.;
  else              degstep = - pitch / 3600.;

  switch (dir)
    {
    case 0: OFFSET_HA  += degstep;        /* Move W one step */
      break;
    case 1: OFFSET_DEC += degstep;        /* Move N one step */
      break;
    case 2: OFFSET_HA  -= degstep;        /* Move E one step */
      break;
    case 3: OFFSET_DEC -= degstep;        /* Move S one step */
    }
}
/*******************************************************************/
/* Read in command, and execute it.                                */
void do_command( )
{
  command = SLang_getkey();
  switch ( command )
    {
    case 's' :  moving = 0;
      break;
    case 'g' :  moving = 1;
      break;
    case 'p' :  set_pitch( );
      break;
    case 'h' :  home( );
      break;
    case 'q' :  quit = 1;
      break;
    case 'f' :  for_rev = 1;        /* Forward motion */
      moving = 1;
      break;
    case 'r' :  for_rev = -1;       /* Reverse motion */
      moving = 1;
      break;
    case 't' :  set_dtime( );
      break;
    case '?' :  help( );
      break;
    case 'z' :  OFFSET_HA  = 0.;    /* Zero the offsets */
      OFFSET_DEC = 0.;
      moving = 0;
      size = 1;
      step = 0;
      dir  = 0;
      for_rev = 1;
      break;
    default  :  sprintf( tbuf, "?" );  /* Invalid character */
      sldraw(1,0,"?");
    }
  sprintf(tbuf, "Diameter/2  =%8.1lf    OFFSETS:  %8.1lf  %8.1lf  ",
	  ( 0.5 * pitch * (double) size),
	  3600. * OFFSET_HA,
	  3600. * OFFSET_DEC );
  sldraw(1,0,tbuf);
}

/**********************************************************************/
void home( )
{
    moving = 0;                         /* Stop motion */
    OFFSET_HA = home_ha;                /* Go back to original location */
    OFFSET_DEC = home_dec;
    size = 1;
    step = 0;
    dir  = 0;
    for_rev = 1;
}

/*********************************************************************/
void go( )
{
    check_vals( );
    if ( good_vals )
        moving = 1;
}


/*********************************************************************/

void set_pitch( )
{
  char input[100];

  moving = 0;                         /* Stop motion */
  
  slgetstr("Enter new pitch", input);

  sscanf( input, "%lf", &pitch );
  check_vals( );

}

/*********************************************************************/
void set_dtime( )
{
  char input[100];

  moving = 0;                         /* Stop motion */
  slgetstr("Enter new dwell time: ", input);
  sscanf( input, "%lf", &dtime );
  check_vals( );
}

/**********************************************************************/
void check_vals( )
{
  good_vals = 0;
  if ( pitch <= 0 || dtime <= 0 )
    {
      sldraw(0,0, "ERROR: D_Time and Pitch must be > 0.0" );
    }
  else if ( dtime < 0.2 ) {
    sldraw(0,0, "ERROR:  Minimum dwell time = 0.2 sec." );
  }
  else if ( pitch > 60.01) {
    sldraw(0,0,"ERROR:  Maximum pitch size = 60\" " );
  }
  else
    {
      good_vals = 1;
      iwait = ( long ) ( 1000000.0 * dtime );
      sprintf( tbuf, "Pitch = %6.1lf \".  Dwell time = %6.1lf seconds.",
	       pitch , dtime );
      sldraw(0,0, tbuf);
    }
}

/********************************************************************/
void help( )
{
    moving = 0;
    printf( "\n SPIRAL [pitch] [time/point]\n\n" );
    printf( "moves the telescope in an expanding \"square\" spiral. If you\n" );
    printf( "enter the optional parameters then the spiral starts\n" );
    printf( "immediately.  Otherwise, start it with the \"g\" command.\n" );
    printf( "The default pitch is 5.0 arcseconds.\n" );
    printf( "The default dwell time is 0.5 seconds.\n" );
    printf( "The pitch sets the spacing between turns of the spiral, AND\n" );
    printf( "the step size along the spiral.\n" );
    printf( "The dwell time sets the delay after each step.\n" );
    printf( "Other commands are:\n" );
    printf( "g - Go         ( Start the spiral motion. )\n" );
    printf( "f - Forward    ( Start forward expanding motion. )\n" );
    printf( "r - Reverse    ( Start reverse contracting motion. )\n" );
    printf( "q - Quit       ( Exit at current position. )\n" );
    printf( "s - Stop       ( Stop in current position and wait. )\n" );
    printf( "h - Home       ( Go to pre-spiral position and wait. )\n" );
    printf( "z - Zero       ( Go to offset (0,0) and wait. )\n" );
    printf( "t - dwell Time ( Set dwell time -- 0.1 to 60. seconds/point. )\n");
    printf( "p - Pitch      ( Set pitch      -- 0.0 to 60. arcseconds. )\n" );
    printf( "? - help       ( Display this screen. )\n" );
    /*    sprintf( tbuf, ">" );
	  write( STDOUT_FILENO, tbuf, strlen(tbuf) ); */
}


void log_offsets( char *comment )
{
   FILE     *fp;

   char     s1[ 98 ];

   double   ha_off, dec_off;

   sprintf( s1, "%s: spiral", comment );
   log_entry( s1 );
   fp = fopen( "/home/observer/wiro/logging/log", "a" );
   if( fp == NULL )
      printf( "\nUnable to open /home/observer/wiro/logging/log.\n" );
   ha_off = OFFSET_HA * 3600.0;
   dec_off = OFFSET_DEC * 3600.0;
   fprintf( fp, " HA offset:   %lf\n", ha_off );
   fprintf( fp, " Dec offset:  %lf\n", dec_off );
   fclose( fp );
}

