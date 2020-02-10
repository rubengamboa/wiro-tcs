/* julian calculates the julian date given the year, month, day, and time */
/* Time is in decimal hours */

#include "wtrack.h"

double julian( int year, int month, int day, double time )
{

int
   a,
   b;

double
   jul;

if (month <= 2) {
   year--;
   month += 12;
}

a = year / 100;
b = 2 - a + ( int ) ( a / 4 );

jul = (int) (365.25 * year) + (int) (30.6001 * ( month + 1 ) ) + day + b + 1720994.5;

return( jul +  time / 24.0 );

/* end julian */ }
