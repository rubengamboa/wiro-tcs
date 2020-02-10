
#include <stdio.h>
#include "wirotypes.h"
#include "track.h"
#include "wiro.h"

struct 
   wiro_memory *tinfo;
main()
{

tinfo = get_tinfo();
tinfo->keep_tracking = REMOTE_REFRESH;
}
