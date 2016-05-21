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

#pragma once
#include <ostream>

namespace VBE {

typedef unsigned char StitchType;
static const StitchType ST_NONE = 0;
static const StitchType ST_PUSH = 1;
static const StitchType ST_GET = 2;
static const StitchType ST_POP = 3;

struct StitchSymbol
{
    StitchType type;
    short index;
    unsigned size;

public:
    StitchSymbol(StitchType t, const short i = 0, const unsigned s = 0):
	type(t), index(i), size(s)
    {}
    StitchSymbol(const size_t s):
	type(ST_NONE), index(0), size(s)
    {}
    bool has_index() const {
	return type == ST_GET;
    }
    operator unsigned() const {
	return type;
    }
};

inline std::ostream& operator<<(std::ostream& os, const StitchSymbol& s)
{
    static const char *name[4] = {"None", "Push", "Get", "Pop"};
    os << name[s.type];
    if (s.has_index() == true)
	os << '\t' << s.index << " (" << s.size << ')';
    return os;
}

}
