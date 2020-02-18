/* ================================================================= 
   Pointing System Diagnostic Software
   Patrick Broos, April 1994 

   The function do_diagnostic is an interactive thread in track.c.

   Rates are calculated in a coordinate system where zero velocity 
   corresponds to the value 0.  They are encoded for the hardware
   at the very last minute in get_velocity_from_diag().

   RRH 96/03/05  Modified print statement for net printing
   				 through Guildenstern
   JSW 2Dec02  Modified print statement so that it works from Penguin
   ================================================================= */

#include "parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "GPIBports.h"
#include "wirotypes.h"
#include "track.h"
#include "wtrack.h"         /* This must come before wiro */
#include "wiro.h"

#define min(x,y) (x < y ? x : y)

/* ================================================================= 
   Defines and Type definitions for this thread
   ================================================================= */
#define INT		0
#define LONG	1
#define FLOAT	2
#define DOUBLE	3

struct int_parameter {
	int    Type;
	int    Val;
	int    Min;
	int    Max;
	char   Prompt[80];
	char   Name[80];
	};

struct long_parameter {
	int    Type;
	long   Val;
	long   Min;
	long   Max;
	char   Prompt[80];
	char   Name[80];
	};

struct float_parameter {
	int    Type;
	float  Val;
	float  Min;
	float  Max;
	char   Prompt[80];
	char   Name[80];
	};

struct double_parameter {
	int    Type;
	double Val;
	double Min;
	double Max;
	char   Prompt[80];
	char   Name[80];
	};

typedef union {	struct int_parameter 	 Int;
				struct long_parameter	 Long;
				struct float_parameter  Float;
				struct double_parameter Double; } parameter; 

#define MAXTICKS 10000		/* the physical length of TickData */
#define MAXPLOTS 10000		/* the physical length of PlotData */

#define TIME_AXIS 0
#define TICK_AXIS 1

#define VELOCITY_DISPLAYS 0
#define POSITION_DISPLAYS 1
#define PASSIVE_DISPLAYS  2

/* To compute velocities, we estimate a change in position DelPos
   over some time interval.  Telescope position is known to a 
   resolution of one encoder tick.
   A quick diagram will reveal that the actual change in position
   is in the range (DelPos-1,DelPos+1) in encoder units. 
   Thus the error in DelPos is 1 encoder unit.
   This error, expressed in degrees is defined below. */
#define DEL_POS_ERROR (1.0 / ENCODERS_PER_DEGREE)

/* ================================================================= 
   External Vars and Function Prototypes
   ================================================================= */
extern struct wiro_memory *tinfo;     /* Pointer to shared tracking info */
extern struct d80data     drives;
extern int   debug;

void  do_turns			(void);

void  do_diagnostic		(void);
void  save_diagnostic_state 	(void);
void  get_position_from_diag	(void);
void  get_velocity_from_diag	(void);
void  set_zero_velocity		(void);
void  simulate_telescope	(void);

void  velocity_menu		(void);
void  position_menu		(void);
void  display_menu		(int OptionSet);
char* menu				(char *Choices[]);
int   query				(char *Prompt);
void  get_parameter		(parameter *Par);

void  passive_track 	(int CollectData);
void  vel_multistep		(void);
void  vel_step			(void);
void  pos_step 			(void);
void  pos_ramp 			(void);
void  check_safety_limits	(void);
short encode_ha_rate 		(double Velocity);
short encode_dec_rate 		(double Velocity); 
double decode_ha_rate 		(short Rate);
double decode_dec_rate 		(short Rate);
void  init_diagnostics 		(void);
void  run_test				(int Mode);

void  plot_vactual_vdesired	(void);
void  plot_vactual_time		(int Style);
void  plot_vdesired_time	(void);
void  plot_pdesired_time	(void);
void  plot_pactual_time		(void);
void  plot_encoder_bit		(void);
void  plot_error_time		(void);

void  show_plot				(void);
void  save_plot				(void);


/* ================================================================= 
   Global vars for this thread
   ================================================================= */

/* test waveform and measurement waveform storage */
struct {
	     double desired_ha;			/* desired HA  position */
	     double desired_dec;		/* desired DEC position */
	     double desired_ha_rate;	/* desired HA  velocity */
	     double desired_dec_rate;	/* desired DEC velocity */
	     double actual_ha;			/* actual  HA  position */
	     double actual_dec;			/* actual  DEC position */
	     double ha_error;			/* error in HA  */
	     double dec_error;			/* error in HA  */
 unsigned short ha_encoder;			/* HA encoder value */
 unsigned short dec_encoder;		/* DEC encoder value */
	} TickData [MAXTICKS];

long TestLength;		/* the logical length of TickData */
long TickNumber;		/* an index into TickData < TestLength */

/* plotting variables */
float PlotX   [MAXPLOTS];
float PlotY   [MAXPLOTS];
float PlotErr [MAXPLOTS];
long  PlotLength;		/* the logical length of PlotX,Y */
char *PlotXlabel, *PlotYlabel;
char  PlotTitle   [100];
char  PlotSubtitle[100];

/* testing configuration variables */
int  DiagnosticMode 	= 0;
char *HA_AXIS  		= "hour angle";
char *DEC_AXIS		= "declination";
char *CurrentAxis;

int   SlewModeFlag = 0;	/* set if slew mode entered */

/* ================================================================= 
   Diagnostic thread function and user interface functions
   ================================================================= */
/* ----------------------------------------------------------------- 
   do_diagnostic function
   ----------------------------------------------------------------- */
