/* follows objects */
/* follow.c               July 24, 1991          Robert R. Howell */

/* JSW 20Jan00  Port to Linux */

#include "wirotypes.h" 
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "track.h"
#include "wiro.h"
#include "wtrack.h"

void log_entry( char *comment );

double
   julian( ),
   oblecl( ),
   rem( ),
   hms2dec( ),
   dec2hms( ),
   str2dec( );

#define RAD( x ) ( ( x ) * PI / 180.0 )
#define DEG( x ) ( ( x ) / PI * 180.0 )


/* Prototype internal functions */
double str2dec( char str[] );
double julian( int year, int month, int day, double time );
void lonobl(double jul, double *lon, double *obl);
double oblecl(double jul);
void nutation(double jul, double *ra, double *dec);
void precession( double js, double je, double pmra, double pmdec,
                 double *ra, double *dec );
void pmatrix( double js, double je, double matrix[][3] );
double rem(double val);
double hms2dec(double hms);
double dec2hms(double dec);
void annabr(double jul, double *deltara, double *deltadec);


#pragma check_pointer (off)        /* Turn this off, so I can access */
		      	      	   /* the tracker memory without     */
				   /* protection errors.             */


int main( int argc, char *argv[] )
{

  double  js, je, pmra, pmdec, dra, ddec, ra, dec;

  char    lstr[ 100 ], num[ 10 ], sra[ 15 ],sdec[ 15 ],
    sname[ 80 ]="                                           ",
    name[ 81 ]="                                        ";

  int     s,i;

  double  epoch;
  FILE    *catalog;
  struct  wiro_memory *tinfo,junk;
  char    *ignore, temp[ 80 ], cat_name[ 80 ], s1[ 98 ];

  tinfo = get_tinfo();

  if (argc == 1) {
    printf("Enter an object name\n");
    scanf("%s",name);
  }

  else if (argc == 2)  strcpy(name,argv[1]);
  else if (argc == 3) {
    strcpy(name,argv[1]);
    strcat(name," ");
    strcat(name,argv[2]);
  }
  else if (argc==4)  {
    strcpy(name,argv[1]);
    strcat(name," ");
    strcat(name,argv[2]);
    strcat(name," ");
    strcat(name,argv[3]);
  }
  else if (argc==4)  {
    strcpy(name,argv[1]);
    strcat(name," ");
    strcat(name,argv[2]);
    strcat(name," ");
    strcat(name,argv[3]);
    strcat(name," ");
    strcat(name,argv[4]);
  }
  else {
    printf("That name is too complicated - please user quotes!\n");
    exit(-1);
  }
  strcpy(temp, CATALOGS);
  strcat(temp, "current.cat");
  catalog = fopen( temp, "r" );
  if ( catalog == NULL ) {
    printf( "No catalog has been selected.  Use \"catalog\" to do this.");
    exit( -2 );
  }
  fscanf( catalog, "%s", cat_name );
  fclose( catalog );

  strcpy( temp, CATALOGS ); 
  strcat( temp, cat_name );
  catalog = fopen( temp, "r" );
  if ( catalog == NULL ) {
    printf( "Unable to open catalog:  %s\n", temp );
    exit( -1 );
  }

  do {
    fgets( lstr, 99, catalog );
    if ( lstr[ 0 ] == '*' ) 
      continue;
    ignore = strchr( lstr, '|' );
    if ( ignore != NULL )
      *ignore = '\0';
    s = sscanf( lstr, "%s '%[^']' %s %s %lf %lf %lf", 
		num, sname, sra, sdec, &pmra, &pmdec, &epoch );
    if ( s == 6 )
      epoch = 1950.0;

    if ( strcmp( num, name ) == 0 || strcmp( name, sname ) == 0 )
      break;
    strcpy( num, "" );
  } while ( !feof( catalog ) );
  fclose( catalog );

  if ( strcmp( num, "" ) == 0 ) {
    printf( "Unable to locate object: %s\n", name );
    exit( -1 );
  }

  pmra /= 10000.0;
  pmdec /= 1000.0;

  printf( "#%s\nName: %s\nEpoch: %lf\nRA: %s\nDec: %s\nPMRA: %lf\nPMDec: %lf\n", 
	  num, sname, epoch, sra, sdec, pmra, pmdec );


  ra = str2dec( sra );
  dec = str2dec( sdec );

  /* tinfo = tinfo_ptr();         Get address of tracking info memory */

  ORIG_RA = ra;
  ORIG_DEC = dec;
  EPOCH = epoch;

  strcpy( tinfo->object_name, sname );

  je = julian( YEAR, MONTH, DAY, UT_TIME );
  js = julian( ( short ) epoch, 1, 1, 0.0 );

  precession( js, je, pmra, pmdec, &ra, &dec );

  dra = ra;
  ddec = dec;
  nutation( je, &dra, &ddec );
  ABER_NUT_RA = 15. * dra;
  ABER_NUT_DEC = ddec;

  dra = ra;
  ddec = dec;
  annabr( je, &dra, &ddec );
  ABER_NUT_RA += 15. * dra;
  ABER_NUT_DEC += ddec;

  DES_RA = ra;
  DES_DEC = dec;

  V_RA = 0.0;
  V_DEC = 0.0;

  tinfo->motion_type = NON_SOLAR;

  HP=0.0;
  strcpy( s1, "follow " );
  strcat( s1, sname );
  log_entry( s1 );

  /* end main */ }



