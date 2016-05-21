/*
    Copyright (C) 2009 Paolo Simone Gasparello <djgaspa@gmail.com>

    This file is part of libhds.

    libhds is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libhds is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libhds.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HALF_EDGE_HH
#define HALF_EDGE_HH

class HalfEdge
{
    friend std::ostream &operator<<(std::ostream &os, const HalfEdge &e);
    int v1, v2;
public:
    HalfEdge(const unsigned int _v1, const unsigned int _v2) : v1(_v1), v2(_v2) { }
    bool operator< (const HalfEdge &_e) const {
	return (v1 < _e.v1) || (v1 == _e.v1 && v2 < _e.v2);
    }
    bool operator== (const HalfEdge &e) const {
	return v1 == e.v1 && v2 == e.v2;
    }
    int getV1() const {
	return v1;
    }
    int getV2() const {
	return v2;
    }
};

#endif