void do_diagnostic(void)
{
  char *MENUquit		= "Quit";
  char *MENUha_axis		= "Select HA axis";
  char *MENUdec_axis	= "Select DEC axis";
  char *MENUvelocity	= "Velocity Mode Tests";
  char *MENUposition	= "Position Mode Tests";
  char *MENUtrack		= "Go to normal tracking mode";
  char *MENUpassive		= "Go to normal tracking mode and collect data";
  char *Command, *Choices [] = 
  		 {MENUquit, MENUvelocity, MENUposition, MENUha_axis, MENUdec_axis, 
  		  MENUtrack, MENUpassive, NULL}; 

  init_diagnostics();

  do
   {
   printf("\nDIAGNOSTICS MAIN MENU; %s axis\n", CurrentAxis);
   Command = menu( Choices );

        if (Command == MENUvelocity)	velocity_menu();
   else if (Command == MENUposition)	position_menu();
   else if (Command == MENUha_axis)		CurrentAxis = HA_AXIS;
   else if (Command == MENUdec_axis)	CurrentAxis = DEC_AXIS;
   else if (Command == MENUtrack)		passive_track(0);
   else if (Command == MENUpassive)		passive_track(TRUE);
   }
  while (Command != MENUquit);

  printf("goodbye\n");
  tinfo->keep_tracking = TRACK_STOP;
}

void velocity_menu(void)
{
  char *MENUdone		= "Done";
  char *MENUstep		= "Step Response; Constant Velocity";
  char *MENUmultistep	= "Stair-step Response";
  char *Command, *Choices [] = {MENUdone, MENUstep, MENUmultistep, NULL};
  do
   {
   printf("\nVELOCITY TESTS on %s axis; (%d ticks = 1 second)\n", 
   		  CurrentAxis, (int)TICKS_PER_SECOND);
   Command = menu( Choices );

        if (Command == MENUstep)		vel_step();
   else if (Command == MENUmultistep)	vel_multistep();
   }
  while (Command != MENUdone);
}

void position_menu(void)
{
  char *MENUdone	= "Done";
  char *MENUstep	= "Step Response; Constant Position";
  char *MENUramp	= "Continuous Ramp (constant velocity)";
  char *Command, *Choices [] = {MENUdone, MENUstep, MENUramp, NULL};
  do
   {
   printf("\nPOSITION TESTS on %s axis; (%d ticks = 1 second)\n", 
   		  CurrentAxis, (int)TICKS_PER_SECOND);
   Command = menu( Choices );

        if (Command == MENUstep)	pos_step();
   else if (Command == MENUramp)	pos_ramp();
   }
  while (Command != MENUdone);
}

void display_menu(int OptionSet)
{
  char *MENUdone	= "Done";
  char *MENUv_v		= "Plot desired vs actual velocity";
  char *MENUvinput  = "Plot desired velocity vs time";
  char *MENUv_time	= "Plot actual velocity vs time (seconds)";
  char *MENUv_ticks	= "Plot actual velocity vs time";
  char *MENUpinput  = "Plot desired position vs time";
  char *MENUoutput  = "Plot actual position vs time";
  char *MENUerror   = "Plot position error";
  char *MENUencbits = "Plot encoder bit";
  char *Command; 
  char *VelChoices [] =  {MENUdone, MENUv_v, MENUvinput, 
  						  MENUv_ticks, MENUoutput, MENUencbits, NULL};
  char *PosChoices [] =  {MENUdone, MENUvinput, MENUv_ticks, 
  						  MENUpinput, MENUoutput, MENUerror, MENUencbits, NULL};
  char *PasChoices [] =  {MENUdone, MENUvinput, MENUv_ticks, 
  						  MENUoutput, MENUerror, MENUencbits, NULL};
  do
   {
   printf("\nEXAMINE TEST RESULTS:\n");
   switch (OptionSet) {
     case VELOCITY_DISPLAYS: Command = menu( VelChoices );
     						 break;
     case POSITION_DISPLAYS: Command = menu( PosChoices );
     						 break;
     case PASSIVE_DISPLAYS : Command = menu( PasChoices );
     						 break;
     }

        if (Command == MENUv_v)		plot_vactual_vdesired();
   else if (Command == MENUv_time)	plot_vactual_time(TIME_AXIS);
   else if (Command == MENUv_ticks)	plot_vactual_time(TICK_AXIS);
   else if (Command == MENUvinput)	plot_vdesired_time();
   else if (Command == MENUpinput)	plot_pdesired_time();
   else if (Command == MENUoutput)	plot_pactual_time();
   else if (Command == MENUencbits)	plot_encoder_bit();
   else if (Command == MENUerror)	plot_error_time();
   }
  while (Command != MENUdone);
}

/* ================================================================= 
   Diagnostic tests
   ================================================================= */
/* ----------------------------------------------------------------- 
   Passive tracking test
   ----------------------------------------------------------------- */
void passive_track (int CollectData)
{
  static struct long_parameter Duration = 
  	{LONG, TICKS_PER_MINUTE, 1, MAXTICKS, 
  	"data collection period (ticks)"};

  if (CollectData)
    {
    printf("This test will return the telescope to normal tracking mode\n"
    	   "and will passively collect data for a while.\n");

    get_parameter((parameter*) &Duration);
    TestLength = Duration.Val;

    run_test(PASSIVE_MODE);
    sprintf(PlotTitle, "%s", "Passive Observation of Tracking");
    display_menu(PASSIVE_DISPLAYS);
    }
  else
    DiagnosticMode = 0;
  printf("The telescope is now in normal tracking mode!\n");
}

/* ----------------------------------------------------------------- 
   Velocity Step Response and Constant Rate Test
   ----------------------------------------------------------------- */
