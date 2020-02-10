/**********
 * Return the proper corrections for annual
 * abberration for the given julian date.
 *
 * Reference: Microcomputer Control of Telescopes
 *            Trueblood & Genet
 *            Copyright 1985
 *
 * Pages: 87-88
 **********/

#include <math.h>
#include "track.h"

void annabr(double jul, double *deltara, double *deltadec)

{
double radra, raddec, C, c, cp, D, d, dp, k, L, Lm,
       obl, diff, newE, T, T2, T3, lp, v, e, E;

radra = RAD(*deltara * 15.0);
raddec = RAD(*deltadec);

/***** Number of Julian centuries *****/

T = (jul - 2451545.0) / 36525.0;
T2 = T * T;
T3 = T2 * T;

/***** Mean anomaly of the Sun *****/

lp = RAD(357.5277233 + 35999.05034 * T - 0.0001602778 * T2 - 0.0000033333 * T3);

/***** Eccentricity of the Earth's orbit *****/

e = 0.016708320 - 0.000042229 * T - 0.000000126 * T2;

/***** Eccentric annomaly of the Earth in its orbit *****/

E = lp;
diff = 1.0;
while (fabs(diff) > 0.0000000048) {
   newE = lp + e * sin(E);
   diff = newE - E;
   E = newE;
}

/***** True anomaly of the Earth in its orbit *****/

v = 2.0 * atan(sqrt((1 + e) / (1 - e)) * tan(E / 2.0));

/***** Geometric mean longitude of the Sun *****/

Lm = RAD(280.4660694 + 36000.7697972222 * T + 0.0003025 * T2);

/***** Sun's true longitude *****/

L = Lm + v - lp;

/***** Obliquity of the ecliptic *****/

obl = RAD(oblecl(jul));

dp = cos(radra) * sin(raddec);

cp = tan(obl) * cos(raddec) - sin(radra) * sin(raddec);

d = 0.06666666667 * sin(radra) / cos(raddec);

c = 0.06666666667 * cos(radra) / cos(raddec);

/***** Constant of aberration *****/

k = RAD(0.0056932);

D = -k * sin(L);

C = -k * cos(obl) * cos(L);

*deltara = DEG(C * c + D * d);

*deltadec = DEG(C * cp + D * dp);

/* END annabr */ }
