/* Sets the particular terminal unit to raw mode */

#include <sys/ioctl.h>
#include <sgtty.h>
#include <fcntl.h>

setraw ( unit )

int unit;

{

   struct termio  tbuf;
   int flags;

   ioctl ( unit, TCGETA, &tbuf);

   if ( unit != 0 ) {
      tbuf.c_cflag &= ~CBAUD;
      tbuf.c_cflag |= EXTA;
   }
   tbuf.c_iflag &= ~(INLCR | ICRNL | IUCLC | ISTRIP | IXON | BRKINT );
   tbuf.c_oflag &= ~OPOST;
   tbuf.c_lflag &= ~(ICANON | ISIG | ECHO);
   tbuf.c_cc[4] = 0;
   tbuf.c_cc[5] = 10;

   ioctl ( unit, TCSETAF, &tbuf);

   flags = fcntl( unit, F_GETFL, 0 );
   fcntl( unit, F_SETFL, flags | O_NDELAY );

}