void vel_step (void)
{
  static struct double_parameter Velocity1 =
  		{DOUBLE, 0, -MAX_SLEW_SPEED, MAX_SLEW_SPEED, 
  		"velocity before step  "};

  static struct double_parameter Velocity2 =
  		{DOUBLE, SIDERIAL_RATE, -MAX_SLEW_SPEED, MAX_SLEW_SPEED, 
  		"velocity after step "};

  static struct long_parameter Dwell1 =
  		{LONG, TICKS_PER_SECOND, 0, MAXTICKS, 
  		"dwell before step (ticks)   "};

  static struct long_parameter Dwell2 =
  		{LONG, 10*TICKS_PER_SECOND, 0, MAXTICKS, 
  		"dwell after step (ticks)   "};

  long   i;

  /* setup input waveform */

  printf("Velocity Step Test Parameters (+ is N or W) \n");
  get_parameter((parameter*) &Velocity1);
  get_parameter((parameter*) &Velocity2);
  get_parameter((parameter*) &Dwell1);
  get_parameter((parameter*) &Dwell2);

  TestLength   = Dwell1.Val + Dwell2.Val + 1;
  if (TestLength >= MAXTICKS)
    {
    printf("Test is too long! Modify parameters.\n");
    return;
    }
  for (i=0; i<Dwell1.Val; i++)
    if (CurrentAxis == HA_AXIS)
        {
        TickData[i].desired_ha_rate  = Velocity1.Val;
		TickData[i].desired_dec_rate = 0.0;
        }
    else
        {
        TickData[i].desired_ha_rate  = 0.0;
        TickData[i].desired_dec_rate = Velocity1.Val;
        }

  for (i=Dwell1.Val; i<TestLength; i++)
    if (CurrentAxis == HA_AXIS)
        {
        TickData[i].desired_ha_rate  = Velocity2.Val;
		TickData[i].desired_dec_rate = 0.0;
        }
    else
        {
        TickData[i].desired_ha_rate  = 0.0;
        TickData[i].desired_dec_rate = Velocity2.Val;
        }

  /* run test */
  run_test(VELOCITY_MODE);
  sprintf(PlotTitle, "%s; %d-->%d", 
  		  "Velocity Step Response", (int)Velocity1.Val, (int)Velocity2.Val);
  display_menu(VELOCITY_DISPLAYS);
}

/* ----------------------------------------------------------------- 
   Velocity Stair-step Test
   We need to check that the values in will not violate
   any test constraints.
   ----------------------------------------------------------------- */
void vel_multistep (void)
{
  static struct int_parameter Start =
  		{INT, 0, -MAX_SLEW_SPEED, MAX_SLEW_SPEED, 
  		"starting velocity               "};

  static struct int_parameter Increment =
  		{INT, 1, -MAX_SLEW_SPEED, MAX_SLEW_SPEED, 
  		"velocity increment              "};

  static struct int_parameter Levels =
  		{INT, 8, 1, MAXTICKS, 
  		"number of levels               "};

  static struct int_parameter Dwell =
  		{INT, TICKS_PER_SECOND, 1, MAXTICKS, 
  		"dwell time at each level (ticks)"};

  int    i, j; 
  long   Index;
  double Velocity;

  /* setup input stair-step */

  printf("Velocity Stair-step Test Parameters (+ is N or W)\n"); 
  get_parameter((parameter*) &Start);
  get_parameter((parameter*) &Increment);
  get_parameter((parameter*) &Levels);
  get_parameter((parameter*) &Dwell);

  Index        = 0;
  Velocity     = Start.Val;
  for (i=0; i<Levels.Val; i++)
    {
    for (j=0; j<Dwell.Val; Index++, j++)
      {
      if (Index >= MAXTICKS) 
        {
        printf("Test is too long! Modify parameters.\n");
        return;
        }
      if (CurrentAxis == HA_AXIS)
        {
        TickData[Index].desired_ha_rate  = Velocity;
		TickData[Index].desired_dec_rate = 0.0;
        }
      else
        {
        TickData[Index].desired_ha_rate  = 0.0;
        TickData[Index].desired_dec_rate = Velocity;
        }
      }
    Velocity += Increment.Val; 
    }

  /* add one extra tick to keep velocity calculation happy */
  TickData[Index].desired_ha_rate = TickData[Index-1].desired_ha_rate ;
  TickData[Index].desired_dec_rate= TickData[Index-1].desired_dec_rate;
  Index++;

  TestLength = Index;

  /* run test */
  run_test(VELOCITY_MODE);
  sprintf(PlotTitle, "%s; start = %d; dwell = %d ticks", 
  		  "Velocity Stair-step Test", Start.Val, Dwell.Val);
  display_menu(VELOCITY_DISPLAYS);
}

/* ----------------------------------------------------------------- 
   Position Step Response and Constant Position Test
   ----------------------------------------------------------------- */
void pos_step (void)
{
  double InitialHA, InitialDEC;

  static struct double_parameter ArcSecOffset =
  		{DOUBLE, 60, -720000, 720000, 
  		"step offset (arcseconds)"};

  static struct long_parameter Dwell1 =
  		{LONG, TICKS_PER_SECOND, 0, MAXTICKS, 
  		"dwell before step (ticks)"};

  static struct long_parameter Dwell2 =
  		{LONG, 10*TICKS_PER_SECOND, 0, MAXTICKS, 
  		"dwell after step (ticks)"};

  long   i;

  /* setup input waveform */

  printf("Position Step Test Parameters (+ is N or W)\n");
  get_parameter((parameter*) &ArcSecOffset);
  get_parameter((parameter*) &Dwell1);
  get_parameter((parameter*) &Dwell2);

  TestLength   = Dwell1.Val + Dwell2.Val + 1;
  if (TestLength >= MAXTICKS)
    {
    printf("Test is too long! Modify parameters.\n");
    return;
    }

  InitialHA  = HA;
  InitialDEC = DEC;

  for (i=0; i<Dwell1.Val; i++)
    {
    TickData[i].desired_ha  = InitialHA;
    TickData[i].desired_dec = InitialDEC;
    }

  for (i=Dwell1.Val; i<TestLength; i++)
    if (CurrentAxis == HA_AXIS)
        {
        TickData[i].desired_ha  = InitialHA + (ArcSecOffset.Val / 3600.);
        TickData[i].desired_dec = InitialDEC;
        }
    else
        {
        TickData[i].desired_ha  = InitialHA; 
        TickData[i].desired_dec = InitialDEC + (ArcSecOffset.Val / 3600.);
        }

  /* run test */
  run_test(POSITION_MODE);
  sprintf(PlotTitle, "%s; offset = %5.1lf \"", 
  		  "Position Step Response", ArcSecOffset.Val);
  display_menu(POSITION_DISPLAYS);
}

