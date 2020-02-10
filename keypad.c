#include <termio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

void log_entry( char *comment );

struct 
   wiro_memory *tinfo;

int
   tty;

double
   start_scale,
   scale;

main( argc, argv )

int
   argc;

char
   *argv[ ];

{

scale = 1.0;

if ( argc == 3 ) {
   sscanf( argv[ 2 ], "%lf", &scale );
   if ( ( scale <= 0.0 ) || ( scale > 10.0 ) ) {
      printf( "Range: 0.0 < rate <= 10.0\n" );
      exit( -3 );
   }
}

printf( "Type <q> to exit\n\r" );
printf("hjkl = West South North East \n");
printf("5 increases the speed by 10\n");
printf("J moves the dome right K moves the dome left\n");
init( );

if ( argc == 1 ) {
   printf( "<local> or <remote> required.\n" );
   exit( -1 );
}
else if ( ( argc == 2 ) && ( strcmp( argv[ 1 ], "local" ) == 0 ) ) {
   tty = 0; 
   PADDLE_STATUS = PADDLE_LOCAL;
}
else if ( ( argc == 2 ) && ( strcmp( argv[ 1 ], "remote" ) == 0 ) ) {
   printf( "Type <q> to exit\n" );
   PADDLE_RATE = scale;
   PADDLE_STATUS = PADDLE_REMOTE;
   exit( 1 );
}
else if ( argc == 2 ) {
   printf( "<local> or <remote> required.\n" );
   exit( -2 );
}

PADDLE_RATE = start_scale = scale;

if ( tty == 0 )
   setraw( tty );
log_offsets( "BEGIN", argv[ 1 ] );
keypad( );
log_offsets( "END", argv[ 1 ] );
PADDLE_STATUS = PADDLE_OFF;
strcpy( PADDLE_CMD, " " );
PADDLE_RATE = 0.0;
restore( tty );

/* end main */ }


init( )

{
tinfo = get_tinfo();
}


log_offsets( char *comment, char *lcl_rem )

{

FILE
   *fp;

char
   s1[ 98 ];

double
   ha_off,
   dec_off,
   dome_off;

sprintf( s1, "%s: keypad %s %lf", comment, lcl_rem, scale );
log_entry( s1 );
fp = fopen( "/home/observer/wiro/logging/log", "a" );
if( fp == NULL )
   printf( "\nUnable to open /home/observer/wiro/logging/log.\n" );
ha_off = OFFSET_HA * 3600.0;
dec_off = OFFSET_DEC * 3600.0;
fprintf( fp, " HA offset:   %lf\n", ha_off );
fprintf( fp, " Dec offset:  %lf\n", dec_off );
fprintf( fp, " Dome offset: %d\n", tinfo -> dome_offset );
fclose( fp );
}



keypad( )

{

char
   c;

double
   factor = 1.0 / 3600.0;

do {
   read( tty, &c, 1 );

   switch ( c ) {
   case '5' : {
      scale *= 10.0;
      if ( scale > 100.0 ) 
         scale = start_scale;
      PADDLE_RATE = scale;
      break;
   }
   case 'j' : {
      sprintf( PADDLE_CMD, "S" );
      OFFSET_DEC -= scale * factor;
      break;
   }
   case 'h': {
      sprintf( PADDLE_CMD, "W" );
      OFFSET_HA += scale * factor;
      break;
   }
   case  'l': {
      sprintf( PADDLE_CMD, "E" );
      OFFSET_HA -= scale * factor;
      break;
   }
   case  'k' : {
      sprintf( PADDLE_CMD, "N" );
      OFFSET_DEC += scale * factor;
      break;
   }
   case 'J' : {
      tinfo -> dome_offset += 1;
      break;
   }
   case 'K' : {
      tinfo -> dome_offset -= 1;
      break;
   }
}

} while ( c != 'q' );

/* end keypad */ }


static struct termio tbufsave, tbufsave1;

setraw ( unit ) 

int unit;

{

   struct termio  tbuf;
   int flags;

   ioctl ( unit, TCGETA, &tbuf);

   if ( unit == 0 )
      tbufsave = tbuf;
   else
      tbufsave1 = tbuf;

   tbuf.c_iflag &= ~(INLCR | ICRNL | IUCLC | ISTRIP | IXON /* | BRKINT */ );
   tbuf.c_oflag &= ~OPOST;
   tbuf.c_lflag &= ~(ICANON | ISIG | ECHO);
   tbuf.c_cc[4] = 1;
   tbuf.c_cc[5] = 0;

   ioctl ( unit, TCSETAF, &tbuf);

   /*
   flags = fcntl( unit, F_GETFL, 0 );
   fcntl( unit, F_SETFL, flags | O_NDELAY );
   */
 
/* end setraw */ }


restore ( unit ) 

int unit;

{

   int flags;

   if ( tty != 0 )
      return;

   if ( unit == 0 ) {
      ioctl ( unit, TCSETAF, &tbufsave);
      flags = fcntl( unit, F_GETFL, 0 );
      fcntl( unit, F_SETFL, flags & ~O_NDELAY );
   }
   else
      ioctl ( unit, TCSETAF, &tbufsave1);

/* end restore */ }
