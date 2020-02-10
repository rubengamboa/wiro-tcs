/* 9/6/91    Modified to run as a thread on console */
/* 12/26/90  23:25   Modified by RRH to output only variable part of dial. */
#include <math.h>

/*****
 *
 * This program controls the tracker screen
 *
 *****/
 

#include "wiro.h"
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <termio.h>
#include <values.h>

#define TTY_DEVICE_0 "/dev/tty8"
#define SCREEN "/usr/local/bin/WIRO_SCREEN"

/*
 * VT100 Commands 
 */

#define NUM_TIME_FIELDS 30

#define CLEAR 			"\033[2J"
#define CURSOR_OFF 		"\033[?25l"
#define CURSOR_ON		"\033[?25h"
#define STATUS_ON		"\033[31h"
#define STATUS_OFF		"\033[31l"
#define GOTOXY			"\033[00;00H"
#define UNDER			"\033[4m"
#define NORMAL			"\033[0m"
#define BLINK			"\033[5m"
#define REV_BLINK		"\033[5;7m"
#define INIT_GRAPHICS		"\033(0"
#define GRAPHICS_ON		"\017"
#define GRAPHICS_OFF            "\016"
#define ERASE_TO_EOL		"\033[?0K"

static char 
   *directions[ 16 ] = {
      "N  ",
      "NNE",
      "NE ",
      "ENE",
      "E  ",
      "ESE",
      "SE ",
      "SSE",
      "S  ",
      "SSW",
      "SW ",
      "WSW",
      "W  ",
      "WNW",
      "NW ",
      "NNW"
   };

struct wiro_memory
   *tracking_info;

REMOTE_INFO
   *remote_info;

CHOP_INFO
   *chopping_info;

int
   pgm_counter = 0,
   cur_field,
   cur_buf = 0,
   tty0;

char
   screen_name[ 80 ],
   tty_buf[ 4000 ];


double 
   last_times[ NUM_TIME_FIELDS ],
   dec2hms( );


main( argc, argv )

int
   argc;
   
char
   *argv[ ];
   
{

if ( argc == 2 )
   strcpy( screen_name, argv[ 1 ] );
else
   strcpy( screen_name, SCREEN );
   
init( );

while ( tracking_info -> keep_tracking ) 
   if ( tracking_info -> keep_tracking == LOCAL_REFRESH ) {
      tracking_info -> keep_tracking = TRACK_GO;
      refresh( );
   }
   else
      update( );

clean_up( );

printf( "Exiting wiro_screen.\n" );

/* end main */ }



/*****
 *
 * Set up the buffer to send to the screen
 *
 *****/

write_buf( str )

char
   str[ ];

{

int
   l;

if ( cur_field != -1 ) {
   if ( strcmp( str, SCR_VALS[ cur_field ] ) == 0 )
      return;
   strcpy( SCR_VALS[ cur_field ], str );
   l = strlen( GOTO_STRS[ cur_field ] );
   memcpy( tty_buf + cur_buf, GOTO_STRS[ cur_field ], l );
   cur_buf += l;
}
 
l = strlen( str );
memcpy( tty_buf + cur_buf, str, l );
cur_buf += l;

/* end write_buf */ }



/*****
 *
 * Clean up the screen
 *
 *****/

clean_up( )

{

write_buf( CLEAR );
write_buf( CURSOR_ON );
write_buf( STATUS_ON );
write_buf( GOTOXY );
write( tty0, tty_buf, cur_buf );

cur_buf = 0;

/* end clean_up */ }



/*****
 *
 * Write a boolean value as ON or OFF
 *
 *****/

write_bool( b )

int
   b;

{

char
   str[ 4 ];

if ( b )
   strcpy( str, "ON " );
else
   strcpy( str, "OFF" );

write_buf( str );

/* end write_bool */ }



/*****
 *
 * Write a long numeric value out
 *
 *****/

write_fnum( num )

double
   num;

{

char
   blanks[ 21 ],
   numstr[ 20 ];

strcpy( blanks, "          " );
sprintf( numstr, "%6.2lf", num );
strncpy( blanks, numstr, strlen( numstr ) );

write_buf( blanks );

/* end write_fnum */ }



/*****
 *
 * Write a long numeric value out
 *
 *****/

write_lnum( num, sign )

