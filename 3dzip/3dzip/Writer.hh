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

#ifndef WRITER_HH
#define WRITER_HH

#include <iosfwd>

namespace VBE {

class Writer
{
    struct Impl;
    Impl* pimpl;

    static unsigned char suggestQ(const unsigned n);
    Writer(const Writer&);
    Writer& operator=(const Writer&);

public:
    Writer(const unsigned* p, const size_t n, const size_t num_veri, const bool correct_manifold = true);
    ~Writer();
    void addAttrib(const float* p, const size_t n, const size_t dim, const char* descr, int q_bit = 0);
    void operator()(std::ostream& os);
    unsigned* getVertexOrdering();
    unsigned* getFaceOrdering();
};

}

#endif
