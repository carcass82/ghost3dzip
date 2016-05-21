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

#ifndef AIMESH_HH
#define AIMESH_HH

#include "Mesh.hh"

namespace VBE {

class AIMesh : public Mesh
{
public:
    AIMesh(HDS::Mesh& m) : Mesh(m) {}
    virtual ~AIMesh() {}
    virtual bool isVisited(const HDS::Handle_t v) const = 0;
    virtual void conquest(const HDS::Handle_t) = 0;
    virtual int valence(const HDS::Handle_t v) const = 0;
    virtual void encodeProps(const HDS::Handle_t v, const HDS::Handle_t active_edge = NULL) = 0;
};

}

#endif
