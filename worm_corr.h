/* This file defines the worm corrections to be used by wiro_track.c  */
/* The errors are defined in terms of seconds of arc at the equator,  */
/*  that is, they are really 15th of a second of time.                */
/*  A positive number MOVES the telescope west.                       */

/* Robert R. Howell    NEW COEFFICIENTS AS OF FEB. 26, 1995           */

#ifndef _worm_corr
#define _worm_corr

#define worm_n (256)     /* Number of terms in the correction array */

long   worm_indx;       /* Index into the array.  Keep long for intermediate results. */
double worm_corr[worm_n] =
 {
 0.08,
 0.10,
 0.11,
 0.12,
 0.13,
 0.14,
 0.15,
 0.16,
 0.17,
 0.18,
 0.18,
 0.19,
 0.19,
 0.20,
 0.20,
 0.20,
 0.20,
 0.21,
 0.21,
 0.21,
 0.22,
 0.22,
 0.23,
 0.24,
 0.25,
 0.26,
 0.27,
 0.29,
 0.30,
 0.32,
 0.34,
 0.35,
 0.37,
 0.39,
 0.41,
 0.42,
 0.43,
 0.45,
 0.45,
 0.46,
 0.47,
 0.48,
 0.47,
 0.47,
 0.48,
 0.48,
 0.48,
 0.47,
 0.47,
 0.47,
 0.47,
 0.47,
 0.47,
 0.47,
 0.47,
 0.48,
 0.47,
 0.47,
 0.48,
 0.48,
 0.48,
 0.48,
 0.48,
 0.48,
 0.47,
 0.46,
 0.45,
 0.44,
 0.42,
 0.41,
 0.39,
 0.37,
 0.35,
 0.33,
 0.31,
 0.29,
 0.27,
 0.25,
 0.22,
 0.21,
 0.18,
 0.16,
 0.14,
 0.11,
 0.09,
 0.07,
 0.05,
 0.03,
 0.00,
-0.02,
-0.05,
-0.07,
-0.10,
-0.12,
-0.15,
-0.17,
-0.19,
-0.22,
-0.24,
-0.26,
-0.28,
-0.30,
-0.32,
-0.34,
-0.35,
-0.36,
-0.37,
-0.38,
-0.39,
-0.40,
-0.41,
-0.42,
-0.42,
-0.43,
-0.44,
-0.45,
-0.46,
-0.48,
-0.49,
-0.50,
-0.51,
-0.52,
-0.54,
-0.56,
-0.57,
-0.58,
-0.59,
-0.60,
-0.61,
-0.61,
-0.61,
-0.61,
-0.62,
-0.61,
-0.61,
-0.60,
-0.59,
-0.58,
-0.57,
-0.56,
-0.55,
-0.55,
-0.54,
-0.52,
-0.51,
-0.51,
-0.50,
-0.48,
-0.47,
-0.46,
-0.45,
-0.44,
-0.43,
-0.41,
-0.40,
-0.39,
-0.37,
-0.35,
-0.34,
-0.32,
-0.31,
-0.28,
-0.26,
-0.25,
-0.23,
-0.21,
-0.20,
-0.19,
-0.17,
-0.16,
-0.15,
-0.14,
-0.13,
-0.12,
-0.11,
-0.10,
-0.08,
-0.07,
-0.06,
-0.04,
-0.03,
-0.01,
 0.02,
 0.03,
 0.06,
 0.08,
 0.10,
 0.11,
 0.14,
 0.16,
 0.18,
 0.19,
 0.20,
 0.21,
 0.23,
 0.23,
 0.23,
 0.22,
 0.22,
 0.21,
 0.20,
 0.19,
 0.18,
 0.17,
 0.15,
 0.14,
 0.13,
 0.11,
 0.10,
 0.09,
 0.08,
 0.06,
 0.06,
 0.05,
 0.04,
 0.04,
 0.04,
 0.04,
 0.04,
 0.03,
 0.03,
 0.04,
 0.03,
 0.04,
 0.03,
 0.03,
 0.03,
 0.03,
 0.04,
 0.03,
 0.03,
 0.03,
 0.03,
 0.03,
 0.02,
 0.02,
 0.02,
 0.02,
 0.02,
 0.01,
 0.01,
 0.01,
 0.01,
 0.01,
 0.01,
 0.01,
 0.01,
 0.02,
 0.02,
 0.02,
 0.03,
 0.03,
 0.04,
 0.05,
 0.07,
 0.07
} ;	
#endif