/* ----------------------------------------------------------------- 
   Position Continuous Ramp Test
   ----------------------------------------------------------------- */
void pos_ramp (void)
{
  double InitialHA, InitialDEC, Offset; /* degrees */
  long   i, Dwell = (long)1*TICKS_PER_SECOND; 


  static struct double_parameter Velocity =
  		{DOUBLE, 15, -MAX_SLEW_SPEED*15./SIDERIAL_RATE, 
  					  MAX_SLEW_SPEED*15./SIDERIAL_RATE, 
  		"slope of position ramp (arcsec/sec) "};

  static struct long_parameter Levels =
  		{LONG, 10*TICKS_PER_SECOND, 0, MAXTICKS, 
  		"length of ramp (ticks)"};

  /* setup input ramp */

  printf("Position Ramp Test Parameters (+ is N or W)\n");
  get_parameter((parameter*) &Velocity);
  get_parameter((parameter*) &Levels);

  TestLength   = Dwell + Levels.Val;
  if (TestLength >= MAXTICKS)
    {
    printf("Test is too long!\n");
    return;
    }

  InitialHA  = HA;
  InitialDEC = DEC;

  for (i=0; i<Dwell; i++)
    {
    TickData[i].desired_ha  = InitialHA;
    TickData[i].desired_dec = InitialDEC;
    }

  for (i=Dwell; i<TestLength; i++)
    {
    Offset = (Velocity.Val / 3600.) * (i-Dwell) * (TRACKTIME / 1e6);  
    if (CurrentAxis == HA_AXIS)
        {
        TickData[i].desired_ha  = InitialHA + Offset;
        TickData[i].desired_dec = InitialDEC;
        }
    else
        {
        TickData[i].desired_ha  = InitialHA; 
        TickData[i].desired_dec = InitialDEC + Offset;
        }
    }

  /* run test */
  run_test(POSITION_MODE);
  sprintf(PlotTitle, "%s; velocity = %5.1lf (\"/sec)", 
  		  "Position Ramp Test", Velocity.Val);
  display_menu(POSITION_DISPLAYS);
}

/* ----------------------------------------------------------------- 
   Function to run a test
   We have added a toggle of no_motion flag as a temporary fix for
   the bizarre hardware hang.
   ----------------------------------------------------------------- */
void run_test (int Mode)
{
  char* ModeName;

  switch (Mode) {
   case VELOCITY_MODE: ModeName = "velocity mode"; break;
   case POSITION_MODE: ModeName = "position mode"; break;
   case PASSIVE_MODE:  ModeName = "passive mode"; break;
   }
  tinfo->no_motion = 1;
  printf("Starting %s test of length %ld ticks (%d seconds),\n"
  		 "turn telescope OFF to interrupt.\n", 
  		 ModeName, TestLength, (int)(TestLength / TICKS_PER_SECOND));
  sleep(1);
  tinfo->no_motion = 0;

  if (Mode == POSITION_MODE) tinfo->motion_type = NO_MOTION;

  SlewModeFlag = 0;
  TickNumber     = 0;
  DiagnosticMode = Mode;
  while (DiagnosticMode != IDLE_MODE)
    {
    if (tinfo->no_motion) 
      {
      DiagnosticMode = IDLE_MODE;
      printf("TEST INTERRUPTED; TELESCOPE IS OFF.\n");
      }
    check_safety_limits(); printf("."); fflush(stdout);
    sleep(1);
    }
  printf("Test complete.\n");
  if (SlewModeFlag) printf("Warning! Test entered slew mode.\n");
}

/* ================================================================= 
   Functions that allow communication between the diagnostic 
   thread and the trackloop thread.
   ================================================================= */
/* This function saves the state (position) of the telescope */

void save_diagnostic_state (void)
{
  TickData[TickNumber].actual_ha  = HA;
  TickData[TickNumber].actual_dec = DEC;
  TickData[TickNumber].ha_encoder = tinfo->ha_enc;
  TickData[TickNumber].dec_encoder= tinfo->dec_enc;

  if (DiagnosticMode != VELOCITY_MODE)
    {
    TickData[TickNumber].desired_ha_rate  = decode_ha_rate ( tinfo->ha_rate );
    TickData[TickNumber].desired_dec_rate = decode_dec_rate( tinfo->dec_rate );
    }

  TickData[TickNumber].ha_error  = DES_HA   - HA;
  TickData[TickNumber].dec_error = DES_DEC  - DEC;

  if (++TickNumber >= TestLength) DiagnosticMode = IDLE_MODE;
}

/* ----------------------------------------------------------------- 
   This function replaces do_angles, do_corrections, do_tracking, i.e.
   it sets the desired (HA,DEC) position in shared memory. */

void get_position_from_diag(void)
{
tinfo->tel_des_ha  = TickData[TickNumber].desired_ha;
tinfo->tel_des_dec = TickData[TickNumber].desired_dec;
}

/* ----------------------------------------------------------------- 
   This function replaces do_rates, i.e. it sets the desired velocities
   in both shared memory and in the drives structure used by gpib_wr. */

void get_velocity_from_diag(void)
{
  double HArate, DECrate;

  HArate  = TickData[TickNumber].desired_ha_rate;
  DECrate = TickData[TickNumber].desired_dec_rate;

  tinfo->ha_rate  = encode_ha_rate ( HArate);
  tinfo->dec_rate = encode_dec_rate(DECrate);

  drives.ha  = tinfo->ha_rate ;    /* load the rates into working area */
  drives.dec = tinfo->dec_rate;

  if (debug)  {
    sprintf(USER_STR[1], "Rates: %8.0f  %8.0f", HArate, DECrate);
    sprintf(USER_STR[2], "Cmds: %8d  %8d", tinfo ->ha_rate, tinfo ->dec_rate);
    }
}

