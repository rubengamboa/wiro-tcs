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

FILE *cat;

main( int argc, char *argv[] )

{
  char  cat_name[ 180 ],command[200];

  printf("%s is CATALOGS\n",CATALOGS);
  strcpy(cat_name, CATALOGS);
  strcat(cat_name, "current.cat");

  cat = fopen( cat_name, "r" );
  if ( cat == NULL ) {
    printf( "Unable to open %s\n",cat_name );
    exit( -1 );
  }
  fscanf(cat,"%s",cat_name);


  fclose( cat );
  strcpy(command,"less ");
  printf("The current catalog is %s\n",cat_name);
  strcat(command,CATALOGS);
  strcat(command,cat_name);
  system(command);

}





