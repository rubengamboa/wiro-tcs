/* rhpad.c          Paddle motions for telescope           */
/* Robert R. Howell   03/21/92  15:48   Posix LynxOS version    
/** Uses steps of 0.5" and 5.0", and prints out offset
 Modifications:	
 96-07-01  Howell:  Added logging of initial & final offsets 
 99 Spillar: move over to Linux */


#include <ctype.h>
#include <slang/slang.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct  wiro_memory *tinfo, *get_tinfo();


int tty;

main( int argc, char *argv[] )
{
  char	log_message[ 128 ];

  tinfo = get_tinfo();
  /*  PADDLE_STATUS = PADDLE_LOCAL; */

  sprintf( log_message,
	   "Begin pad:  HA offset:   %10.2lf  DEC offset:  %10.2lf",
	   OFFSET_HA * 3600.0, OFFSET_DEC * 3600.0);

  log_entry( log_message );

  keypad( );

  sprintf( log_message,
	   "End pad:    HA offset:   %10.2lf  DEC offset:  %10.2lf",
	   OFFSET_HA * 3600.0, OFFSET_DEC * 3600.0);

  log_entry( log_message );

  /*  PADDLE_STATUS = PADDLE_OFF; */
  strcpy( PADDLE_CMD, " " );
  PADDLE_RATE = 0.0;

  /* end main */ }


keypad( ) {

  char   c;
  char   omsg[80];
  int    scale = 1;
  double factor = 0.5 / 3600.0;
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


  do {
    /* Want following, but Lynxos doesn't support %+8.1f format */
    /* sprintf(omsg, "\r %+8.1f  %+8.1f", OFFSET_HA*3600., OFFSET_DEC*3600.); */
    SLsmg_gotorc(11,0);
    sprintf(omsg, "%8.1f  %8.1f", OFFSET_HA*3600., OFFSET_DEC*3600.);

    SLsmg_write_string( omsg );
    SLsmg_refresh();
    c=SLang_getkey();
    cursorkey( &c);          /* convert VT 100 cursor keys to numbers */
    switch ( c ) {
    case '5' : {
      scale *= 10;
      if ( scale > 10.0 ) 
	scale = 1;
      /* Want %+8.1f here too */
      sprintf(omsg, " %8.1f  %8.1f     Step = %5.1f",
	      OFFSET_HA * 3600., OFFSET_DEC * 3600., scale * factor * 3600.);
      SLsmg_gotorc(11,0);
      SLsmg_write_string( omsg );
      SLsmg_refresh();
      break;
    }
    case '2' : {
      OFFSET_DEC -= scale * factor;
      break;
    }
    case '6' : {
      OFFSET_HA += scale * factor;
      break;
    }
    case '4' : {
      OFFSET_HA -= scale * factor;
      break;
    }
    case '8' : {
      OFFSET_DEC += scale * factor;
      break;
    }
    }
  } while ( c != '\n' && c != '\r' && c != ' ' );

  SLsmg_write_string( omsg );

  SLsmg_reset_smg();
  SLang_reset_tty();
  /* end keypad */ }


/**************** Convert VT 100 cursor keys into numbers 8462 **********/
cursorkey( char *key)
{
  char tstmsg[81];
  int tstval;
  if (*key != 27)  return;           /* If not escape, done */

  (*key)=SLang_getkey();   /* Check next character */
  if (*key != '[') return;           /* Could it be cursor command? */

  *key=SLang_getkey();    /* Check next character */
  switch( *key) {                    /* Is it cursor command? */
  case 'A' : {
    *key = '8';                   /* Translate up   to 8 */
    break;
  }
  case 'B' : {
    *key = '2';                   /* Translate down to 2 */
    break;
  }
  case 'C' : {
    *key = '6';                   /* Translate right to 6 */
    break;
  }
  case 'D' : {
    *key = '4';                   /* Translate left  to 4 */
    break;
  }
  case 'G' : {
    *key = '5';                   /* 5 on Lynxos keypad */
    break;
  }
  }
}