void set_zero_velocity(void)
{
  double HArate, DECrate;

  HArate  = 0.0;
  DECrate = 0.0;

  tinfo->ha_rate  = encode_ha_rate (0);
  tinfo->dec_rate = encode_dec_rate(0);

  drives.ha  = tinfo->ha_rate ;    /* load the rates into working area */
  drives.dec = tinfo->dec_rate;

  if (debug)  {
    sprintf(USER_STR[1], "Rates: %8.0f  %8.0f", HArate, DECrate);
    sprintf(USER_STR[2], "Cmds: %8d  %8d", tinfo ->ha_rate, tinfo ->dec_rate);
    }
}

/* This function, called by do_encoders, updates shared memory to 
   simulate telescope motion 

   HArate and DECrate are the zero-based velocities commanded during 
   the past tick.  DegreePerRate is the motion (in degrees of arc) 
   that each rate unit should produce in one tick. 
   EncoderPerRate is the same motion in encoder units. 
*/
   
void  simulate_telescope	(void)
{
  static unsigned short PreviousHAenc, PreviousDECenc;
                 double HArate, DECrate, DegreePerRate, EncoderPerRate;

  if ( tinfo->no_motion ) return;

   DegreePerRate = (TRACKTIME*360.)/(SIDERIAL_RATE*24.*3600.*1e6);
  EncoderPerRate = DegreePerRate * ENCODERS_PER_DEGREE;

  /* decode rate values sent to hardware */
  HArate  = decode_ha_rate ( tinfo->ha_rate );
  DECrate = decode_dec_rate( tinfo->dec_rate );

  PreviousHAenc  += ((unsigned short)(HArate  * EncoderPerRate)) << 2;
  PreviousDECenc += ((unsigned short)(DECrate * EncoderPerRate)) << 2;

  tinfo->ha_enc   = PreviousHAenc; 
  tinfo->dec_enc  = PreviousDECenc;

  /* Before I needed to simulate the encoder slots, I simply updated
     the HA and DEC slots as below and this function was called at
     the end of do_angles.

  static         double PreviousHA, PreviousDEC;
  PreviousHA     += HArate  * DegreePerRate;
  PreviousDEC    += DECrate * DegreePerRate;

              HA  = PreviousHA;
              DEC = PreviousDEC;
   */
}


/* ================================================================= 
   MISC FUNCTIONS
   ================================================================= */
/* ----------------------------------------------------------------- 
   Initialization
   ----------------------------------------------------------------- */
void init_diagnostics (void)
{
  DiagnosticMode = IDLE_MODE; /* stop telescope */
  CurrentAxis    = HA_AXIS;
}

/* ----------------------------------------------------------------- 
   Telescope safety function
   ----------------------------------------------------------------- */
void  check_safety_limits(void)
{
}


/* ================================================================= 
   USER INTERFACE FUNCTIONS
   ================================================================= */
/* ----------------------------------------------------------------- 
   Menu function
   Choices is an array of pointers to strings, terminated by NULL.
   ----------------------------------------------------------------- */
char* menu( char *Choices[] )
{
  int Nchoices = 0;
  int LabelNum, UsersChoice;

  while (Choices[Nchoices] != NULL)
    Nchoices++;

  do 
    {
    for (LabelNum = 0; LabelNum < Nchoices; LabelNum++)
      printf("  %d:%s\n", LabelNum, Choices[LabelNum]);

    printf("?");
    if (scanf("%d", &UsersChoice) == 0) 
      {
      printf("INPUT ERROR!\n");
      return Choices[0];
      }
    }
  while (UsersChoice < 0 || UsersChoice >= Nchoices);

  return Choices[UsersChoice];
}

/* ----------------------------------------------------------------- 
   Query Function; used to prompt user for yes/no questions.
   ----------------------------------------------------------------- */
int query(char *Prompt)
{
  int Answer = ' ';

  printf("\n%s (y/n):", Prompt);
  while (Answer == ' ' || Answer == '\n') Answer = getc(stdin);
  return ('y' == Answer || 'Y' == Answer);
}

/* ----------------------------------------------------------------- 
   Parameter Input Function; used to prompt user for parameters.
   ----------------------------------------------------------------- */
