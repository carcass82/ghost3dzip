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

#ifndef READER_HH
#define READER_HH

#include <iosfwd>

namespace VBE {

class Reader
{
    struct Impl;
    Impl* pimpl;
    unsigned n_tri, n_ver;

public:
    Reader();
    ~Reader();
    void operator()(std::istream& is);
    unsigned getNumTri() const {
	return n_tri;
    }
    unsigned getNumVer() const {
	return n_ver;
    }
    unsigned getNumAttr() const;
    bool hasAttrib(const char* name) const;
    unsigned rows(const char* name) const;
    unsigned* getTriangles() const;
    void getTriangles(unsigned* p) const;
    float* getAttrib(const char* name) const;
    void getAttrib(const char* name, float* p) const;
    const char* getName(const unsigned int i) const;
};

}

#endif
