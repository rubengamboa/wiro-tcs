/* track.h          July 23, 1991           Robert. R. Howell.           */
/* This header file contains misc. things which can be included in all   */
/* of the tracking code programs.  It does not include the shared memory */
/* structures in atwiro.h .  Track.h should be incuded before atwiro.h  .*/

/* Add declaration for log_entry().  19Jan00 JSW */

#include <sys/time.h>

struct wiro_memory *tinfo_ptr(void);   /* Prototype of fcn which gets    */
                                       /* pointer to tinfo memory block. */

struct wiro_memory *get_tinfo();
double julian( int year, int month, int day, double time );
double oblecl(double jul);
char *hr_hms(double hr, int fld, int dpl);
double dec2hms(double dec);
double str2dec( char str[] );
void lonobl(double jul, double *lon, double *obl);
void nutation(double jul, double *ra,double *dec);
double hms2dec(double hms);
void annabr(double jul, double *deltara, double *deltadec);
double rem(double val);
double rad( double x);
void tscreen_init(void);        /* Get ready to print screen */
void tminn(void);               /* Print Minnesota style screen        */
int follow( void );
int timecopy(struct timeval src, struct timeval *dest);
int timesub(struct timeval first, struct timeval second, struct timeval *dest);
double timesize( struct timeval a);
int timeprint(struct timeval a);
void logentry( char *comment );