void get_parameter(parameter *Par)
{
  int    Nscanned, Done=0;
  char   Response[80], c;
  int    IntVal;
  long   LongVal;
  float  FloatVal;
  double DoubleVal;

  /* Need to flush input buffer here to get rid of residual EOLN's */
  /* The behaviour of fflush is not defined on input streams, but it
     seems to work in LynxOS. */
  /* fflush(stdin); */
  while ((( c=getchar() ) != '\n' ) && ( c != EOF ));

  switch (Par->Int.Type) {
   case INT:
    do
     {
     printf("%s (%d) :", Par->Int.Prompt, Par->Int.Val);
     fgets(Response, sizeof(Response), stdin);

     Nscanned = sscanf(Response, "%d", &IntVal);
     if (Nscanned == 1)
       {
       Done = (IntVal >= Par->Int.Min && IntVal <= Par->Int.Max);
       if (Done)
         Par->Int.Val = IntVal;
       else
         printf("ERROR: Value %d must be in range [%d,%d].\n",
       		    IntVal, Par->Int.Min, Par->Int.Max);
       }
     else Done = 1;
     }
    while (!Done);
    break;

   case LONG:
    do
     {
     printf("%s (%ld) :", Par->Long.Prompt, Par->Long.Val);
     fgets(Response, sizeof(Response), stdin);

     Nscanned = sscanf(Response, "%ld", &LongVal);
     if (Nscanned == 1)
       {
       Done = (LongVal >= Par->Long.Min && LongVal <= Par->Long.Max);
       if (Done)
         Par->Long.Val = LongVal;
       else
         printf("ERROR: Value %ld must be in range [%ld,%ld].\n",
       		    LongVal, Par->Long.Min, Par->Long.Max);
       }
     else Done = 1;
     }
    while (!Done);
    break;

   case FLOAT:
    do
     {
     printf("%s (%f) :", Par->Float.Prompt, Par->Float.Val);
     fgets(Response, sizeof(Response), stdin);

     Nscanned = sscanf(Response, "%f", &FloatVal);
     if (Nscanned == 1)
       {
       Done = (FloatVal >= Par->Float.Min && FloatVal <= Par->Float.Max);
       if (Done)
         Par->Float.Val = FloatVal;
       else
         printf("ERROR: Value %f must be in range [%f,%f].\n",
       		    FloatVal, Par->Float.Min, Par->Float.Max);
       }
     else Done = 1;
     }
    while (!Done);
    break;

   case DOUBLE:
    do
     {
     printf("%s (%lf) :", Par->Double.Prompt, Par->Double.Val);
     fgets(Response, sizeof(Response), stdin);

     Nscanned = sscanf(Response, "%lf", &DoubleVal);
     if (Nscanned == 1)
       {
       Done = (DoubleVal >= Par->Double.Min && DoubleVal <= Par->Double.Max);
       if (Done)
         Par->Double.Val = DoubleVal;
       else
         printf("ERROR: Value %lf must be in range [%lf,%lf].\n",
       		    DoubleVal, Par->Double.Min, Par->Double.Max);
       }
     else Done = 1;
     }
    while (!Done);
    break;
   default:
    printf("Unknown parameter type!\n");
  }

}					


/* ================================================================= 
   Functions to encode and decode rates
   If rate is faster than MAX_TRACK_SPEED, then we enter slew mode,
   turn off the preload motor, and lower the rate value by a factor
   of SLEW_TO_TRACK_GAIN.
   Slew mode and preloads are controlled by the bit mask TRACK_MASK.
   ================================================================= */
/* SlewMode flag is set if slew mode is used */
short encode_ha_rate (double Velocity)
{
  /* Clip speed at maximum slew speed */
  if (Velocity >  MAX_SLEW_SPEED) Velocity =  MAX_SLEW_SPEED;
  if (Velocity < -MAX_SLEW_SPEED) Velocity = -MAX_SLEW_SPEED;

  if (fabs((double)Velocity) <= MAX_TRACK_SPEED)
    return (int16) (Velocity + HA_ZERO_RATE) | TRACK_MASK;
  else 
    {
    SlewModeFlag = TRUE;
    return (int16) ((Velocity / SLEW_TO_TRACK_GAIN) + HA_ZERO_RATE) & 
    		   (~TRACK_MASK);
    }
}

/* SlewMode flag is set if slew mode is used */
short encode_dec_rate (double Velocity)
{
  /* Clip speed at maximum slew speed */
  if (Velocity >  MAX_SLEW_SPEED) Velocity =  MAX_SLEW_SPEED;
  if (Velocity < -MAX_SLEW_SPEED) Velocity = -MAX_SLEW_SPEED;

  if (fabs(Velocity) <= MAX_TRACK_SPEED)
    return (int16) (Velocity + DEC_ZERO_RATE) | TRACK_MASK;
  else 
    {
    SlewModeFlag = TRUE;
    return (int16) ((Velocity / SLEW_TO_TRACK_GAIN) + DEC_ZERO_RATE) &
    		   (~TRACK_MASK);
    }
}

/* Returns the zero-based commanded velocity */ 
double decode_ha_rate (short Rate)
{
  if (Rate & TRACK_MASK)
    {/* Tracking */
    Rate = Rate & (~TRACK_MASK); /* zero the slew and preload flags */
    return ((double)Rate) - HA_ZERO_RATE;
    }
  else
    {/* Slewing */
    Rate = Rate & (~TRACK_MASK); /* zero the slew and preload flags */
    return (((double)Rate) - HA_ZERO_RATE) * SLEW_TO_TRACK_GAIN;
    }
}

/* Returns the zero-based commanded velocity */ 
double decode_dec_rate (short Rate)
{
  if (Rate & TRACK_MASK)
    {/* Tracking */
    Rate = Rate & (~TRACK_MASK); /* zero the slew and preload flags */
    return ((double)Rate) - DEC_ZERO_RATE;
    }
  else
    {/* Slewing */
    Rate = Rate & (~TRACK_MASK); /* zero the slew and preload flags */
    return (((double)Rate) - DEC_ZERO_RATE) * SLEW_TO_TRACK_GAIN;
    }
}


/* ================================================================= 
   PLOTTING FUNCTIONS
   ================================================================= */