int
   num, sign;

{

char
   blanks[ 8 ],
   numstr[ 20 ];

if ( sign )
   sprintf( numstr, "%d", num );
else
   sprintf( numstr, "%d", num & 0xffffffff );

strcpy( blanks, "       " );
strncpy( blanks, numstr, strlen( numstr ) );

write_buf( blanks );

/* end write_lnum */ }



/*****
 *
 * Write a numeric value out
 *
 *****/

write_num( short num, short sign )
{

char
   blanks[ 8 ],
   numstr[ 20 ];

if ( sign )
   sprintf( numstr, "%3hd", num );
else
   sprintf( numstr, "%3hd", num & 0xffff );

strcpy( blanks, "       " );
strncpy( blanks, numstr, strlen( numstr ) );

write_buf( blanks );

/* end write_num */ }



/*****
 *
 * Write a numeric hex value out
 *
 *****/

write_hex( num )

short
   num;

{

char
   numstr[ 20 ];

sprintf( numstr, "%X", num & 0xffff );
strcat( numstr, "    " );
write_buf( numstr );

/* end write_hex */ }



/*****
 *
 * Initialize the terminal and attach the shared memory
 *
 *****/

init( )

{

int
   i;
   
tty0 = open( TTY_DEVICE_0, O_WRONLY ); /* open device in write only mode */
if ( tty0 == -1 ) {
   printf( "Unable to open tty device: %s\n", TTY_DEVICE_0 );
   exit( 1 );
}

setraw( tty0 );

tracking_info = ( struct wiro_memory * ) shm_attach( WIRO_SHARED_MEMORY );
if ( tracking_info == ( struct wiro_memory * ) -1 ) {
   printf( "Unable to attach shared memory\n" );
   exit( -3 );
}

i = shm_allocate( WIRO_REMOTE_MEMORY, sizeof( REMOTE_INFO ) );
if ( i == -1 ) {
   printf( "Unable to allocate remote shared memory\n" );
   exit( -1 );
}

remote_info = ( REMOTE_INFO * ) shm_attach( WIRO_REMOTE_MEMORY );
if ( remote_info == ( REMOTE_INFO * ) -1 ) {
   printf( "Unable to attach remote shared memory\n" );
   exit( -2 );
}

chopping_info = ( CHOP_INFO * ) shm_attach( WIRO_CHOPPING_MEMORY );
if ( chopping_info == ( CHOP_INFO * ) -1 ) {
   printf( "Unable to attach chop shared memory\n" );
   exit( -6 );
}

memset( GOTO_STRS, ( char ) 0, sizeof( GOTO_STRS ) );
memset( remote_info, ( char ) 0, sizeof( REMOTE_INFO ) );

refresh( );

/* end init */ }



/*****
 *
 * Refresh the screen's title fields and options
 *
 *****/

refresh( )

{

int
   under = FALSE,
   row,
   col,
   field,
   i,
   j;

char
   scr_buffer[ 165 ];

FILE
   *scr_file;

for( i = 0; i < NUM_TRACKING_FIELDS; i++ )
   strcpy( SCR_VALS[ i ], "" );

cur_field = -1;
cur_buf = 0;
write_buf( CLEAR );
write_buf( CURSOR_OFF );
write_buf( STATUS_OFF );
write_buf( INIT_GRAPHICS );
write_buf( GOTOXY );

scr_file = fopen( screen_name, "r" );
if ( scr_file == NULL ) {
   printf( "Unable to open screen template: %s\n", screen_name );
   exit( -1 );
}

field = 0;
row = 1;
for( i = 0; i < NUM_TIME_FIELDS; i++ )
   last_times[ i ] = -10101.0101; /* weird arbitrary value */

do {
   fgets( scr_buffer, 160, scr_file );
   if ( feof( scr_file ) )
      break;
   j = strlen( scr_buffer );
   i = 0;
   while ( i < j )  
      if ( scr_buffer[ i ] == '$' ) {
         i++;
         col = i;
         tty_buf[ cur_buf++ ] = ' ';
         field = ( int ) ( scr_buffer[ i++ ] - '0' ) * 10; 
         tty_buf[ cur_buf++ ] = ' ';
         field += ( int ) ( scr_buffer[ i++ ] - '0' );
         tty_buf[ cur_buf++ ] = ' ';
         gotoxy( field, col, row );
      }
      else if ( scr_buffer[ i ] == '_' ) {
         tty_buf[ cur_buf ] = ' ';
         if ( under ) 
            write_buf( NORMAL );
         else
            write_buf( UNDER );
         under = !under;
         i++;
      }
      else if ( scr_buffer[ i ] == '{' ) {
         i++;
         write_buf( GRAPHICS_ON );
      }
      else if ( scr_buffer[ i ] == '}' ) {
         i++;
         write_buf( GRAPHICS_OFF );
      }
      else
         tty_buf[ cur_buf++ ] = scr_buffer[ i++ ];
   if ( row != 24 )
      tty_buf[ cur_buf++ ] = '\r';
   else if ( tty_buf[ cur_buf - 1 ] == '\n' )
      cur_buf--;
   row++;
} while ( TRUE );

fclose( scr_file );

write( tty0, tty_buf, cur_buf );
cur_buf = 0;

/* end refresh */ }



