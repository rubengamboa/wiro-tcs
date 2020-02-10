/**********
 * Return the nutated RA and Dec corrections for the given julian date
 * RA should be given in the form HH.hhhhhhhh whereas Dec
 * should be in the form DD.dddddddddd
 **********/
#include "wtrack.h"
#include <math.h>

void nutation(double jul, double *ra,double *dec)
{

double radra, raddec, ecl, lon, obl;

lonobl(jul, &lon, &obl);
ecl = RAD(oblecl(jul));

/* Convert ra to degrees then to radians */

radra = RAD(*ra * 15.0);
raddec = RAD(*dec);

*ra = ((cos(ecl) + sin(ecl) * sin(radra) * tan(raddec)) * lon
      - cos(radra) * tan(raddec) * obl) / 15.0;

*dec = sin(ecl) * cos(radra) * lon + sin(radra) * obl;

/* END nutation */ }