void show_plot (void)
{
  FILE *fp1; 
  FILE *fp2;
  char TitleBuf[100];

  if ((fp1 = fopen("xgnu", "w")) == NULL)
    {
    printf("ERROR opening xgnu\n");
    return;
    }
  if ((fp2 = fopen("pgnu", "w")) == NULL)
    {
    printf("ERROR opening pgnu\n");
    return;
    }
  sprintf(TitleBuf, "%s; %s", PlotTitle, PlotSubtitle);
  fprintf(fp1, "set size 0.9, 0.9\n");
  fprintf(fp2, "set size 1, 1\n");

  fprintf(fp1, "set xlabel '%s'\n",     PlotXlabel);
  fprintf(fp2, "set xlabel '%s'\n",     PlotXlabel);

  fprintf(fp1, "set ylabel '%s'\n",     PlotYlabel);
  fprintf(fp2, "set ylabel '%s'\n",     PlotYlabel);

  fprintf(fp1, "set title '%s' 0,2\n",  TitleBuf);
  fprintf(fp2, "set title '%s' 0,2\n",  TitleBuf);

  fprintf(fp1, "set nokey\n");
  fprintf(fp2, "set nokey\n");

  fprintf(fp1, "set time\n");
  fprintf(fp2, "set time 2\n");

  fprintf(fp2, "set terminal postscript landscape \"Times-Roman\" 20\n");
  fprintf(fp2, "set output 'plot.ps'\n");

  fprintf(fp1, "plot 'plot.dat' with errorbars\n");
  fprintf(fp2, "plot 'plot.dat' with errorbars\n");
  fprintf(fp2, "set terminal x11\n");
  /* fprintf(fp2, "!lpr 'plot.ps'\n");  Old version */
  /* fprintf(fp2, "!lpr -l 'plot.ps'\n");   For net printing */
  fprintf(fp2, "!lpr plot.ps\n");  /* For Penguin */

  fclose(fp1);
  fclose(fp2);
  save_plot();

  printf("\nThe plot data (%d points) has been saved to 'plot.dat'.\n",
  		PlotLength);
  printf("Display this data to the X11 screen or on paper using the "
  		 "gnuplot \ncommands:  load 'xgnu  OR  load 'pgnu\n");
}

void save_plot(void)
{
  FILE *fp;
  int i;

  if ((fp = fopen("plot.dat", "w")) == NULL)
    {
    printf("ERROR opening plot.dat\n");
    return;
    }
  for(i=0; i<PlotLength; i++)
    fprintf(fp, "%f %f %f\n", PlotX[i], PlotY[i], PlotErr[i]);
  fclose(fp);
}


/* Plot actual velocity vs time in seconds */
void plot_vactual_time(int Style)
{
  static struct int_parameter TicksToMeasure =
  		{INT, 2, 1, MAXTICKS, 
  		"time interval to compute velocity (ticks)"};

  long   Iplot, Start, End, TimeScale;
  double DelPosition;     /* estimated change in position in degrees */
  double MeasureTime;     /* estimated change in time in seconds */

  if (Style == TIME_AXIS) TimeScale = TICKS_PER_SECOND;
  else                    TimeScale = 1.0;

  get_parameter((parameter*) &TicksToMeasure);
  MeasureTime = TicksToMeasure.Val / TICKS_PER_SECOND;

  for ( Iplot=0, Start=0,                     End = Start + TicksToMeasure.Val;
       (Iplot<MAXPLOTS       &&               End<TestLength); 
        Iplot++, Start += TicksToMeasure.Val, End = Start + TicksToMeasure.Val)
    {
    if (CurrentAxis == HA_AXIS)
      DelPosition =(TickData[End].actual_ha  - TickData[Start].actual_ha);
    else
      DelPosition =(TickData[End].actual_dec - TickData[Start].actual_dec);

    PlotX  [Iplot] = 0.5*(Start + End) / TimeScale;
    PlotY  [Iplot] =   DelPosition / MeasureTime * 3600.;
    PlotErr[Iplot] = DEL_POS_ERROR / MeasureTime * 3600.;
    }
  PlotLength  = Iplot;
  if (Style == TIME_AXIS) 
    PlotXlabel  = "Time (seconds)";
  else
    PlotXlabel  = "Time (ticks)";

  if (CurrentAxis == HA_AXIS)
    PlotYlabel = "Measured HA Velocity (\"/sec)"; 
  else
    PlotYlabel = "Measured DEC Velocity (\"/sec)"; 
  sprintf(PlotSubtitle, "velocity calculated every %d ticks",
          TicksToMeasure.Val); 
  show_plot();
}

/* Plot actual velocity vs desired velocity */
void plot_vactual_vdesired(void)
{
  static struct int_parameter TicksToSettle =
  		{INT, 2, 1, 10*TICKS_PER_SECOND, 
  		"time interval for telescope to settle after steps (ticks)"};

  long   Iplot, Start, End;
  double DesiredV, PrevDesiredV;
  double DelPosition;   /* estimated change in position in degrees */
  double MeasureTime;	/* in seconds */
  int    Done = 0, EndOfTest = 0;

  get_parameter((parameter*) &TicksToSettle);

  /* Start and End point to changes in the desired velocity input */
  Iplot = 0;
  Start = 0;
  End   = 1;
  if (CurrentAxis == HA_AXIS)
    PrevDesiredV = TickData[Start].desired_ha_rate;
  else
    PrevDesiredV = TickData[Start].desired_dec_rate;

  while (!Done)
    {
    if (CurrentAxis == HA_AXIS)
      DesiredV = TickData[End].desired_ha_rate;
    else
      DesiredV = TickData[End].desired_dec_rate;

    if (DesiredV != PrevDesiredV || EndOfTest)
      {/* Complete the current velocity measurement, then start another */

      Start += TicksToSettle.Val;
      MeasureTime = (End-Start) / TICKS_PER_SECOND;
      if (MeasureTime > 0)
        {
        if (CurrentAxis == HA_AXIS)
          DelPosition =(TickData[End].actual_ha - TickData[Start].actual_ha);
        else
          DelPosition =(TickData[End].actual_dec - TickData[Start].actual_dec);

        PlotX  [Iplot] = PrevDesiredV;
        PlotY  [Iplot] =   DelPosition / MeasureTime * 3600.;
        PlotErr[Iplot] = DEL_POS_ERROR / MeasureTime * 3600.;
        if (++Iplot >= MAXPLOTS) Done = TRUE;
        }
      else printf("Dwell too short!\n");

      Start = End;
      PrevDesiredV = DesiredV;
      if (EndOfTest) Done = TRUE;
      } 
    else 
      {
      if (++End >= (TestLength-1)) EndOfTest = TRUE;
      }
    } /*while*/

  PlotLength = Iplot;
  if (CurrentAxis == HA_AXIS)
    {
    PlotXlabel = "Desired HA Rate (D/A units)"; 
    PlotYlabel = "Measured HA Velocity (\"/sec)"; 
    }
  else
    {
    PlotXlabel = "Desired DEC Rate (D/A units)"; 
    PlotYlabel = "Measured DEC Velocity (\"/sec)"; 
    }
  sprintf(PlotSubtitle,  "settle time = %d ticks", TicksToSettle.Val); 
  show_plot();
}