/*****
 *
 * Update the tracking screen
 *
 *****/

update( )

{

for( cur_field = 0; cur_field < NUM_TRACKING_FIELDS; cur_field++ ) 
   if ( GOTO_STRS[ cur_field ][ 0 ] != '\0' ) 
      display( cur_field );
 
if ( cur_buf != 0 )
   write( tty0, tty_buf, cur_buf );

pgm_counter++;
cur_buf = 0;
astpause( -1, 100 );

/* end update */ }



display( field )

int
   field;

{

int
   tmp;

char
   tmpc[ 40 ];

double
   ha_out,
   ra_out,
   dec_out;
   
switch ( field ) {
   case 0: {
      write_time( 1 );
      write( tty0, tty_buf, cur_buf );
      cur_buf = 0;
      break;
   }
   case 1: {
      write_time( 5 );
      break;
   }
   case 2: {
      write_time( 8 );
      break;
   }
   case 3: {
      write_time( 11 );
      break;
   }
   case 4: {
      write_time( 13 );
      break;
   }
   case 5: {
      write_time( 6 );
      break;
   }
   case 6: {
      write_time( 7 );
      break;
   }
   case 7: {
      write_time( 9 );            
      break;
   }
   case 8: {
      write_time( 10 );
      break;
   }
   case 9: {
      write_time( 12 );
      break;
   }
   case 10: {
      write_time( 2 );
      break;
   }
   case 11: {
      write_time( 3 );
      break;
   }
   case 12: {
      write_time( 14 );
      break;
   }
   case 13: {
      if ( ALT < 12. )  {
         strcpy( tmpc, REV_BLINK );
         strcat( tmpc, "LIMIT" );
         strcat( tmpc, NORMAL );
      }
      else  
         sprintf( tmpc, "%4.3f", 1 / sin( ALT * M_PI / 180.0 ) );
      write_buf( tmpc );
      break;
   }
   case 14: {
      write_fnum( FOCUS );
      break;
   }
   case 15: {
      write_bool( ! tracking_info -> no_motion );
      break;
   }
   case 16: {
      write_num( ( int ) AZI, 1 );
      break;
   }
   case 17: {
      strcpy( tmpc, directions[ ( int ) ( AZI / 22.50 ) % 16 ] );
      write_buf( tmpc );
      break;
   }
   case 18: {
      if ( tracking_info -> no_motion == 1 )
         strcpy( tmpc, "      " );
      else { 
         if ( !( tracking_info -> dec_rate & 0xC000 ) ||
              !( tracking_info -> ha_rate & 0xC000 ) )   {
           tmpc[ 0 ] = '\0';
           strcat( tmpc, REV_BLINK );
           strcat( tmpc, " SLEW " );
           strcat( tmpc, NORMAL );
          }
      else { if ( ( abs( (int) ((tracking_info -> dec_rate) & 
                                 0x2FFF) - 0x800) > 0xC0 ) ||
          ( abs( (int) ((tracking_info -> ha_rate) & 
                         0x2FFF ) - 0x800) > 0xC0 ) ) {
           tmpc[ 0 ] = '\0';
           strcat( tmpc, REV_BLINK );
           strcat( tmpc, " SET  " );
           strcat( tmpc, NORMAL );
          } else { 
         if ( tracking_info -> motion_type == NO_MOTION )
             strcpy( tmpc, "FIXED " );
          else
             strcpy( tmpc, "TRACK " ); 
      }  }  } 
      write_buf( tmpc );
      break;
   }
   case 19: {
      if ( ALT < 12. )  {
         strcpy( tmpc, REV_BLINK );
         strcat( tmpc, "LIMIT" );
         strcat( tmpc, NORMAL );
         write_buf( tmpc );
      }
      else  
         write_fnum( ALT );
      break;
   }
   case 20: {
      write_bool( tracking_info -> dome_on );
      break;
   }
   case 21: {
      tmp = tracking_info -> dome_enc - tracking_info -> dome_offset;
      if ( tmp > 255 ) 
         tmp -= 256;
      else if ( tmp < 0 )
         tmp += 256;
      write_num( ( int ) ( tmp * 360.0 / 256.0 ), 0 );
      break;
   }
   case 22: {
      tmp = tracking_info -> dome_enc - tracking_info -> dome_offset;
      if ( tmp > 255 ) 
         tmp -= 256;
      else if ( tmp < 0 )
         tmp += 256;
      tmp = ( int ) ( tmp * 360.0 / 256.0 );
      tmp = ( int ) ( ( tmp + 11.25 ) / 22.50 );
      if ( tmp > 15 )
         tmp = 15;
      else if ( tmp < 0 )
         tmp = 0;
      strcpy( tmpc, directions[ ( int ) tmp ] );
      write_buf( tmpc );
      break;
   }
   case 23: {
      break;
   }
   case 24: {
      if ( PADDLE_STATUS == PADDLE_OFF )
         strcpy( tmpc,  "OFF   " );
      else if ( PADDLE_STATUS == PADDLE_LOCAL )
         strcpy( tmpc, "LOCAL " );
      else if ( PADDLE_STATUS == PADDLE_REMOTE )
         strcpy( tmpc, "REMOTE" );
      write_buf( tmpc );
      break;
   }
   case 25: {
      write_time( 0 );
      break;
   }
   case 26: {
      break;
   }
   case 27: {
      memset( tmpc, ' ', 15 );
      tmpc[ 15 ] = '\0';
      strncpy( tmpc, OBJECT_NAME, min( 15, strlen( OBJECT_NAME ) ) );
      write_buf( tmpc );
      break;
   }
   case 28: {
      write_time( 4 );
      break;
   }
   case 29: {
      break;
   }
   case 30: {
      strcpy( tmpc, tracking_info -> filter );
      write_buf( tmpc );
      break;
   }
   case 31: {
      write_fnum( JULIAN );
      break;
   }
   case 32: {
      write_fnum( TEMPERATURE );
      break;
   }
   case 33: {
      write_fnum( PRESSURE );
      break;
   }
   case 34: {
      strcpy( tmpc, tracking_info -> device_name );
      write_buf( tmpc );
      break;
   }
   case 35: {
      write_fnum( tracking_info -> integration_time );
      break;
   }
   case 36: {
      sprintf( tmpc, "%02d/%02d/%04d", MONTH, DAY, YEAR );
      write_buf( tmpc );
      break;
   }
   case 37: {
      write_user( 0 );
      break;
   }
   case 38: {
      write_user( 1 );
      break;
   }
   case 39: {
      write_user( 2 );
      break;
   }
   case 40: {
      write_user( 3 );
      break;
   }
   case 41: {
      write_user( 4 );
      break;
   }
   case 42: {
      write_user( 5 );
      break;
   }
   case 43: {
      write_user( 6 );
      break;
   }
   case 44: {
      strcpy( tmpc, PADDLE_CMD );
      write_buf( tmpc );
      break;
   }
   case 45: {
      sprintf( tmpc, "%6.2lf", PADDLE_RATE );
      write_buf( tmpc );
      break;
   }
   case 46: { /* 1950 RA */
      write_time( 15 );
      break;
   }
   case 47: { /* 1950 Dec */
      write_time( 16 );
      break;
   }
   case 49: { /* Secondary Offset HA */
      write_time( 17 );
      break;
   }
   case 50: { /* secondary offset dec */
      write_time( 18 );
      break;
   }
   case 51: { /* Nod delta RA */
      write_time( 19 );
      break;
   }
   case 52: { /* Nod delta Dec */
      write_time( 20 );
      break;
   }
   case 53: { /* Throw */
      write_time( 21 );
      break;
   }
   case 54: {
      sprintf( tmpc, "%6.2lf", CHOP_INT_TIME );
      write_buf( tmpc );
      break;
   }
   case 55: {
      sprintf( tmpc, "%6.0lf", CHOP_TOL );
      write_buf( tmpc );
      break;
   }
   case 56: {
      sprintf( tmpc, "%6.2lf", CHOP_FREQUENCY );
      write_buf( tmpc );
      break;
   }
   case 57: {
      sprintf( tmpc, "%6.2lf", CHOP_AMPLITUDE );
      write_buf( tmpc );
      break;
   }
   case 58: {
      sprintf( tmpc, "%6.2lf", CHOP_ZERO );
      write_buf( tmpc );
      break;
   }
   case 59: {
      if ( tracking_info -> motion_type != NO_MOTION) 
      dec_out = DES_DEC - DEC;
      dec_out *= dec_out;
      ha_out = ( DES_HA - HA ) / 15. * cos( DES_DEC * M_PI / 180.0 );
      ha_out *= ha_out;
      sprintf( tmpc, "%6.0lf", sqrt( ha_out + dec_out ) * 3600.0 );
      write_buf( tmpc );
      break;
   }
   case 60: {
      write_buf( CHOP_STR[ 0 ] );
      break;
   }
   case 61: {
      write_buf( CHOP_STR[ 1 ] );
      break;
   }
   case 62: {
      write_buf( CHOP_STR[ 2 ] );
      break;
   }
   case 63: {
      write_buf( CHOP_STR[ 3 ] );
      break;
   }
   case 64: {
      sprintf( tmpc, "%3d", CHOP_FILE_NUM );
      write_buf( tmpc );
      break;
   }
   case 65: {
      sprintf( tmpc, "%3d", CHOP_PAIR );
      write_buf( tmpc );
      break;
   }
   case 66: {
      sprintf( tmpc, "%5d,%3d", CHOP_HIFI_GAIN, CHOP_PRE_GAIN );
      write_buf( tmpc );
      break;
   }
   case 67: {
      write_time( 22 );
      break;
   }
   case 68: {
      write_time( 23 );
      break;
   }
}
/* end display */ }



