/* 
	Open a terminal port at the designated speed. Used for IEEE 488 control. 
	Return a filedescripter.
	Other things are also set:  See the code for comments;
	the input is very "raw", which we need for IO with the IEEE 488
*/

#define _POSIX_SOURCE
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <fcntl.h>

int openport( name, speed )
char *name;     /* The name of the port to be opened */
speed_t speed;    /* An integer designating the speed - like B2400 */
{
	struct termios term, termsave;
	int terminal;	

	usleep(1);	
	if ( -1 == (terminal=open(name,O_RDWR) ) ) {
		printf("Unable to open up the terminal %s\n",name);
		exit(1);
	}
	printf("Number we are opened to %d \n",terminal); 
	fflush(stdout);
	if (tcgetattr(terminal, &term) < 0) {
		printf("Could not get the terminal structure\n");
		exit(1);
	}

	if ( 0 != cfsetispeed(&term,(speed_t) speed)) {
		printf("Could not set the speed of port %s to the desired value %s\n",
			name,speed);	
		exit(1);
	}

	if ( 0 != cfsetospeed(&term, (speed_t) speed)) {
		printf("Could not set the speed of port %s to the desired value %s\n",
			name,speed);	
		exit(1);
	}

	term.c_iflag &= ~IGNCR;  /* CR is not deleted from input stream */
	term.c_iflag &= ~ICRNL;  /* do NOT turn CR into NULL */
	term.c_iflag &= ~INLCR;  /* do NOT convert NL to CR */
	term.c_iflag &= ~INPCK;  /* disable parity checking */
	term.c_iflag &= ~ISTRIP; /* Do NOT strip parity bits */
	term.c_iflag &= ~IGNBRK; /* Ignore break condition on line */
	term.c_iflag &= ~IXON;   /* Turn OFF XON/XOFF flow control  for input */
	term.c_iflag &= ~IXOFF;  /* Trun OFF XON/XOFF flow control for output */
	
	term.c_cflag |= CLOCAL;  /* Ignore modem control lines */
	term.c_cflag |= CREAD;	 /* Enable reading characters */
	term.c_cflag |= CSTOPB;  /* two stop bits */
	term.c_cflag |= CS8;     /* Set 8 bits per byte */
	term.c_cflag &= ~PARENB; /* Disable parity checking */
	term.c_cflag &= ~HUPCL;  /* Do not terminate on hangup */
	
	term.c_lflag &= ~ICANON; /* Non- canonical mode - "raw" */
	term.c_lflag &= ~ECHO;   /* Do not echo back to terminal */
	term.c_lflag &= ~ISIG;   /* NO special characters, like SUSP */
	term.c_lflag &= ~IEXTEN; /* NO special characters */

	term.c_cc[VMIN]=1;
	term.c_cc[VTIME]=0;	
		
	if ( 0 != tcsetattr(terminal,TCSAFLUSH,&term)){
		printf("Could not set the attributes of terminal %s \n",name);
		exit(1);
	}


/*
#include <ioctl.h>


	ioctl(terminal,TIOCGETP,&tryit);
	tryit.sg_ispeed=EXTB;
	ioctl(terminal, TIOCSETP,&tryit);
*/
	return(terminal);
}
