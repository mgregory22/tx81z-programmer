/*
 * random.c - interface for random integers
 *
 * Copyright (c) 2006, 2015 Matt Gregory
 *
 * This file is part of TX81Z Programmer.
 *
 * TX81Z Programmer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * TX81Z Programmer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with TX81Z Programmer.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

/*
 * Global procedures
 */
extern double Rand1(void);
extern void Randomize(void);
extern __inline int RandNum(int num);
extern __inline int RandRange(int min, int max);

/*
 * Unit variables
 */
static long idum = 1L;

/*
 * Procedure definitions
 */
void Randomize(void)
{
	idum = (long) -time(NULL);
}


/* Rand1() - "Minimal" random number generator of Park and Miller with
 *           Bays-Durham shuffle and added safeguards. Returns a uniform
 *           random deviate between 0.0 and 1.0 (exclusive of the
 *           endpoint values). Call with idum a negative integer to
 *           initialize; thereafter, do not alter idum between successive
 *           deviates in a sequence. RNMX should approximate the largest
 *           floating value that is less than 1.
 */

double Rand1(void)
{
    int j;
    long k;
    static long iy=0;
    static long iv[NTAB];
    double temp;

    if (idum <= 0 || !iy) {
        idum = (-idum < 1) ? 1 : -idum;
        for (j = NTAB + 7; j >= 0; j--) {
            k = idum / IQ;
            idum = IA * (idum - k * IQ) - IR * k;
            if (idum < 0)
                idum += IM;
            if (j < NTAB)
                iv[j] = idum;
        }
        iy=iv[0];
    }
    k = idum / IQ;
    idum = IA * (idum - k * IQ) - IR * k;
    if (idum < 0)
        idum += IM;
    j = iy / NDIV;
    iy = iv[j];
    iv[j] = idum;

    if ((temp = AM * iy) > RNMX)
        return RNMX;

    return temp;
}

