/* This file contains subroutines to deal with time */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>  

/* First, copy a timeval structure */
int timecopy(struct timeval src, struct timeval *dest)
{
  dest->tv_sec = src.tv_sec;
  dest->tv_usec = src.tv_usec;
}

/* Calculate the difference in time between two time structures */
int timesub(struct timeval first, struct timeval second, struct timeval *dest)
{

  dest->tv_sec  = first.tv_sec  - second.tv_sec;
  dest->tv_usec = first.tv_usec - second.tv_usec;


	while( dest->tv_usec < 0 ) {
		dest->tv_usec += 1000000;
		dest->tv_sec -= 1;
	}
	
	while( dest->tv_usec > 1000000 ) {
		dest->tv_usec -= 1000000;
		dest->tv_sec += 1;
	}
}	

/* The double precision number of seconds in a time. */
double timesize( struct timeval a) 
{
	return ( ( (double) a.tv_sec) + 0.000001 * a.tv_usec );
}

int timeprint(struct timeval a)
{
	printf("Time: %ld  %d\n",a.tv_sec, a.tv_usec);
	fflush(stdout);
}



