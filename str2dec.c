/********************************************************************/

#include <stdio.h>
#include <ctype.h>

double str2dec( char str[] )
{
double  num;
char    tmp[ 80 ];
int     i, neg=1;
double ntmp;
double pow();

	i=strlen(str);
	while( isspace( (int) str[i])  && i>0) str[i--] = NULL;
			
	tmp[ 2 ] = '\0';

	if ( str[ 0 ] == '+' ) {
   		neg = 1;
   		str++;
	}

	else if ( str[ 0 ] == '-' ) {
   		neg = -1;
   		str++;
	}

	else
   		neg = 1;

	i=0; ntmp=0;
	while ( (*str != NULL) && (':' != *str ) )  tmp[i++] = *(str++); 
	tmp[i]=NULL;
	sscanf( tmp, "%lf", &ntmp );
	num = ( double ) ntmp;
	str++;

	i=0; ntmp=0;	
	while ( (*str != NULL) && (':' != *str ) )  tmp[i++] = *(str++); 
	tmp[i]=NULL;
	sscanf( tmp, "%lf\n", &ntmp );
	num += ( double ) ntmp / 60.0;
	str++;

	i=0; ntmp = 0;	
	while ( (*str != NULL) && ('.' != *str ) ) tmp[i++] = *(str++); 
	tmp[i]=NULL;
	sscanf( tmp, "%lf\n", &ntmp );
	num += ( double ) ntmp / 3600.0;
	str++;

	i=0; ntmp = 0;	
	while  (*str != NULL)  tmp[i++] = *(str++); 
	tmp[i]=NULL;
	sscanf( tmp, "%lf\n", &ntmp );

	num += ( double ) ntmp / (3600. * pow((double) 10,(double) i)); 

	num *= neg;
	return( num );
}
