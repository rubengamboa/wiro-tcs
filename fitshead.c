/******************************

	fitshead library of fits header utilities

	fitsout() writes card info for specified card name to fits header
	fitsin() retrieves card info for specified card name from fits header
	fitsrep() replaces card info for specified card name to fits header

	Kopywrite ESpn 1989,1990
	modifications by Pfred!

	verbose comments at end of file

*******************************/


#define UNIX
#include <stdio.h>
#include <string.h>
#ifdef UNIX
#include <unistd.h>
#endif


int fitsout(s,key,type,value)
FILE *s;
char *key;
char type;
void *value;
{
char logic='N';
long tell();
static char blanks[]="     ";
static int bytesout=0;
if(type == 'c')
	fprintf(s,"%-8s %-71s", "COMMENT", (char*) value);
else if (type == 'l') {
	if(&value)
		logic='T';
	fprintf(s,"%-8s= %-19s%c%-50s",key,blanks,logic,blanks);
	}
else if(type == 'i')
	fprintf(s,"%-8s= %20d%-50s",key, *((int*) value),blanks);
else if (type == 'f')
	fprintf(s,"%-8s= %20f%-50s",key, *((double*) value),blanks);
else if (type == 'h')
	fprintf(s,"%-8s= '%-19s'%-49s",key, value,blanks);
else if (type == 'e')
	fprintf(s,"END%77s",blanks);

if (type == 'e') {
	bytesout=(int) ftell(s);
	while( bytesout > 2880)
		bytesout -= 2880;
	fprintf(s,"%*s",2880-bytesout,blanks);
	}
return (1);
}



int fitsin(s,key,type,rv)
FILE *s;
char *key;
char type;
void* rv;
{
char buff[80],inkey[9];
static char inbuff[80];
long here;
long bytesout;

buff[79] = NULL;
do{
	here=ftell(s);
	if(80 != fread(buff,(long)1,(long)80,s))
		goto NOLUCK_ONE;
	sscanf(buff,"%8s",inkey);
	if(strcmp("END",inkey) == 0 && type != 'd' )
		goto NOLUCK_TWO;
	if(strcmp("END",inkey) == 0 && type == 'd' ) {
		bytesout= ftell(s);
		while( bytesout > (long) 2880)
			bytesout -= 2880;
		fseek(s,(( (long) 2880)-bytesout), SEEK_CUR);
		return(1);
		}
	} while(strcmp(inkey,key));

buff[79]=NULL;
fseek(s,here,SEEK_SET);

if (type == 'i') {
	sscanf(buff+11,"%20d", (int*) rv );
/* 		printf("%d", *(int*) rv );  */
	return(1);
	} else if (type == 'f') {
	sscanf(buff+11,"%20lf", (double*) rv );
	return(1);
	} else if (type == 'c') {
	memcpy(inbuff,buff,80);
	inbuff[79]=NULL;
	return(1);
	} else if (type == 'h'){
	sscanf(buff+11,"%20s", (char*) rv);
	return(1);
	}	else if (type == 's')
	return(1);
return(0);

NOLUCK_ONE:

fseek(s,here,SEEK_SET);
printf("No luck reading file. \n");
return (0);

NOLUCK_TWO:

fseek(s,here,SEEK_SET);
printf("No luck finding card name. \n");
return (0);
}



#include <stdio.h>
#include <string.h>

fitsrep(s,key,type,value)
FILE *s;
char *key;
char type;
void *value;
{
int i;

i=fitsin(s,key,'s',value);

if ( i == 1 ){
	fitsout(s,key,type,value);
	} else if ( i != 1 ) {
	fitsout(s,key,type,value);
	fitsout(s,"END",'e',value);
	}
}

/*************************************************************
   This file contains subroutines to handle fits headers
	 FITSOUT handles writing keywords and variables
		 fitsout( FILE *s, char *key, char type, void *value)
	   FITSIN  handles reading in keywords and variables
	   FITSREP	 changes a key value

  ALL FILES SHOULD BE OPENED BINARY (RAW)

					ver 1.1 23 june 1989             esp
 improved FITSIN to place pointer cat current read position
 added FITSREP
					ver 1.2 2 july 1989              esp
 Make the floating variables double to accomodate coordinates
					ver 1.3 19 sept 1989             esp
 put = sign in position 9 instead of 10 to make IRAF happy
					ver 1.31 7 jan 1990              esp
 add 'd' code to fitsin - seek beggining of data

					ver 2.0 started 20 jan 1990      esp
 add routines to open files
 work with raw files
 add buffering
 add autodoc
****************************************************************/

/****************************************************************
  FITSOUT writes an individual FITS keyword to the stream s.
   s should be a file pointer,
	 *key should be the keyword string,
	 type should be the type of keyvalue for formatting
			  'c' is a comment
			  'l' logical - T or F in 30
			  'i' integer - right just. in 11-30, i part 31-40
			  'f' floating - F is free format 11-30 31-40
							 remember the decimal!
							 E is right justified
				   NOTE: f is a double value
			  'h' character, surrounded by ' in 11 and 20+
							 left justified
			  'e' types the end statement and pads out the
					  header to 2880 bytes
	  *value should be a pointer to the value
****************************************************************/
/****************************************************************
  void *FITSIN reads an individual FITS keyword
	from the stream s.
	 s should be a file pointer to the beggining of the
			header which will be searched,
	 *key should be the keyword string searched for,
	 type should be the type of keyvalue for formatting
			  'c' is a comment
			  'l' logical - T or F in 30
			  'i' integer - right just. in 11-30, i part 31-40
			  'f' floating - F is free format 11-30 31-40
							 remember the decimal!
							 E is right justified
					NOTE: returns pointer to DOUBLE
			  'h' character, surrounded by ' in 11 and 20+
							 left justified
			  's' just seeks the appropriate spot line
							  and stays pointed there
			  'd' seeks the beggining of the data, that is
				  the 2880 boundry after the END.
   The stream is left pointed at the record which contained
	  the keyword in question
   The return is the current pointer on success, 0 otherwise.
****************************************************************/
/**************************************************************
  FITSREP replaces an individual FITS keyword in a stream s.
	 if the key does not exist, it is appended at the end of
	 the file and END is added

	 s should be a file pointer,
	 *key should be the keyword string,
	 type should be the type of keyvalue for formatting
			  'c' is a comment
			  'l' logical - T or F in 30
			  'i' integer - right just. in 11-30, i part 31-40
			  'f' floating - F is free format 11-30 31-40
							 remember the decimal!
							 E is right justified
					NOTE: argument is DOUBLE!!!!
			  'h' character, surrounded by ' in 11 and 20+
							 left justified
	  *value should be a pointer to the value
**************************************************************/
/* END OF FILE */
