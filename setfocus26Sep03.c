/* setfocus.c          Focus motions for telescope           */

/* The history of rhpad.c from which setfocus.c was hacked: */
/* Robert R. Howell   03/21/92  15:48   Posix LynxOS version    
/** Uses steps of 0.5" and 5.0", and prints out offset
 Modifications:	
 96-07-01  Howell:  Added logging of initial & final offsets 
 99 Spillar: move over to Linux */

/* JSW 27Oct2000 Modified rhpad.c to produce setfocus.c. */


#include <ctype.h>
#include <slang/slang.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "wirotypes.h"
#include "track.h"
#include "GPIBports.h"
#include "wiro.h"

void log_entry( char *comment );

struct  wiro_memory *tinfo, *get_tinfo();


int tty;

main( int argc, char *argv[] )
{
  char	log_message[ 64 ];

  tinfo = get_tinfo();

  sprintf( log_message,
	   "Begin focus setting:  %5.2lf", FOCUS);

  log_entry( log_message );

  HIFI[0] |= 0x04;   /* The first byte of HIFI contains flags which determine */
                     /* how trackloop will read or write to the HIFI D80.  Set*/
  keypad( );

  usleep( (unsigned long) 100000 );  /* Wait while trackloop sends the
                                        hold focus command. */

  HIFI[0] &= 0xfb;   /* then clear the bit which tells trackloop to write to */
                     /* the focus motor through HIFI D80 channel 2, port 5. */
  sprintf( log_message,
	   "End focus setting: %5.2lf", FOCUS);

  log_entry( log_message );

  /* end main */ }


keypad( ) {

  char   c;
  char   r[8];
  char   omsg[80];
  int rrslt;
  struct winsize ws;
  
  ioctl(1,TIOCGWINSZ, &ws);
  SLtt_get_terminfo();
  SLtt_Screen_Rows = ws.ws_row;
  SLtt_Screen_Cols = ws.ws_col;
  SLsmg_init_smg();
  SLang_init_tty(-1,0,1);

  sprintf(omsg, "Type <CR> or <Space> to exit" );
  SLsmg_gotorc(10,0);
  SLsmg_write_string( omsg );

  if (FOCUS_RATE == SLOWF) sprintf(r, "SLOW    ");
  else sprintf(r, "FAST    ");

  SLsmg_gotorc(11,0);
  sprintf(omsg, " %5.2f    %-3s", FOCUS, r);
  SLsmg_write_string( omsg );
  SLsmg_refresh();

  do {
    do {
      sprintf(omsg, " %5.2f", FOCUS);
      SLsmg_gotorc(11,0);
      SLsmg_write_string( omsg );
      SLsmg_refresh();
      usleep( (unsigned long) 500000 );
    } while ( !( c=SLang_getkey() ) );
/*  cursorkey( &c);             convert VT 100 cursor keys to numbers */
    switch ( c ) {
      case '2' : {
        if (FOCUS_RATE == SLOWF) FOCUS_RATE = FASTF;
          else FOCUS_RATE = SLOWF;
        break;
      }
      case '3' : {
        FOCUS_CMD = OUTF;
        break;
      }
      case '1' : {
        FOCUS_CMD = INF;
        break;
      }
      default : { 
        FOCUS_CMD = HOLDF;
        break;
      }
    }
    HIFI[5] = FOCUS_RATE | FOCUS_CMD;
    if (FOCUS_RATE == SLOWF) sprintf(omsg, " %5.2f    SLOW    ", FOCUS);
      else sprintf(omsg, " %5.2f    FAST    ", FOCUS);
    SLsmg_gotorc(11,0);
    SLsmg_write_string( omsg );
    SLsmg_refresh();
  } while ( c != '\n' && c != '\r' && c != ' ' );

  SLsmg_reset_smg();
  SLang_reset_tty();
} /* end setfocus */