/***********************************************************************/
double julian( int year, int month, int day, double time )
{

int a,
    b;

double  jul;

if (month <= 2) {
   year--;
   month += 12;
}

a = year / 100;
b = 2 - a + ( int ) ( a / 4 );

jul = (int) (365.25 * year) + (int) (30.6001 * ( month + 1 ) ) + day + b + 1720994.5;

return( jul +  time / 24.0 );
}


/********************************************************************
 * Return the nutation in longitude and the nutation in
 * obliquity for the given julian date
 **********/

void lonobl(double jul, double *lon, double *obl)
{

double T, T2, T3, omega, l, lp, F, D, tomega, tl, tlp, tF, tD, r = 1296000.0;

T = (jul - 2451545.0) / 36525.0;
T2 = T * T;
T3 = T2 * T;

/******
omega = 450160.280 - (5 * r + 482890.539) * T + 7.455 * T2 + 0.008 * T3;
l = 485866.733 + (1325 * r + 715922.633) * T + 31.310 * T2 + 0.064 * T3;
lp = 1287099.804 + (99 * r + 1292581.224) * T - 0.577 * T2 - 0.012 * T3;
F = 335778.877 + (1342 * r + 295263.137) * T - 13.257 * T2 + 0.011 * T3;
D = 1072261.307 + (1236 * r + 1105601.328) * T - 6.891 * T2 + 0.019 * T3;
******/

omega = 450160.280 - 6962890.539 * T + 7.455 * T2 + 0.008 * T3;
l = 485866.733 + (1717915922.633) * T + 31.310 * T2 + 0.064 * T3;
lp = 1287099.804 + (129596581.224) * T - 0.577 * T2 - 0.012 * T3;
F = 335778.877 + (1739527263.137) * T - 13.257 * T2 + 0.011 * T3;
D = 1072261.307 + (1602961601.328) * T - 6.891 * T2 + 0.019 * T3;

omega = RAD(omega / 3600.0);
l = RAD(l / 3600.0);
lp = RAD(lp / 3600.0);
F = RAD(F / 3600.0);
D = RAD(D / 3600.0);

tomega = 2.0 * omega;
tl = 2.0 * l;
tlp = 2.0 * lp;
tF = 2.0 * F;
tD = 2.0 * D;

*lon = - (17.1996 + 0.01742 * T) * sin(omega)
       + 0.2062 * sin(tomega)
       + 0.0046 * sin(-tl + tF + omega)
       + 0.0011 * sin(tl - tF)
       - 1.3187 * sin(tF - tD + tomega)
       + 0.1426 * sin(lp)
       - 0.0517 * sin(lp + tF - tD + tomega)
       + 0.0217 * sin(-lp + tF - tD + tomega)
       + 0.0129 * sin(tF - tD + omega)
       + 0.0048 * sin(tl - tD)
       - 0.0022 * sin(tF - tD)
       + 0.0017 * sin(tlp)
       - 0.0015 * sin(lp + omega)
       - 0.0016 * sin(tlp + tF - tD + tomega)
       - 0.0012 * sin(-lp + omega)
       - 0.2274 * sin(tF + tomega)
       + 0.0712 * sin(l)
       - 0.0386 * sin(tF + omega)
       - 0.0301 * sin(l + tF + tomega)
       - 0.0158 * sin(l - tD)
       + 0.0123 * sin(-l + tF + tomega)
       + 0.0063 * sin(tD)
       + 0.0063 * sin(l + omega)
       - 0.0058 * sin(-l + omega)
       - 0.0059 * sin(-l + tF + tD + tomega)
       - 0.0051 * sin(l + tF + omega)
       - 0.0038 * sin(tF + tD + tomega)
       + 0.0029 * sin(tl)
       + 0.0029 * sin(l + tF - tD + tomega)
       - 0.0031 * sin(tl + tF + tomega)
       + 0.0026 * sin(tF)
       + 0.0021 * sin(-l + tF + omega)
       + 0.0016 * sin(-l + tD + omega)
       - 0.0013 * sin(l - tD + omega)
       - 0.0010 * sin(-l + tF + tD + omega);
       
*obl =   9.2025 * cos(omega)
       - 0.0895 * cos(tomega)
       - 0.0024 * cos(-tl + tF + omega)
       + 0.5736 * cos(tF - tD + tomega)
       + 0.0054 * cos(lp)
       + 0.0224 * cos(lp + tF - tD + tomega)
       - 0.0095 * cos(-lp + tF - tD + tomega)
       - 0.0070 * cos(tF - tD + omega)
       + 0.0977 * cos(tF + tomega)
       + 0.0200 * cos(tF + omega)
       + 0.0129 * cos(l + tF + tomega)
       - 0.0053 * cos(-l + tF + tomega)
       - 0.0033 * cos(l + omega)
       + 0.0032 * cos(-l + omega)
       + 0.0026 * cos(-l + tF + tD + tomega)
       + 0.0027 * cos(l + tF + omega)
       + 0.0016 * cos(tF + tD + tomega)
       - 0.0012 * cos(l + tF - tD + tomega)
       + 0.0013 * cos(tl + tF + tomega)
       -0.0010 * cos(-l + tF + omega);

*lon /= 3600.0;
*obl /= 3600.0;
}

