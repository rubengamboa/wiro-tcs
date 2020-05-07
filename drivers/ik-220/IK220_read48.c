#include <stdio.h>
#include <stdint.h>
#include "IK220Con.h"



uint32_t  IKCard[16];						// Port address of IK card(s)
uint16_t  Ax;							// Axis number

char      VersCard[20], VersDrv[20], VersDll[20];		// Version text of card, driver and DLL

double	  CntVal;							// Counter value
uint32_t  DllStatus, DllInfo;

int main (void)
{
	if (!IK220Find (IKCard))		// Look for IK 220
	{
		printf ("Error: IK220Find\n");									
		if (!IK220DllStatus (&DllStatus, &DllInfo))  
			printf ("Error: IK220DllStatus\n");		// Read DLL-Status
		printf ("DLL-Status: 0x%08lX     DLL-Info: 0x%08lX\n", DllStatus, DllInfo);
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
		
		 
			if ( !IK220WritePar (Ax, 1, 0) ) 
				printf ("IK 220 (%2d) not set!\n", Ax);

			if ( !IK220WritePar (Ax, 2, 0) ) 
				printf ("IK 220 (%2d) not set!\n", Ax);
					
	  }		
  }

	sleep(1);

  while(1)
  {
		if (IKCard[0])
			{
				if (!IK220Read48 (0, 0, &CntVal))  
					printf ("Error: IK220Read48 card %d\n", 0);	// Read counter value 
				else 
				 
				  // FIXME: Insert the implementation for correct conversion to Counting with your
				  //        Encoder Type:
				  //        CntVal = CntVal/10000/360    (degree)
				  printf ("Axis %d: %12.0f  ", 0, CntVal);
			}
		 printf ("\r");
		
  }

  printf ("\n\n");
	return 0;
}