/* Plot actual position vs time */
void plot_pactual_time(void)
{
  double StartPosn;
  long   Iplot;

  if (CurrentAxis == HA_AXIS)
    StartPosn = TickData[0].actual_ha;
  else
    StartPosn = TickData[0].actual_dec;

  for( Iplot=0; Iplot < min(MAXPLOTS,TestLength); Iplot++)
    {
    PlotX  [Iplot] = Iplot;
    PlotErr[Iplot] = 3600.0 / ENCODERS_PER_DEGREE;

    if (CurrentAxis == HA_AXIS)
      PlotY[Iplot] = (TickData[Iplot].actual_ha-StartPosn)  * 3600.;
    else
      PlotY[Iplot] = (TickData[Iplot].actual_dec-StartPosn) * 3600.;
    }
  PlotLength  = Iplot;
  PlotXlabel  = "Time (ticks)";

  if (CurrentAxis == HA_AXIS)
    PlotYlabel = "HA offset (\")"; 
  else
    PlotYlabel = "DEC offset (\")"; 
  PlotSubtitle[0] = '\0';
  show_plot();
}

/* plot desired velocity vs time */
void plot_vdesired_time(void)
{
  long   Iplot;

  for( Iplot=0; Iplot < min(MAXPLOTS,TestLength); Iplot++)
    {
    PlotX  [Iplot] = Iplot;
    PlotErr[Iplot] = 0.0;
    if (CurrentAxis == HA_AXIS)
      PlotY[Iplot] = TickData[Iplot].desired_ha_rate;
    else
      PlotY[Iplot] = TickData[Iplot].desired_dec_rate;
    }
  PlotLength  = Iplot;
  PlotXlabel  = "Time (ticks)";

  if (CurrentAxis == HA_AXIS)
    PlotYlabel = "Desired HA Velocity (D/A units)"; 
  else
    PlotYlabel = "Desired DEC Velocity (hardware units)"; 
  PlotSubtitle[0] = '\0';
  show_plot();
}

/* plot desired position vs time */
void plot_pdesired_time(void)
{
  double StartPosn;
  long   Iplot;

  if (CurrentAxis == HA_AXIS)
    StartPosn = TickData[0].desired_ha;
  else
    StartPosn = TickData[0].desired_dec;

  for( Iplot=0; Iplot < min(MAXPLOTS,TestLength); Iplot++)
    {
    PlotX  [Iplot] = Iplot;
    PlotErr[Iplot] = 3600.0 / ENCODERS_PER_DEGREE;

    if (CurrentAxis == HA_AXIS)
      PlotY[Iplot] = (TickData[Iplot].desired_ha-StartPosn)  * 3600.;
    else
      PlotY[Iplot] = (TickData[Iplot].desired_dec-StartPosn) * 3600.;
    }
  PlotLength  = Iplot;
  PlotXlabel  = "Time (ticks)";

  if (CurrentAxis == HA_AXIS)
    PlotYlabel = "Desired HA offset (\")"; 
  else
    PlotYlabel = "Desired DEC offset (\")"; 
  PlotSubtitle[0] = '\0';
  show_plot();
}

/* plot a selected encoder bit */
/* The encoder puts out 14 bits which are stored in the top 14 bits of
   the ha_enc and dec_enc slots in shared memory.
   The lower 2 bits of these slots are zero. */
void plot_encoder_bit (void)
{
  static struct int_parameter BitNum =
  		{INT, 0, 0, 13,  "bit number to plot"};

           long  Iplot;
  unsigned short EncoderVal;

  get_parameter((parameter*) &BitNum);

  for( Iplot=0; Iplot < min(MAXPLOTS,TestLength); Iplot++)
    {
    PlotX  [Iplot] = Iplot;
    PlotErr[Iplot] = 0.0;
    if (CurrentAxis == HA_AXIS)
      EncoderVal = TickData[Iplot].ha_encoder;
    else
      EncoderVal = TickData[Iplot].dec_encoder;
    PlotY[Iplot] = (float) (( EncoderVal >> (BitNum.Val+2) ) & 0x0001);
    }
  PlotLength  = Iplot;
  PlotXlabel  = "Time (ticks)";
  PlotYlabel  = "Bit Value";

  if (CurrentAxis == HA_AXIS)
    sprintf(PlotSubtitle, "HA encoder bit #%d", BitNum.Val);
  else
    sprintf(PlotSubtitle, "DEC encoder bit #%d", BitNum.Val);
  show_plot();
}

/* Plot actual error vs time */
void plot_error_time(void)
{
  long   Iplot;

  for( Iplot=0; Iplot < min(MAXPLOTS,TestLength); Iplot++)
    {
    PlotX  [Iplot] = Iplot;
    PlotErr[Iplot] = 0.0;

    if (CurrentAxis == HA_AXIS)
      PlotY[Iplot] = TickData[Iplot].ha_error * 3600.;
    else
      PlotY[Iplot] = TickData[Iplot].dec_error * 3600.;
    }
  PlotLength  = Iplot;
  PlotXlabel  = "Time (ticks)";

  if (CurrentAxis == HA_AXIS)
    PlotYlabel = "HA error (\")"; 
  else
    PlotYlabel = "DEC error (\")"; 
  PlotSubtitle[0] = '\0';
  show_plot();
}


/* ================================================================= */
/* ================================================================= 
   ================================================================= */
/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- 
   ----------------------------------------------------------------- */