write_user( str_num )

int
   str_num;


{

char
   tmp[ 100 ];

if ( str_num == 6 && ( CMD_STR[ 0 ] != '\0' ) )
   strcpy( tmp, CMD_STR );
else if ( USER_STR[ str_num ][ 0 ] != '\0' ) 
   strcpy( tmp, USER_STR[ str_num ] );
else
   return;

strcat( tmp, ERASE_TO_EOL );

write_buf( tmp );

/* end write_user */ }




/*****
 *
 * Write out LST, GMT, RA, Desired RA 
 *
 *****/

write_time( val )

int
   val;

{

double 
   ra_out,
   dec_out,
   ha_out;

switch ( val ) {
   case 0 : {
      if ( LST == last_times[ val ] )
	 return;
      last_times[ val ] = LST;
      make_time_str( LST, 1 );
      break;
   }
   case 1 : {
      if ( UT_TIME == last_times[ val ] )
	 return;
      last_times[ val ] = UT_TIME;
      make_time_str( UT_TIME, 1 );
      break;
   }
   case 2 : {
      if ( tracking_info -> motion_type != NO_MOTION) 
         ra_out = RA - SIDEREAL;
      else 
         ra_out = RA;
      if ( ra_out == last_times[ val ] )
	 return;
      last_times[ val ] = ra_out;
      make_time_str( ra_out, 2 );
      break;
   }
   case 3 : {
      if ( DES_RA == last_times[ val ] )
	 return;
      last_times[ val ] = DES_RA;
      make_time_str( DES_RA, 2 );
      break;
   }
   case 4 : {
      if ( tracking_info -> motion_type != NO_MOTION) 
         ha_out = HA/15.0 + SIDEREAL;
      else 
         ha_out = HA/15.0;
      if ( ha_out == last_times[ val ] )
	 return;
      last_times[ val ] = ha_out;
      make_time_str( ha_out, 2 );
      break;
   }
   case 5 : {
      ha_out = DES_HA / 15.0;
      if ( ha_out == last_times[ val ] )
	 return;
      last_times[ val ] = ha_out;
      make_time_str( ha_out, 2 );
      break;
   }
   case 6 : {
      if ( DEC == last_times[ val ] )
	 return;
      last_times[ val ] = DEC;
      make_time_str( DEC, 1 );
      break;
   }
   case 7 : {
      if ( DES_DEC == last_times[ val ] )
	 return;
      last_times[ val ] = DES_DEC;
      make_time_str( DES_DEC, 1 );
      break;
   }
   case 8 : {
      if ( tracking_info -> motion_type != NO_MOTION) 
         ha_out = HA/15.0 + SIDEREAL;
      else 
         ha_out = HA/15.0;
      ha_out = DES_HA / 15.0 - ha_out;
      if ( ha_out == last_times[ val ] )
	 return;
      last_times[ val ] = ha_out;
      make_time_str( ha_out, 2 );
      break;
   }
   case 9 : {
      ha_out = DES_DEC - DEC;
      if ( ha_out == last_times[ val ] )
	 return;
      last_times[ val ] = ha_out;
      make_time_str( ha_out, 1 );
      break;
   }
   case 10 : {
      if ( COL_DEC == last_times[ val ] )
	 return;
      last_times[ val ] = COL_DEC;
      make_time_str( COL_DEC, 1 );
      break;
   }
   case 11 : {
      if ( COL_HA == last_times[ val ] )
	 return;
      last_times[ val ] = COL_HA;
      make_time_str( COL_HA, 1 );
      break;
   }
   case 12 : {
      if ( OFFSET_DEC == last_times[ val ] )
	 return;
      last_times[ val ] = OFFSET_DEC;
      make_time_str( OFFSET_DEC, 1 );
      break;
   }
   case 13 : {
      if ( OFFSET_HA == last_times[ val ] )
	 return;
      last_times[ val ] = OFFSET_HA;
      make_time_str( OFFSET_HA, 1 );
      break;
   }
   case 14 : {
      if ( tracking_info -> motion_type != NO_MOTION) 
         ra_out = RA - SIDEREAL;
      else 
         ra_out = RA;
      ra_out = DES_RA - ra_out;
      if ( ra_out == last_times[ val ] )
	 return;
      last_times[ val ] = ra_out;
      make_time_str( ra_out, 2 );
      break;
   }
   case 15 : { /* 1950 RA */
      if ( ORIG_RA == last_times[ val ] )
	 return;
      last_times[ val ] = ORIG_RA;
      make_time_str( ORIG_RA, 2 );
      break;
   }
   case 16 : { /* 1950 Dec */ 
      if ( ORIG_DEC == last_times[ val ] )
	 return;
      last_times[ val ] = ORIG_DEC;
      make_time_str( ORIG_DEC, 1 );
      break;
   }
   case 17 : { /* Secondary Offset RA */ 
      if ( OFFSET_2_HA == last_times[ val ] )
	 return;
      last_times[ val ] = OFFSET_2_HA;
      make_time_str( OFFSET_2_HA, 1 );
      break;
   }
   case 18 : { /* Secondary Offset RA */ 
      if ( OFFSET_2_DEC == last_times[ val ] )
	 return;
      last_times[ val ] = OFFSET_2_DEC;
      make_time_str( OFFSET_2_DEC, 1 );
      break;
   }
   case 19 : { /* Nod delta HA */
      if (  CHOP_RA == last_times[ val ] )
	 return;
      last_times[ val ] = CHOP_RA;
      make_time_str( CHOP_RA, -6 );
      break;
   }
   case 20 : { /* Nod delta Dec */
      if (  CHOP_DEC == last_times[ val ] )
	 return;
      last_times[ val ] = CHOP_DEC;
      make_time_str( CHOP_DEC, -6 );
      break;
   }
   case 21 : { /* Throw */
      ra_out = sqrt( CHOP_DEC * CHOP_DEC + CHOP_RA * CHOP_RA );
      if (  ra_out == last_times[ val ] )
	 return;
      last_times[ val ] = ra_out;
      make_time_str( ra_out, -6 );
      break;
   }
   case 22 : { /* Dial RA */
      ra_out = tracking_info -> dial_ha;
      if (  ra_out == last_times[ val ] )
	 return;
      last_times[ val ] = ra_out;
      make_time_str( ra_out, 1 );
      break;
   }
   case 23 : { /* Dial Dec */
      ra_out = tracking_info -> dial_dec;
      if (  ra_out == last_times[ val ] )
	 return;
      make_time_str( ra_out, 1 );
      break;
   }
} 

/* end write_time */ }



