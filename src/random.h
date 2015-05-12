/*
 * msg/random.h - interface for random integers
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

#ifndef MSG_RANDOM_H
#define MSG_RANDOM_H

/*
 * Rand1() - RNG's from Numerical Recipes in C
 */
double Rand1(void);

/*
 * Randomize() - Seeds the random number generator.
 */
void Randomize(void);

/*
 * RandNum() - Returns a random number between 0 and (num - 1).
 */
__inline int RandNum(int num);

/*
 * RandRange() - Returns a random number between min and max inclusive.
 */
__inline int RandRange(int min, int max);

/*
 * Inline definitions
 */
int RandNum(int num)
{
	//return (int) ((double) rand() / ((double) RAND_MAX + 1) * num);
    //return rand() / (RAND_MAX / num + 1);
    return (int) ((double) Rand1() * num);
}

int RandRange(int min, int max)
{
	//return (int) ((double) rand() / ((double) RAND_MAX + 1)
    //    * ((max + 1) - min)) + min;
    return (int) ((double) Rand1() * ((max + 1) - min) + min);
}

#endif
