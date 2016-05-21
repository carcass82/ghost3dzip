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

#ifndef AOMESH_HH
#define AOMESH_HH

#include <boost/array.hpp>

#include "Mesh.hh"

namespace VBE {

class AOMesh : public Mesh
{
public:
    typedef boost::array<int, 3> Triangle;
    AOMesh(HDS::Mesh& m, const unsigned num_ver) : Mesh(m, num_ver) {}
    virtual ~AOMesh() {}
    virtual HDS::Handle_t conquest(const Triangle &v) = 0;
    virtual int decodeProps(const int val, const HDS::Handle_t active_edge = NULL) = 0;
};

}

#endif
