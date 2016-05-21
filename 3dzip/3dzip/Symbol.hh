/*
    Copyright (C) 2009-2011 Paolo Simone Gasparello <djgaspa@gmail.com>

    This file is part of lib3dzip.

    lib3dzip is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    lib3dzip is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with lib3dzip.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <climits>
#include <stdexcept>
#include <iosfwd>

namespace VBE {

enum Type {ADD = 0, SPLIT, MERGE, END_OF_MESH, NONE, NUM_TYPES};

struct Symbol
{
    Type type;
    short int val1, val2, val3;

    explicit Symbol(const Type t = NONE, const short int v1 = 0, const short int v2 = 0, const short int v3 = 0) :
    type(t), val1(v1), val2(v2), val3(v3) { }
    explicit Symbol(const unsigned s) : type(NONE), val1(SHRT_MAX), val2(SHRT_MAX), val3(0) {
	if (s < 29 && s > 1) {
	    type = ADD;
	    val1 = s;
	}
	else if (s == 29)
	    type = ADD;
	else if (s == 30)
	    type = ADD;
	else if (s == 0)
	    type = SPLIT;
	else if (s == 1)
	    type = MERGE;
	else if (s == 31)
	    type = END_OF_MESH;
    }
    /*bool operator<(const Symbol &s) const {
	if (type == s.type)
	    if (val1 == s.val1)
		return val2 < s.val2;
	    else
		return val1 < s.val1;
	else
	    return type < s.type;
    }*/
    operator unsigned() const {
	unsigned cs;
	switch (type) {
	    case ADD:
		if (val1 < 0)
		    cs = 29;
		else if (val1 > 28)
		    cs = 30;
		else
		    cs = val1;
		break;
	    case SPLIT:
		cs = 0;
		break;
	    case MERGE:
		cs = 1;
		break;
	    case END_OF_MESH:
		cs = 31;
		break;
	    default:
		throw std::runtime_error("Want to output an invalid symbol");
	}
	return cs;
    }
    static bool isAdd(const unsigned s) {
	return (s > 1 && s < 29) || s == 30;
    }
    bool has_v1() const {
	return (type == SPLIT ||
		type == MERGE ||
		(type == ADD && (val1 < 0 || val1 > 28)));
    }
    bool has_v2() const {
	return type == MERGE || type == SPLIT;
    }
    bool has_v3() const {
	return type == MERGE;
    }
};

std::ostream &operator<<(std::ostream &os, const Symbol &s);

}

#endif
