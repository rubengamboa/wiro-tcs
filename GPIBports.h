/*   This file should be included by all files using the GPIB. */
/*   Their are three levels of variable relevant to the GPIB:  */
/*                                                             */
/*  1) The number of the actual address of a whole GPIB device */
/*     designated gport*                                        */
/*  Work-around blown ttyS0.  Use ttyQ1a5.  24May00 JSW        */
/* 9Nov00 JSW: add focus constants.                            */

#ifndef _GPIBPorts
#define _GPIBPorts

typedef unsigned char * lpchar;


#define paddle_adr 5   
                       /* GPIB address of paddle  input port  */
#define drive_adr  16     
                       /* GPIB address of drive   output port */
#define encoder_adr  17     
                       /* GPIB address of encoder input port  */
#define hifi_out_adr  8 
					   /* GPIB address of hifi output port */
#define hifi_in_adr   9 
					   /* GPIB address of hifi input */

/* #define IEEESERIAL "/dev/ttyS0"      Linux for COM1 */
#define IEEESERIAL "/dev/ttyQ1a5"       /* USED WHEN COM1 DAMAGED */
/* #define IEEESERIAL "/dev/com1" */
					/* Name of the port the GPIB is attatched to */
/*************************************************************************/
/* Prototypes for functions  */
	/* Setup the interface */
void gpib_setup( void );
	/* Clear the port pointed to */
void gpib_clear(int port);        
	/* gpib_rds reads a string   */
void gpib_wrs(int gpadr, char * asdf  );                 
	/* Read a string of up to maxlength */
void gpib_rds(int gpadr,  char * msg, int maxlentgh);
	/* Read 5 data words int of them - in high speed mode */
int gpib_rd(int gpadr,  lpchar bits, int i);
	/* Write some data words - int of them in high speed mode */
void gpib_wr(int gpadr, lpchar, int i); 

/*************************************************************************/
/* Heres are some variables dealing with the hand paddle    */

#define SPEED_BITS 0x03

#define GUIDE 0x01
#define FAST  0x02
#define SET   0x03

#define NORTH 0x04
#define SOUTH 0x08
#define EAST  0x10
#define WEST  0x20

#define GUIDE_RATE  (   1.5  )        /* Rates in " per second */
#define SET_RATE    (  45.0  )
#define FAST_RATE   ( 200.0  )

/* Here are some variables for dealing with remote focusing: */

/** These are redefined in wiro.h

#define SLOWF 0x00
#define FASTF 0x01
#define HOLDF 0x00
#define INF   0x02
#define OUTF  0x04

**/

unsigned short
   gray2bin( unsigned short gval);

static int
   d80_addrs[ 4 ] = { -1, -1, -1, -1 };

/*************************************************************************/
/* These are handy data structures for reading stuff into from gpib */
/* Used with fast reading */

struct d80data  {                           /* IoTech D80 data */
		unsigned char   junk1;      /* Keeps byte alignment */
                unsigned char   dome;       /* Dome encoder or command */
                unsigned short  dec;        /* Dec  encoder or command */
                unsigned short  ha;         /* HA   encoder or command */
		unsigned long   junk2;      /* Padding for crlf     */
                } ;

struct d80data1 {                           /* IoTech D80 data type 2 */
		unsigned char   port1;      /* keeps byte alignment   */
		unsigned char   port2;      /* Paddle information     */
		unsigned char   port3;
		unsigned char   port4;
		unsigned char   port5;
		unsigned long   junk6;
} ;


struct d80data  drives,drives2;  	/* Commands to send to the drives */
struct d80data  encoders,encoders2; /* Values read from the encoders. */
struct d80data1 paddle;         /* Dome paddle & misc. info.      */

#endif
