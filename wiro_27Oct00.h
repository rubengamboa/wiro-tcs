/* All WIRO tracking shared memory constants, plus pointing model */
/* Howell  Aug. 15, 1994:  Added variables for nodding  */
/* Howell  Aug. 17, 1994:  Adjusted coarse coll.        */
/* Howell  Apr.  7, 1995:  Adjusted coarse coll. & dial */
/* Howell  Jul.  1, 1996:  Adjusted coarse coll. & dial */
/* Howell  Aug. 29, 1996:  Adjusted coarse coll. & dial */
/* Spillar 1999: move to Linux */
/* Weger   Feb. 18, 2000:  ADD_SCREEN defined; new_tscreen inserted */

#ifndef _wiro
#define _wiro

#include <time.h>
#include <sys/time.h>  

#define WIRO_REVISION           17
#define CORE "/home/observer/wiro/track/COREFILE"
/* #define CORE "/home/wiro/wiro/track/COREFILE" */
#define CATALOGS "/home/observer/wiro/catalogs/"
/* #define CATALOGS "/home/wiro/wiro/catalogs/" */

/******************* POINTING MODEL  ************************************/
/* All corrections are in arc-seconds and then converted to degrees     */
/* Pointing model from June 1994.                                       */
/************************************************************************/

#define DIAL_HA      (  -74.02 / 3600. )  /* Coarse HA  dial correction */
#define DIAL_DEC     ( 1113.48 / 3600. )  /* Coarse DEC dial correction */
                                          /* Sign is opposite offset and */
                                          /* fine dial correction.       */

#define COL_ZERO_HA  (   131.5 / 3600. )  /* Coarse HA  col  correction */
#define COL_ZERO_DEC (  -197.6 / 3600. )  /* Coarse DEC col  correction */
                                          /* Sign is same as offset and */
                                          /* fine coll correction.      */

#define Refr_H  (  60.13 / 3600. ) /* Refraction     -- HA          */
#define Refr_D  (  34.97 / 3600. ) /* Refraction     -- DEC         */

#define TFlx_H  (  98.32 / 3600. ) /* Tube flexure   -- HA          */
#define TFlx_D  (  -8.91 / 3600. ) /* Tube flexure   -- DEC         */

#define MAz_H   (   13.21 / 3600.) /* Polar Axis Az. -- HA  (cmn)   */
#define MAz_D   (   13.21 / 3600.) /* Polar Axis Az. -- DEC (cmn)   */
#define MEl_H   (  147.85 / 3600.) /* Polar Axis El. -- HA          */
#define MEl_D   (  133.58 / 3600.) /* Polar Axis El. -- DEC         */
#define NPerp   (  -15.69 / 3600.) /* Axes nonperp.  -- HA only     */

#define Dial_H  ( -158.28 / 3600.) /* Dial (encoder) -- HA          */
#define Dial_D  (   13.96 / 3600.) /* Dial (encoder) -- DEC         */

#define Col_H   (   92.20 / 3600.) /* Col (fcl.pln.) -- HA only     */

#define Emprc1  (  -40.22 / 3600.) /* Empirical #1   -- HA only     */
#define Emprc2  (   -5.11 / 3600.) /* Empirical #2   -- HA only     */


/************************************************************************/
/*  Here come handy defines to make it easier to reach useful variables */
/************************************************************************/

#define SIDEREAL ( 0.0/ 3600.0 )

#define HA          tinfo->tel_ha
#define RA          tinfo->tel_ra
#define DEC         tinfo->tel_dec
#define     V_RA    tinfo->tel_v_ra
#define     V_DEC   tinfo->tel_v_dec
#define PAD_V_RA    tinfo->tel_pv_ra
#define PAD_V_DEC   tinfo->tel_pv_dec

#define OBJECT_NAME tinfo->object_name
#define FILTER      tinfo->filter
#define FOCUS       tinfo->focus

#define DES_HA      tinfo->tel_des_ha
#define DES_RA      tinfo->tel_des_ra
#define DES_DEC     tinfo->tel_des_dec

#define COL_DEC     tinfo->col_dec
#define COL_HA      tinfo->col_ha

#define OFFSET_DEC      tinfo->offset_dec
#define OFFSET_HA       tinfo->offset_ha
#define OFFSET_2_DEC    tinfo->offset_2_dec
#define OFFSET_2_HA     tinfo->offset_2_ha
#define NOD_DEC			tinfo->nod_dec
#define NOD_HA			tinfo->nod_ha

#define NODDED			tinfo->nodded

#define ORIG_DEC        tinfo->orig_dec
#define ORIG_RA         tinfo->orig_ra
#define ABER_NUT_RA     tinfo->aber_nut_ra
#define ABER_NUT_DEC    tinfo->aber_nut_dec

#define LST             tinfo->time_cur_lst /* The current LST */
#define ELAPSED         tinfo->time_elapsed /* Time since something */
#define JULIAN          tinfo->julian       /* The julian date */
#define UT_TIME         tinfo->ut_time     /* UT hours */
#define CURRENT 	    tinfo->time_current   /* Time since Jan 1 1970 in
												 seconds */
#define START			tinfo->time_zero  /* time since tracking started */
#define TONOBJ			tinfo->time_object /* Start time on object */

#define AZI             tinfo->tel_azi
#define ALT             tinfo->tel_alt

#define HP              tinfo->hp

#define MONTH           tinfo->month
#define DAY             tinfo->day
#define YEAR            tinfo->year


#define TEMPERATURE     tinfo->temperature
#define PRESSURE        tinfo->pressure

