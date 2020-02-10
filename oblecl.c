/**********
 * Return the Obliquity of ecliptic for the given julian day
 **********/
#include "wtrack.h"

double oblecl(double jul)
{

double T;

T = (jul - 2451545.0) / 36525.0;

return(23.43929167 - 
       0.0130041667 * T - 
       0.0000001667 * T * T + 
       0.0000005036 * T * T * T);

/* END oblecl */ }