/**********************************************************************
 * Return the Obliquity of ecliptic for the given julian day
 **********/

double oblecl(double jul)
{

double T;

T = (jul - 2451545.0) / 36525.0;

return(23.43929167 - 
       0.0130041667 * T - 
       0.0000001667 * T * T + 
       0.0000005036 * T * T * T);
}

/**********************************************************************
 * Return the nutated RA and Dec corrections for the given julian date
 * RA should be given in the form HH.hhhhhhhh whereas Dec
 * should be in the form DD.dddddddddd
 **********/

void nutation(double jul, double *ra, double *dec)
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
}

/*************************************************************************/
/*  This is a general precession program.
    It uses the matrix formulation given in the "Astronomical Almanac"
    page B18.
    The precession matrix is computed in the module "pmatrix.obj".
*/

void precession( double js, double je, double pmra, double pmdec,
                 double *ra, double *dec )
{
double matrix[3][3], cosin[3], cosout[3], years;
int i,j;

years = (je-js)/365.25;
*dec += pmdec*years/3600.;
*ra += pmra*years/3600.;

*ra = RAD( *ra * 15.0 );
*dec = RAD( *dec );

pmatrix( js, je, matrix );

cosin[2] = sin( *dec);
cosin[1] = cos( *dec)*sin( *ra);
cosin[0] = cos( *dec)*cos( *ra);

for (i=0; i<3; ++i)
	{
    cosout[i] = 0.;
	for (j=0; j<3; ++j)
		{
		cosout[i] += cosin[j]*matrix[j][i];
		}
	}


*dec = DEG( asin(cosout[2]) );
*ra = DEG( atan2(cosout[1],cosout[0]) ) / 15.0;

if ( *ra < 0.) *ra += 24.0;
}

