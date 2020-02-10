/* GPIBPorts.c                                                */
/* Containts the basic GPIB handeling routines                */


#include <stdio.h>
#include <unistd.h>
#include <termios.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include "GPIBports.h"

#define TERMINATOR '\n'
int fdGPIB=0;  /* This is the number of the serial port for GPIB */

/*************************************************************************/
/*  easywrite(int fd, str) just write the string to the associated file  */
/*  descripter.  It really just counts up the number of bytes in str,    */
/*  making it easier to use.											 */
/*************************************************************************/


int easywrite(int fd,  char * str)
{
	write(fd, str, strlen(str));
/*	printf("%s",str);  */
}


/*************************************************************************/
/* readstring() reads until the terminator is detected from a stream     */
/* If more than number characters are read in, the string is truncated   */
/*************************************************************************/
int readstring(int fd, lpchar str, int number)
{   int i=0;	
	
	while(number > 0) {
		read(fd,str+i,1);  /* Read the bloody character */
		if (str[i] == TERMINATOR) break;
		str[i+1]=NULL;
		i++;			   /* I could put this elsewhere - str+i++ !? */
		number--;
	}
	str[i-1] = NULL;   		/* Terminate the string for good luck */
	return(0);				/* Always succeed - for NOW! */
}

/*************************************************************************/
/* multiread() reads the specified number of bytes from a very raw       */
/* input stream. 														 */
/*************************************************************************/
int multiread(int fd, char * str, int number)
{   int i=0;	
	number += 2;
	while(number > 0) {
		read(fd,str+i,1);  /* Read the bloody character */
		i++;			   /* I could put this elsewhere - str+i++ !? */
		number--;
	}
	str[i] = NULL;   		/* Terminate the string for good luck */
	return(0);				/* Always succeed - for NOW! */
}
	
/*************************************************************************/
/*  gpibport_setup   - initializes the interface  in a device dependent way  */
/*  In this particular implementation, it opens the serial port and loads*/
/*  the variable gpibfile with the appropriate file number.			     */
/*************************************************************************/

void gpibport_setup( void ) {

	struct termios term;
	char strID[100]="          ";
	char strIDdesired[100]="Micro488A Revision 1.2 Copyright (C) 1988 IOtech Inc.";

	fdGPIB = openport(IEEESERIAL, B57600);
	/*	setspeed(fdGPIB); */

	printf("Now Initializing gpib serial interface \n");          
	fflush(stdout);
	
/* sleep ( 1 );	*/
	sleep ( 2 );	
	easywrite(fdGPIB,"@@");  /* Write abort command */
/* sleep( 2 ); */ /* A second to allow the system to reset */
	sleep( 4 );  /* A second to allow the system to reset */
	easywrite(fdGPIB,"RESET\n");    /* Send out reset string */
/* usleep((time_t) 100000); */      /* Another second to allow it to work */
	usleep((time_t) 200000);       /* Another second to allow it to work */
	easywrite(fdGPIB,"ABORT\n");    /* Send out abort string */
/* usleep((time_t) 1000000); */      /* Another second to allow it to work */
	usleep((time_t) 2000000);       /* Another second to allow it to work */
	easywrite(fdGPIB,"@@");  /* Write abort command */
/* sleep( 2 ); */ /* A second to allow the system to reset */
	sleep( 4 );  /* A second to allow the system to reset */
	printf("Checking ID string from MICRO 488A:\n"); 
	fflush(stdout);
	easywrite(fdGPIB,"HELLO\n");   /* Get the ID string to see if it's OK */
/*	usleep( (time_t) 100000); */
	
	readstring(fdGPIB,(lpchar) strID,strlen(strIDdesired)+2); 
			/* Read ID string with CRLF pair */
	printf("The ID string from the MICRO 488A \n%s\n",strID);
	if ( 0 != strncmp(strID,strIDdesired,20)) {
		printf("%s was expected!\n",strIDdesired);	
		printf("A reload is suggested. Is the MICRO488A turned on?\n");
		exit(1);
	}

	printf("Clearing D80's...");

	easywrite(fdGPIB,"CL 16,17,04,05,08,09\n");  /* Send clear to all
													devices */
	
	fflush(stdout);
}

/*************************************************************************/
/* gpib_clear  performs a gpib clear of the pointed to port              */

void gpib_clear(int port) {
	char output[100]; /* space for an outgoing command */	

	sprintf(output,"CL %02d\n",port);	/* Send clear command to port */
	usleep( (time_t) 200000) ;	
	easywrite(fdGPIB,output);
	usleep( (time_t) 200000 );			/*Pause for 100ms */
}


/*************************************************************************/
/* Write a string to port */

void gpib_wrs(int gpadr, char * str ) {
	char output[100];

	sprintf(output,"OU%02d;%s\n",gpadr,str);
	easywrite(fdGPIB,output);
}
 
/*************************************************************************/
/* Read      terminated string from current channel                      */
/* Subtract off the GPIB terminator characters.                          */
/***********************************************************************/

void gpib_rds(int gpadr, char* msg, int maxlength)
{
    int iread;
    char str[50];
	
	sprintf(str,"EN %02d\n",gpadr);
	easywrite(fdGPIB,str);
   	readstring(fdGPIB,(lpchar) msg,maxlength); 
}

/*************************************************************************/
/* Read data words into structure in F0 Hex mode                         */
/*************************************************************************/
int gpib_rd(int gpadr, lpchar bits, int n)
{   int i,tmp;
	char str[100];

	sprintf(str,"EN %02d #%d\n", gpadr ,2*n);
	easywrite(fdGPIB, str);

 	readstring(fdGPIB,  (lpchar) str, 80);
 	for (i = 0; i<n; i++) {
		sscanf(str+i*2,"%2X",&tmp);
		*(bits+i) = (unsigned char) tmp;
	}
}
	

/*************************************************************************/
/* Read data words into structure  - in high speed mode */
/*************************************************************************/
int gpib_rd_old(int gpadr, lpchar bits, int n) 
{
	char str[100];
		
	sprintf(str,"EN %02d #%d\n", gpadr ,n);
	easywrite(fdGPIB, str);
	usleep((unsigned) 20000);
	multiread(fdGPIB, (char *) bits, n);
	return(0);
}


/*************************************************************************/
/*  Write data string to output port in Hex mode - assume hardware       */
/* is in F0X mode.														 */
/*************************************************************************/
void gpib_wr(int gpadr, lpchar bits, int n) 
{
	int i;
	char str[100];

	sprintf(str,"OU %02d;D",gpadr);
	easywrite(fdGPIB,str);

	for(i=0; i<n; i++) {
		sprintf(str,"%02X",(int) bits[i]);
		str[2]=NULL;
/* 		printf(">>%s<<",str);	 */
		easywrite(fdGPIB,str);
	}

	sprintf(str,"ZX\n",gpadr);
	easywrite(fdGPIB,str);
}


/*************************************************************************/
/* Write data string - int of them in high speed mode */
void gpib_wr_old(int gpadr, lpchar bits, int n) 
{
	char partial[90], str[90];	
	int i=0;
		

/*	sprintf(str,"OUTPUT %d;ASDFE\n",gpadr); */

	sprintf(str,"SEND MTA UNL LISTEN %d DATA ",gpadr);
	easywrite(fdGPIB,str);
	for(i = 0; i<n-1; i++) {
		sprintf(str,"%d,",(int) bits[i]);
		easywrite(fdGPIB,str);
	}
	sprintf(str,"%d\n",(int) bits[n-1]);
	/* write(fdGPIB,str,n); */
	easywrite(fdGPIB,str); 
}