make_time_str( v, num_dec )

double
   v;

int
   num_dec;

{

double 
   val, t;

int
   save_dec, h, m, s, f;

char
   time_str[ 13 ];

strcpy( time_str, " 00:00:00   " );

save_dec = 0;
if ( num_dec < 0 ) {
   save_dec = -num_dec;
   num_dec = 1;
}

if ( num_dec > 0 ) {
   time_str[ 9 ] = '.';
   time_str[ 10 ] = '0';
   if ( num_dec > 1 )
      time_str[ 11 ] = '0';
}

val = v;

if ( val < 0.0 ) {
   val = fabs( val );
   time_str[ 0 ] ='-';
}

t = dec2hms( val );
h = ( int ) t;
t -= h;
if ( h > 9 )
   time_str[ 1 ] = h / 10 + 48;
time_str[ 2 ] = h % 10 + 48;

t *= 100.0;
m = ( int ) t;
t -= m;
if ( m > 9 )
   time_str[ 4 ] = m / 10 + 48;
time_str[ 5 ] = m % 10 + 48;

t *= 100.0;
s = ( int ) t;
t -= s;
if ( s > 9 )
   time_str[ 7 ] = s / 10 + 48;
time_str[ 8 ] = s % 10 + 48;

if ( num_dec > 0 ) {
   t *= 100.0;
   f = ( int ) t;
   t -= f;
   if ( f > 9 ) {
      time_str[ 10 ] = f / 10 + 48;
      if ( num_dec > 1 ) 
         time_str[ 11 ] = f % 10 + 48;
   }
}

if ( save_dec ) {
   save_dec = 10 - save_dec;
   time_str[ save_dec ] = time_str[ 0 ];
}
   
write_buf( time_str + save_dec );

/*  end make_time_str */ }



