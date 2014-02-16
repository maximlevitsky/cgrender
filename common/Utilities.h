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

#include <math.h>
#include <assert.h>
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using std::min;
using std::max;
using std::swap;


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


#ifdef MSC_VER
static double log2( double n )  
{  
	// log(n)/log(2) is log2.  
	return log( n ) / log( 2.0 );  
}

static bool is_bad_fp(double val)
{
	switch(_fpclass(val))
	{
	case _FPCLASS_SNAN:
	case  _FPCLASS_QNAN:
	case _FPCLASS_NINF:
	case _FPCLASS_ND:
	case _FPCLASS_PD:
	case _FPCLASS_PINF:
		return true;
	default:
		return false;
	}
}
#else
static bool is_bad_fp(double x)
{
	return !std::isfinite(x);
}
#endif


/**************************************************************************************
 * Strings
 *
 * ************************************************************************************
 */

static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


static std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

#endif
