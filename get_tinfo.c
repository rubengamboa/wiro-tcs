/* This handy routine returns a pointer to the shared memory area *tinfo.
   It should also take care of revision checking, but not yet.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

struct 
   wiro_memory *tinfo;


struct wiro_memory *get_tinfo() 
{
  int fd;
  void * asdf;

  fd = open(CORE, O_RDWR);
  if (fd == -1) {
	printf("Unable to open corefile \n");
	exit(-1);
  }
  tinfo = (struct wiro_memory *)
    mmap(NULL, sizeof(*tinfo),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);

  
  /*	if ( NULL == ( tinfo = (struct wiro_memory *) 
	    	smem_get("WIRO_MEMORY",sizeof(*tinfo), SM_READ | SM_WRITE ) ) ) {
		printf("Unable to open shared memory \n");
		exit(-1);
		} */

  return(tinfo);
}

