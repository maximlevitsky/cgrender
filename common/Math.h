/*
	This file is part of CG4.

	Copyright (c) Inbar Donag and Maxim Levitsky

    CG4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CG4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UTILITES_H
#define UTILITES_H

#include <cmath>
#include <algorithm>
#include <assert.h>

using std::min;
using std::max;
using std::swap;
using std::floor;
using std::ceil;

#ifndef MSC_VER
	using std::isfinite;
#endif

/**************************************************************************************
 * Floating point helpers
 * 
 * ************************************************************************************
 */

template <typename T> 
T static inline clamp(T value, T min_value, T max_value)
{
	if (value < min_value)
		return min_value;
	if (value > max_value)
		return max_value;
	return value;
}

static inline double frac(const double& x)
{
	return x - floor(x);
}


static inline double powi (double val, unsigned int power)
{
   double result = 1;

   while (power)
   {
		if (power & 1)
			result *= val;

		power >>= 1;
		val *= val;
   }

   return result;
}



#ifdef MSC_VER
static double log2( double n )  
{  
	// log(n)/log(2) is log2.  
	return log( n ) / log( 2.0 );  
}

static bool isfinite(double val)
{
	switch(_fpclass(val))
	{
	case _FPCLASS_SNAN:
	case  _FPCLASS_QNAN:
	case _FPCLASS_NINF:
	case _FPCLASS_ND:
	case _FPCLASS_PD:
	case _FPCLASS_PINF:
		return false;
	default:
		return true;
	}
}
#endif


#endif