/*****
 *
 * Goto a specific column and row and place the goto string
 * into the field list of locations 
 *
 *****/

gotoxy( i, x, y )

int
   i, x, y;

{

char
   goto_str[ 9 ];

strcpy( goto_str, GOTOXY );

goto_str[ 3 ] = ( char ) ( y % 10 + 48 );
if ( y > 9 )
   goto_str[ 2 ] = ( char ) ( y / 10 + 48 );
else
   goto_str[ 2 ] = '0';

goto_str[ 6 ] = ( char ) ( x % 10 + 48 );
if ( x > 9 )
   goto_str[ 5 ] = ( char ) ( x / 10 + 48 );
else
   goto_str[ 5 ] = '0';

strcpy( GOTO_STRS[ i ], goto_str );

/* end gotoxy */ } 


/**********
 * Returns the time or angle in the form HH.MMSSssss or DD.MMSSssss
 * from the given argument in the form DD.dddddddd or HH.hhhhhhh
 **********/

double dec2hms(dec)

double dec;

{

double h, m, s;

h = (int) dec;
dec = ( dec - h ) * 3600.0;

m = (int) (dec / 60.0);

s = dec - (m * 60.0);

return(h + m / 100.0 + s / 10000.0);

/* END dec2hms */ }


setraw ( unit )

int unit;

{

   struct termio  tbuf;
   int flags;

   ioctl ( unit, TCGETA, &tbuf);

   if ( unit != 0 ) {
      tbuf.c_cflag &= ~CBAUD;
   /*   tbuf.c_cflag |= B19200; */
      tbuf.c_cflag |= B9600;
   }
   tbuf.c_iflag &= ~(INLCR | ICRNL | IUCLC | ISTRIP | IXON | BRKINT );
   tbuf.c_oflag &= ~OPOST;
   tbuf.c_lflag &= ~(ICANON | ISIG | ECHO);
   tbuf.c_cc[4] = 0;
   tbuf.c_cc[5] = 10;

   ioctl ( unit, TCSETAF, &tbuf);

   flags = fcntl( unit, F_GETFL, 0 );
   fcntl( unit, F_SETFL, flags | O_NDELAY );

/* end setraw */ }
