#include <sys/wait.h>
#include <pthread.h>      /*  POSIX threads package. */

void* do_screens()
{
  int i, tmp=1;

  for (i=0; i<10; i++) {
    printf("step %d\n",i);
    usleep( (unsigned long) 1000000);
  }


}

void main(int argc, char *argv[])
{	pthread_t thTrack,thScreen,thStore,thDiagnostic;
	pthread_attr_t thatTrack, thatScreen, thatStore, thatDiagnostic;
	long tmp;
	int ret;

	pthread_attr_init(&thatScreen);
	/*	pthread_attr_setschedpolicy(&thatScreen, SCHED_RR); */
	pthread_attr_setschedpolicy(&thatScreen, SCHED_OTHER);

	pthread_attr_setdetachstate(&thatScreen,PTHREAD_CREATE_JOINABLE); 
	printf("0\n");
	ret =  pthread_create( &thScreen, &thatScreen, (void *)do_screens , NULL);
	/*	printf("Thread value is %d\n",ret); */
	printf("1.5\n");
	pthread_join(thScreen, &tmp); 

	printf("2\n");
	exit(1);
}


