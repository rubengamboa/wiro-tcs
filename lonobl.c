/**********
 * Return the nutation in longitude and the nutation in
 * obliquity for the given julian date
 **********/
#include <math.h>
#include "wtrack.h"

void lonobl(double jul, double *lon, double *obl)

{

double T, T2, T3, omega, l, lp, F, D, tomega, tl, tlp, tF, tD, r = 1296000.0;

T = (jul - 2451545.0) / 36525.0;
T2 = T * T;
T3 = T2 * T;

/******
omega = 450160.280 - (5 * r + 482890.539) * T + 7.455 * T2 + 0.008 * T3;
l = 485866.733 + (1325 * r + 715922.633) * T + 31.310 * T2 + 0.064 * T3;
lp = 1287099.804 + (99 * r + 1292581.224) * T - 0.577 * T2 - 0.012 * T3;
F = 335778.877 + (1342 * r + 295263.137) * T - 13.257 * T2 + 0.011 * T3;
D = 1072261.307 + (1236 * r + 1105601.328) * T - 6.891 * T2 + 0.019 * T3;
******/

omega = 450160.280 - 6962890.539 * T + 7.455 * T2 + 0.008 * T3;
l = 485866.733 + (1717915922.633) * T + 31.310 * T2 + 0.064 * T3;
lp = 1287099.804 + (129596581.224) * T - 0.577 * T2 - 0.012 * T3;
F = 335778.877 + (1739527263.137) * T - 13.257 * T2 + 0.011 * T3;
D = 1072261.307 + (1602961601.328) * T - 6.891 * T2 + 0.019 * T3;

omega = RAD(omega / 3600.0);
l = RAD(l / 3600.0);
lp = RAD(lp / 3600.0);
F = RAD(F / 3600.0);
D = RAD(D / 3600.0);

tomega = 2.0 * omega;
tl = 2.0 * l;
tlp = 2.0 * lp;
tF = 2.0 * F;
tD = 2.0 * D;

*lon = - (17.1996 + 0.01742 * T) * sin(omega)
       + 0.2062 * sin(tomega)
       + 0.0046 * sin(-tl + tF + omega)
       + 0.0011 * sin(tl - tF)
       - 1.3187 * sin(tF - tD + tomega)
       + 0.1426 * sin(lp)
       - 0.0517 * sin(lp + tF - tD + tomega)
       + 0.0217 * sin(-lp + tF - tD + tomega)
       + 0.0129 * sin(tF - tD + omega)
       + 0.0048 * sin(tl - tD)
       - 0.0022 * sin(tF - tD)
       + 0.0017 * sin(tlp)
       - 0.0015 * sin(lp + omega)
       - 0.0016 * sin(tlp + tF - tD + tomega)
       - 0.0012 * sin(-lp + omega)
       - 0.2274 * sin(tF + tomega)
       + 0.0712 * sin(l)
       - 0.0386 * sin(tF + omega)
       - 0.0301 * sin(l + tF + tomega)
       - 0.0158 * sin(l - tD)
       + 0.0123 * sin(-l + tF + tomega)
       + 0.0063 * sin(tD)
       + 0.0063 * sin(l + omega)
       - 0.0058 * sin(-l + omega)
       - 0.0059 * sin(-l + tF + tD + tomega)
       - 0.0051 * sin(l + tF + omega)
       - 0.0038 * sin(tF + tD + tomega)
       + 0.0029 * sin(tl)
       + 0.0029 * sin(l + tF - tD + tomega)
       - 0.0031 * sin(tl + tF + tomega)
       + 0.0026 * sin(tF)
       + 0.0021 * sin(-l + tF + omega)
       + 0.0016 * sin(-l + tD + omega)
       - 0.0013 * sin(l - tD + omega)
       - 0.0010 * sin(-l + tF + tD + omega);
       
*obl =   9.2025 * cos(omega)
       - 0.0895 * cos(tomega)
       - 0.0024 * cos(-tl + tF + omega)
       + 0.5736 * cos(tF - tD + tomega)
       + 0.0054 * cos(lp)
       + 0.0224 * cos(lp + tF - tD + tomega)
       - 0.0095 * cos(-lp + tF - tD + tomega)
       - 0.0070 * cos(tF - tD + omega)
       + 0.0977 * cos(tF + tomega)
       + 0.0200 * cos(tF + omega)
       + 0.0129 * cos(l + tF + tomega)
       - 0.0053 * cos(-l + tF + tomega)
       - 0.0033 * cos(l + omega)
       + 0.0032 * cos(-l + omega)
       + 0.0026 * cos(-l + tF + tD + tomega)
       + 0.0027 * cos(l + tF + omega)
       + 0.0016 * cos(tF + tD + tomega)
       - 0.0012 * cos(l + tF - tD + tomega)
       + 0.0013 * cos(tl + tF + tomega)
       -0.0010 * cos(-l + tF + omega);

*lon /= 3600.0;
*obl /= 3600.0;

/* END lonobl */ }

