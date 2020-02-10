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
		"1.1C5E0F0G0I000K0L0000M000P0R0Y0", /* Drive   port -- output  */
		"1.1C0E0F0G0I000K0L0000M000P0R0Y0", /* Encoder port -- input   */
		"1.1C0E0F0G0I000K0L0000M000P0R0Y0", /* Paddle  port -- input   */
		"1.1C0E0F0G0I000K0L0000M000P0R0Y0"
	};

	static char *defaults[ 4 ] =             /* Configuration set.      */
	{
		"C5F0G0I000K0L0000M000P0R0Y0XS0X",  /* Drive   port -- output  */
		"C0F0G0I000K0L0000M000P0R0Y0XS0X",  /* Encoder port -- input   */
		"C0F0G0I000K0L0000M000P0R0Y0XS0X",  /* Paddle  port -- input   */
		"C0F0G0I000K0L0000M000P0R0Y0XS0X"
	};

	/* FIRST, open the path to the IB488 and clear all the devices */


	/******************** Init the drive channel  **************************/
	if (drive_present)
		init_port(drive_adr, "Drive", configs[0], defaults[0]);

	/******************** Init the encoder channel **********************/
	if (encoder_present) 
		init_port(encoder_adr, "Encoder", configs[1], defaults[1]);
	
	
	/******************** Init the paddle channel  *************************/
	if (paddle_present)
		init_port(paddle_adr, "Paddle", configs[2], defaults[2]);
	
	/******************** Init the paddle channel  *************************/
	if (hifi_present) {
		init_port(hifi_in_adr,  "HIFI in", configs[1], defaults[1]);
		init_port(hifi_out_adr, "HIFI out", configs[0], defaults[0]);
	}	
		
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
		if ( strcmp( msg, conf) != 0 )  /* & set it if it is wrong */
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
