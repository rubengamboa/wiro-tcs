/* wiro_catalog
10/31/91 moved to LynxOS

This code maintains the ascii text catalog of objects

12/29/90 esp adds a check to see that the new catalog really 
exists.
*/

#include "wirotypes.h"
#include "wiro.h" 

#include <sys/file.h>
#include <stdio.h>
#include <unistd.h>

void log_entry( char *comment );


char   cat_array[ 500 ][ 100 ], name[ 80 ];

int num, cat_index[ 500 ];

FILE *cat;

main( int argc, char *argv[] )

{
  char cat_name[ 100 ], s1[ 100 ], s2[100], catfile[100];

  strcpy(catfile, CATALOGS);
  strcat(catfile, "current.cat"); 

  if ( argc == 2 && strcmp( "catalog", argv[ 0 ] ) == 0 ) {
    cat = fopen( catfile, "w" );
    if ( cat == NULL ) {
      printf( "Unable to open %s\n",catfile );
      exit( -1 );
    }

    if ( -1 == access( catfile, F_OK ) ) {
      printf("-----------------------------------------------------\n");
      printf("Warning: Cannot open Catalog File >>%s<< \n",catfile); 
      printf("-----------------------------------------------------\n");
      exit(1);	
    }
    strcpy( cat_name, CATALOGS);
    strcpy( cat_name, argv[ 1 ]);

    fprintf( cat, "%s\n", argv[ 1 ] );
    fclose( cat );
    printf("The current catalog is %s\n",cat_name);
    strcpy( s1, argv[ 0 ] );
    strcat( s1, " " );
    strcat( s1, argv[ 1 ] );
    log_entry( s1 );
  }
  else if ( argc == 1 && strcmp( "catalog", argv[ 0 ] ) == 0 ) {
    cat = fopen( catfile, "r" );
    if ( cat == NULL ) {
      printf( "Unable to open %s\n",catfile );
      exit( -1 );
    }
    fscanf( cat, "%s", name );
    printf( "\nCurrent catalog is %s\n\n", name );
    fclose( cat );
    exit(1);
  }
  else if ( argc == 1 && strcmp( "newobject", argv[ 0 ] ) == 0 ) {
    cat = fopen( catfile, "r" );
    if ( cat == NULL ) {
      printf( "Unable to open %s\n",catfile );
      exit( -1 );
    }
    fscanf( cat, "%s", name );
    fclose( cat );
    printf("Adding object to %s\n",cat); 
    add( );
  }

  /* end main */ } 


add( )

{

  int
    good,
    i,
    j;

  char
    ans[ 10 ],
    final[ 100 ],
    line[ 80 ],
    cat_name[ 80 ];

  strcpy( cat_name, CATALOGS );
  strcat( cat_name, name );
  cat = fopen( cat_name, "r" );

  num = 0;
  if ( cat != NULL ) {
    fgets( cat_array[ num ], 99, cat );
    while ( !feof( cat ) ) {
      if ( strcmp( "", cat_array[ num ] ) != 0 ) {
	sscanf( cat_array[ num ], "%d", &cat_index[ num ] );
	num++;
      }
      fgets( cat_array[ num ], 99, cat );
      cat_array[ num ][ strlen( cat_array[ num ] ) - 1 ] = '\0';
    }
    fclose( cat );
  }

  printf( "\n%d entries in catalog %s\n", num, name );

  do {

    do {

      printf( "\nCatalog entry #: " );
      gets( line );
      if ( strcmp( "", line ) == 0 ) {
	update( cat_name );
	return;
      }
      sscanf( line, "%d", &j );
      for( i = 0; i < num; i++ )
	if ( cat_index[ i ] == j )
	  break;
      if ( i != num ) {
	printf( "\n%s\n", cat_array[ i ] );
	printf( "%s exists. Delete ? ", line );
	gets( ans );
	j = i;
	good = ans[ 0 ] == 'y' || ans[ 0 ] == 'Y';
	if ( good )
	  printf( "\nCatalog entry #: %s\n" , line );
      }
      else {
	j = num;
	num++;
	good = 1;
      }

    } while ( !good );

    sscanf( line, "%d", &cat_index[ j ] );
    sprintf( cat_array[ j ], "%s ", line );

    printf( "Name           : " );
    gets( line );
    strcat( cat_array[ j ], "\'" );
    strcat( cat_array[ j ], line );
    strcat( cat_array[ j ], "\'" );
    strcat( cat_array[ j ], "   " );

    printf( "RA             : " );
    gets( line );
    strcat( cat_array[ j ], line );
    strcat( cat_array[ j ], "   " );

    printf( "DEC            : " );
    gets( line );
    strcat( cat_array[ j ], line );
    strcat( cat_array[ j ], "   " );

    printf( "PMRA           : " );
    gets( line );
    strcat( cat_array[ j ], line );
    strcat( cat_array[ j ], "   " );

    printf( "PMDEC          : " );
    gets( line );
    strcat( cat_array[ j ], line );

  } while( 0 );

  /* end add */ }


update( name )

char
   name[ ];

{

  int
    i;

  cat = fopen( name, "w" );
  for( i = 0; i < num; i++ )
    fprintf( cat, "%s\n", cat_array[ i ] );
  fclose( cat );

  /* end update */ }









