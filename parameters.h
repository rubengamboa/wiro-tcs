/* This header file contains lots of parameter values that are used
   in the WIRO pointing system. 
   Set up by Patrick Broos, April 1994.

   Adjust HA_ZERO_RATE, DEC_ZERO_RATE,  and SIDERIAL_RATE from 2050, 2048
   & 147.  JSW, 9May2000.

   Adjust DEC_ZERO_RATE from 2048.  JSW, 7Jne2000.
*/

#define TRUE 1

/* For as long as possible, rates are calculated in a coordinate system
   where zero velocity corresponds to the value 0.
   The telescope has two speed ranges, tracking mode and slew mode.
   The values actually stored in the ha_rate and dec_rate slots of 
   shared memory have been encoded as the hardware requires and include
   bits that control slew mode and the preload motors.

   The maximum tracking speed (MAX_TRACK_SPEED)is set to 2040 rather 
   than 2047 to make sure that the encoded rate that is produced is in
   the range [0, 2^12-1].
   There is no check for overflow during the encoding process, and if
   overflow occurs, the telescope may move in the wrong direction!
   If HA_ZERO_RATE or DEC_ZERO_RATE changes, then MAX_TRACK_SPEED may
   have to be adjusted. 

   Usually, the siderial rate, SIDERIAL_RATE, is added to the calculated
   HA rate so that the telescope won't always lag behind.  
*/
#define MAX_TRACK_SPEED    2040.	/* maximum speed in tracking mode */
#define SLEW_TO_TRACK_GAIN 12.		/* ratio of slew to tracking speed */
#define MAX_SLEW_SPEED     (MAX_TRACK_SPEED * SLEW_TO_TRACK_GAIN)	
					/* maximum speed in slew mode */

#define TRACK_MASK	0xC000		/* OR with rate to get track mode */
#define HA_ZERO_RATE	2049.		/* HA rate for zero velocity */
#define DEC_ZERO_RATE	2044.		/* DEC rate for zero velocity */
#define SIDERIAL_RATE	 147.		/* HA rate for siderial velocity */
                                    /*  Diagnostics give rate for 3 */
                                    /*  drive preamp cards: #1,143; */
                                    /*  #2,138; and #3,141. Aug95 JSW */
/* There are 16384 encoder units per turn, or 32768 per degree. */
#define ENCODERS_PER_DEGREE	32768.	

/* Time in usecs between track updates */
#define TRACKTIME 		70000	

/* Number of ticks per second */
#define TICKS_PER_SECOND	(1e6 / TRACKTIME)
#define TICKS_PER_MINUTE	(60.0*TICKS_PER_SECOND)

