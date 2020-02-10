/*****************************************
* pcat.c d.barnaby 22jan94
* modified, simplified version of pcat.c by ESp
****************************************/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wirotypes.h" 
#include "track.h"
#include "wiro.h"

double
   str2dec( );

#define RAD( x ) ( ( x ) * PI / 180.0 )
#define DEG( x ) ( ( x ) / PI * 180.0 )


/* Prototype internal functions */
double str2dec( char str[] );

#pragma check_pointer (off)        /* Turn this off, so I can access */
		      	      	   /* the tracker memory without     */
				   /* protection errors.             */


int main( int argc, char *argv[] )
{

  double  js,
    je,
    pmra,
    pmdec,
    dra,
    ddec,
    ra,
    dec;

  char    lstr[ 100 ],
    num[ 10 ],
    sra[ 15 ],
    sdec[ 15 ],
    sname[ 80 ]="                                           ",
    name[ 81 ]="                                        ",
    filename[200];
  

  int     s,
    i;

  double  epoch;

  float   xplace, yplace, h, d;

  FILE    *catalog;

  struct  wiro_memory *tinfo,junk;
   
  char    *ignore,
    temp[ 80 ],
    cat_name[ 80 ];

  double xoffset, xscale, yoffset, yscale;

  xoffset=66.;
  xscale=72.*7./24;
  yoffset=110.;
  yscale=72.*9./130.;

  tinfo = get_tinfo();

  strcpy(filename, CATALOGS);
  strcat(filename, "current.cat");
  catalog = fopen( filename, "r" );
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

  printf("/Times-Italic findfont 30 scalefont setfont \n");
  printf("20 20 moveto (WIRO) show \n");
  printf("/Times-Roman findfont 10 scalefont setfont \n");
  printf("0.25 setlinewidth\n");

  for (h=0; h<25; h+=2) 
    {
      printf("%lf %lf moveto %lf %lf rlineto stroke\n",
	     (double) xoffset + h*xscale,
	     (double) yoffset, (double) 0, (double) 120 * yscale);
      printf("%lf %lf moveto (%d h) show\n",
	     (double) (xoffset + h * xscale - 3), 
	     (double) yoffset-10, ( 24 - (int) h ) );
    }

  for (h=0; h<=120; h+=10) 
    {
      printf("%lf %lf moveto %lf %lf rlineto stroke\n",
	     (double) xoffset, 
	     (double) yoffset+h*yscale, 
	     (double) 24*xscale, (double) 0);
      printf("%lf %lf moveto (%d d) show\n",
	     (double) xoffset  - 25, 
	     (double) yoffset+h*yscale, (int) h-30);
    }

  printf("/Times-Roman findfont 6 scalefont setfont \n");
  printf("0.4 setlinewidth\n");

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

    pmra /= 10000.0;
    pmdec /= 1000.0;

    ra = str2dec( sra );
    dec = str2dec( sdec );

    if (dec>-30.) {
      xplace = xoffset +  (24 - ra) *  xscale ;
      yplace = yoffset + (30+dec) * yscale ;

      printf("%f %f moveto \n",xplace+2,yplace+2);
      printf("%f %f lineto stroke\n",xplace-2,yplace-2);
      printf("%f %f moveto \n",xplace-2,yplace+2);
      printf("%f %f lineto stroke\n",xplace+2,yplace-2);

      /* implement all of these for stars
	 printf("%f %f moveto \n",xplace-2,yplace);
	 printf("%f %f lineto stroke\n",xplace+2,yplace);
	 printf("%f %f moveto \n",xplace,yplace+2);
	 printf("%f %f lineto stroke\n",xplace,yplace-2);
      */

      printf("%f %f moveto (%s) show\n",xplace+3,yplace-1,sname);
    }

  } while ( !feof( catalog ) );

  printf("/Times-Roman findfont 20 scalefont setfont \n");
  printf(" 150 20 moveto (%s) show\n",cat_name);  
  printf("showpage \n");

  fclose( catalog );
  /* end main */ }   
/********************************************************************/ 
