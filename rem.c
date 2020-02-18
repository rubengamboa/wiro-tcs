/************************************************************************/
/* Return the fractional portion of the given argument                  */
/**************************************************************************/

#include <math.h>

double rem(double val)
{
	return(val -  floor(val));
}