/*********************************************************************/
void pmatrix( double js, double je, double matrix[][3] )
{

struct pecans { double zeta, zee, theta; } pc;
struct trig { double zeta, zee, theta; } s, c;
double t0, t1;

t0 = RAD( ( js - 2451545.5 )/ 36525.0 );
t1 = RAD( ( je - 2451545.5 )/ 36525.0 );

pc.zeta  = t1*(0.6406161 + t1*(0.0000839 + t1*0.0000050));
pc.zee   = t1*(0.6406161 + t1*(0.0003041 + t1*0.0000051));
pc.theta = t1*(0.5567530 - t1*(0.0001185 + t1*0.0000116));

pc.zeta -= t0*(0.6406161 + t0*(0.0000839 + t0*0.0000050));
pc.zee  -= t0*(0.6406161 + t0*(0.0003041 + t0*0.0000051));
pc.theta-= t0*(0.5567530 - t0*(0.0001185 + t0*0.0000116));

s.zeta = sin(pc.zeta);
s.zee  = sin(pc.zee);
s.theta= sin(pc.theta);

c.zeta = cos(pc.zeta);
c.zee  = cos(pc.zee);
c.theta= cos(pc.theta);

matrix[0][0] = c.zeta*c.theta*c.zee - s.zeta*s.zee;
matrix[1][0] = -s.zeta*c.theta*c.zee - c.zeta*s.zee;
matrix[2][0] = -s.theta*c.zee;
matrix[0][1] = c.zeta*c.theta*s.zee + s.zeta*c.zee;
matrix[1][1] = -s.zeta*c.theta*s.zee + c.zeta*c.zee;
matrix[2][1] = -s.theta*s.zee;
matrix[0][2] = c.zeta*s.theta;
matrix[1][2] = -s.zeta*s.theta;
matrix[2][2] = c.theta;
}



/**********************************************************************
 * Return the decimal equivalent of the given argument which is in the
 * form HH.MMSSssss or DD.MMSSssss
 **********/

double hms2dec(double hms)
{

double h, m;

h = (int) hms;
hms = (hms - h) * 100.0;

m = (int) hms;
hms = (hms - m) * 100.0 + m * 60.0;

return(h + hms / 3600.0);
}

/********************************************************************
 * Returns the time or angle in the form HH.MMSSssss or DD.MMSSssss
 * from the given argument in the form DD.dddddddd or HH.hhhhhhh
 **********/

double dec2hms(double dec)
{

double h, m, s, ms;

h = (int) dec;
dec = ( dec - h ) * 60.0;

m = (int) dec;
dec = ( dec - m ) * 60.0;
s = ( int ) dec;

ms = ( dec - s ) * 1000.0;
return(h + m / 100.0 + s / 10000.0 + ms / 10000000.0 );
}


/******************************************************************
 * Return the proper corrections for annual
 * abberration for the given julian date.
 *
 * Reference: Microcomputer Control of Telescopes
 *            Trueblood & Genet
 *            Copyright 1985
 *
 * Pages: 87-88
 **********/

void annabr(double jul, double *deltara, double *deltadec)
{

double radra, raddec, C, c, cp, D, d, dp, k, L, Lm,
       obl, diff, newE, T, T2, T3, lp, v, e, E;

radra = RAD(*deltara * 15.0);
raddec = RAD(*deltadec);

/***** Number of Julian centuries *****/

T = (jul - 2451545.0) / 36525.0;
T2 = T * T;
T3 = T2 * T;

/***** Mean anomaly of the Sun *****/

lp = RAD(357.5277233 + 35999.05034 * T - 0.0001602778 * T2 - 0.0000033333 * T3);

/***** Eccentricity of the Earth's orbit *****/

e = 0.016708320 - 0.000042229 * T - 0.000000126 * T2;

/***** Eccentric annomaly of the Earth in its orbit *****/

E = lp;
diff = 1.0;
while (fabs(diff) > 0.0000000048) {
   newE = lp + e * sin(E);
   diff = newE - E;
   E = newE;
}

/***** True anomaly of the Earth in its orbit *****/

v = 2.0 * atan(sqrt((1 + e) / (1 - e)) * tan(E / 2.0));

/***** Geometric mean longitude of the Sun *****/

Lm = RAD(280.4660694 + 36000.7697972222 * T + 0.0003025 * T2);

/***** Sun's true longitude *****/

L = Lm + v - lp;

/***** Obliquity of the ecliptic *****/

obl = RAD(oblecl(jul));

dp = cos(radra) * sin(raddec);

cp = tan(obl) * cos(raddec) - sin(radra) * sin(raddec);

d = 0.06666666667 * sin(radra) / cos(raddec);

c = 0.06666666667 * cos(radra) / cos(raddec);

/***** Constant of aberration *****/

k = RAD(0.0056932);

D = -k * sin(L);

C = -k * cos(obl) * cos(L);

*deltara = DEG(C * c + D * d);

*deltadec = DEG(C * cp + D * dp);
}

