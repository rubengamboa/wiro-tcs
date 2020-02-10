#include <sys/ioctl.h>
#include <fcntl.h>

int setspeed(fides)
{
    struct sgttyb tryit;

	ioctl(fides, TIOCGETP, &tryit);
	tryit.sg_ispeed=EXTB;
	tryit.sg_ospeed=EXTB;
	ioctl(fides,TIOCSETP,&tryit);
	return(0);
}
