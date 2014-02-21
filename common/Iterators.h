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
#ifndef ITERATORS_H
#define ITERATORS_H

class polygonIterator
{
public:
	polygonIterator(unsigned int* polygons, int polygonCount) :
	  _polygon_ptr(polygons), _polygonCount(polygonCount), _currentPolygon(0)
	  {}

	  bool  hasmore() const
	  {
		  return _currentPolygon < _polygonCount;
	  }

	  void next()
	  {
		  _polygon_ptr += *_polygon_ptr + 1;
		  _currentPolygon++;
	  }

	  unsigned int operator[] (int i) const
	  {
		  return _polygon_ptr[i+1];
	  }

	  int vertexCount() const { return *_polygon_ptr; }

	  int _currentPolygon;

private:
	unsigned int* _polygon_ptr;
	const int _polygonCount;
};

#endif