#define EPOCH           tinfo->epoch

#define NO_MOTION 0
#define SOLAR     1
#define NON_SOLAR 2

/* Diagnostic modes; see diagnostic manual */
#define VELOCITY_MODE 1
#define POSITION_MODE 2
#define PASSIVE_MODE  3
#define IDLE_MODE     4

#define USER_STR        tinfo->usr_str
#define CMD_STR         tinfo->cmd_str

#define PADDLE_OFF 0
#define PADDLE_LOCAL 1
#define PADDLE_REMOTE 2

#define PADDLE_STATUS   tinfo->paddle_status
#define PADDLE_RATE     tinfo->paddle_rate
#define PADDLE_CMD      tinfo->paddle_cmd

#define HA_SERVO        tinfo->ha_servo
#define DEC_SERVO       tinfo->dec_servo

#define HIFI			tinfo->hifi

/*****
 *
 * Flags for tracking and screen programs  
 * ( possible values for tinfo->keep_tracking )
 *
 *****/
 
#define TRACK_STOP 0
#define TRACK_GO 1
#define REMOTE_REFRESH 2
#define ADD_SCREEN 3

struct wiro_memory {
   short 
      keep_tracking, /* Flag for tracking control 0-exit  1-keep tracking */
      no_motion,     /* Flag for whether or not the telescope recieves the strobe */
      motion_type;   /* Flag for type of telescope motion */

   int
      status;        /* Ever incrementing variable to show things are working */
   
   short 
      dome_on,       /* Flag do we control the dome? */
      dome_des_pos,  /* Desired position of dome */
      dome_enc,      /* Dome encoder position 0-255 */
      dome_offset;   /* Dome offset from zero */

   double
      tel_azi,        /* Telescope azimuth */
      tel_alt;        /* Telescope altitude */
   
   double
      tel_dec,     /* Declination of telescope in degrees */
      tel_ha,      /* Telescope hour-angle in degrees     */
      tel_ra,      /* Right ascension of the telescope    */

      tel_v_ra,    /* ra         velocity for moving objects */
      			/* seconds of ra per hour  */

      tel_v_dec,   /* dec        velocity for moving objects  */
      				/* seconds in dec per hour */
      tel_pv_ra,   /* ra  offset velocity commanded by paddle */
      tel_pv_dec,  /* dec offset velocity commanded by paddle */

      tel_des_dec, /* Desired declination in degrees */
      tel_des_ha,  /* Desired hour-angle             */
      tel_des_ra;  /* Desired RA                     */
      
   unsigned short 
      ha_enc,        /* HA Encoder */
      ha_turns;      /* HA turns */
      
   short
      ha_rate;       /* HA velocity */
 
   unsigned short 
      dec_enc,       /* DEC encoder */
      dec_turns;     /* DEC turns */
      
   short
      dec_rate;      /* DEC velocity */

   union {
      int32 whole;
      int16 halves[ 2 ];
   } ha, dec;

   struct timeval   	      /* LynxOS / POSIX fine time strcuture */
   	  time_current,	      /* Current time */ 
      time_zero,	      /* Time we started tracking */
      time_object;	      /* Time at which we started following object */

   double
      ut_time, 				/* UT hours*/
      time_cur_lst,         /* LST time */
      time_zero_lst,        /* LST time at startup */
      time_elapsed, 
      julian;               /* Julian date */
   
   double 
      temperature,      /* Current outside temperature degrees Fahrenheit */
      pressure;         /* Current atmospheric pressure at observatory ( inches Hg ) */

   double 
      epoch;         /* Epoch of the coordinates */

   double 
      hp;            /* Horizontal Parallax */

   double
      offset_ha,    /* Hand paddle offset for HA */
      offset_dec,   /* Hand paddle offset for dec */
      offset_2_ha,  /* Secondary offset for HA */
      offset_2_dec, /* Secondary offset for dec */
	  nod_ha,	 	/* Nod offset for HA  A beam */
	  nod_dec;	 	/* Nod offset for DEC B beam */

   int
   	  nodded;		/* Bit 0 high -> hardward command of B beam */
   	  				/* Bit 1 high -> software command of B beam */
   	  				/* The B beam command is the logical OR */
   	  				/* of the hardware and software B beam commands */
   	  				/* Both must be zero to command the A beam. */

   double
      dial_ha,       /* dial correction variable */
      dial_dec,      /* dial correction variable */
      col_ha,        /* Collimation error for HA */
      col_dec;       /* Collimation error for Dec */

   double
      orig_dec,     /* Original Dec in 1950 or 2000 coordinates: degrees */
      orig_ra,      /* Original RA in 1950 or 2000 coordinates : hours */
      aber_nut_ra,  /* Aberration and nutation in ra            */
      aber_nut_dec; /* Aberration and nutation in dec           */

   char
      object_name[ 80 ],    /* Name of the object being observed */
      device_name[ 80 ],    /* Name of device taking data */
      filter[ 10 ];         /* Name of filter being used */
 
   double
      focus,                /* Current focus value */
  	  integration_time;     /* Object integration time in seconds */


   short
      month,
      day,
      year;

   int
      paddle_status;
   
   double
      paddle_rate;

   char
      paddle_cmd[ 10 ];

   double
      ha_servo,
      dec_servo;
      
   char
      usr_str[ 7 ][ 80 ],
      cmd_str[ 80 ],
      new_tscreen[ 15 ];

	unsigned char	
		hifi[80];		

	int clockcount;  /* clockcount is used to be sure the clock is running */
};

typedef struct {
   int
      revision;
} REVISION_INFO;

#endif
