#include <stdio.h>
#include <stdint.h>
#include "IK220Con.h"



uint32_t	IKCard[16];									// Port address of IK card(s)
uint16_t	Ax;											// Axis number
uint16_t  EnStatus, EnAlarm, EnType, EnTurns, EnRefDist, EnCntDir;
uint32_t  EnPeriod, EnStep;

char      m_Active[16];
uint32_t  OldSta[16];
double    m_SignalPeriod[16];
uint16_t  m_SignalType[16];
uint16_t  m_EncoderType[16];

char	VersCard[20], VersDrv[20], VersDll[20];		// Version text of card, driver and DLL


double	  CntVal;										// Counter value
uint32_t	DllStatus, DllInfo;

int main (void)
{
	if (!IK220Find (IKCard))		// Look for IK 220
	{
		printf ("Error: IK220Find\n");									
		if (!IK220DllStatus (&DllStatus, &DllInfo))  
			printf ("Error: IK220DllStatus\n");		// Read DLL-Status
		printf ("DLL-Status: 0x%08lX     DLL-Info: 0x%08lX", DllStatus, DllInfo);
		return;
	}

	for (Ax=0; Ax<6; Ax++)
		if (IKCard[Ax])
		{
			if (!IK220Init (Ax))                              // Initialize IK 220
				printf ("Error: IK220Init axis %d\n", Ax);
			else 
			{
				fprintf (stderr, "Axis %d initialized  -  ", Ax);

				// Read port address of IK card(s)
				if (!IK220Version (Ax, &VersCard[0], &VersDrv[0], &VersDll[0])) 
					printf ("Error: IKVersion\n");		// Read port address of IK card(s)
				else 
					printf ("Card: %s  %s  %s\n", VersCard, VersDrv, VersDll);   
			}
		}

	for (Ax=0; Ax<6; Ax++)
	{	
		if (IKCard[Ax])
		{
			printf ("IK 220 (%2d) at address: 0x%08lX\n", Ax, IKCard[Ax]);	
	    printf ("\n");
		
		  m_Active[Ax]            = 0;
		  OldSta[Ax]              = 0xFFFF;
		  m_SignalPeriod[Ax]      = 0.020;
		  m_EncoderType[Ax]       = 1;
		  m_SignalType[Ax]        = 1;
			 
			if ( !IK220WritePar (Ax, 1, m_EncoderType[Ax]) ) 
				printf ("IK 220 (%2d) not set!\n", Ax);

			if ( !IK220WritePar (Ax, 2, m_SignalType[Ax]) ) 
				printf ("IK 220 (%2d) not set!\n", Ax);

			if (!IK220ResetEn  (Ax, &EnStatus)) 
				printf ("IK 220 (%2d) not reset!\n", Ax);

			if (!IK220ConfigEn (Ax, &EnStatus, &EnType, &EnPeriod, &EnStep, 
						              &EnTurns, &EnRefDist, &EnCntDir)) 
				printf ("IK 220 (%2d) not reset!\n", Ax);
			
			printf ("Axis %d: Status:%d Steps:%d Period:%d EnTurns:%d   \n",
											Ax, EnStatus, EnStep, EnPeriod, EnTurns);
//			if (!IK220StartRef  (Ax)) printf ("IK 220 (%2d) not started!\n", Ax);
//			if (!IK220ResetRef  (Ax)) printf ("IK 220 (%2d) not started!\n", Ax);
	  }		
  }

	sleep(3);

  while(1)
  {
		for (Ax=0; Ax<6; Ax++)
			if (IKCard[Ax])
			{
				if (!IK220ReadEn (Ax, &EnStatus, &CntVal, &EnAlarm))  
					printf ("Error: IK220Read48 card %d\n", Ax);	// Read counter value 
				else 
					printf ("Axis %d: %12.4f  Status:%d   \n", Ax, CntVal, EnStatus);
			}
			printf ("\n");
      sleep(1);
  }

  printf ("\n\n");
	return 0;
}
