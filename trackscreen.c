/*****************************************************************************/
/* TRACKSCREEN.C                                                             */
/*                                                                           */
/* Cause a new, additional tracking screen to be displayed on a chosen       */
/*   terminal.  This command is issued while "track" is running.             */
/*                                                                           */
/* void trackscreen (void)                                                   */
/*                                                                           */
/* The command "trackscreen" will cause track's "do_screens()" thread to     */
/*   open a new tracking screen on the designated terminal.                  */
/*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wirotypes.h"
#include "track.h"
#include "wtrack.h"
#include "wiro.h"

struct
   wiro_memory *tinfo;

main()
{
  char s[ 100 ];

  tinfo = get_tinfo();

  printf( "\nTo add an additional tracking screen on a particular\n" );
  printf( "  terminal, please enter the device name for that terminal\n" );
  printf( "  (for example: \"/dev/ttyp4\" [This can be obtained by\n" );
  printf( "  logging in and issuing the command \"tty\" on that\n" );
  printf( "  terminal]).  Alternately, press return to exit.\n" );
  printf( "\nNOTE: Log onto the desired tracking screen terminal\n" );
  printf( "  before issuing this command.\n" );
  printf( "\nWARNING: Do not convert the terminal from which the\n" );
  printf( "  initial \"track\" command was issued into a tracking\n" );
  printf( "  screen.\n" );
  printf( "\nEnter the device name or press return:  ");
  gets( s );
  if( strcmp( s, "" ) == 0) exit(0);
  else
  {
    strcpy( tinfo->new_tscreen, s );  
    tinfo->keep_tracking = ADD_SCREEN;
  }
}
