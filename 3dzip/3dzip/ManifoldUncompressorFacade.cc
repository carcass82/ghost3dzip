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

#include <cstdlib>
#include "ManifoldUncompressorFacade.hh"

namespace VBE {

HDS::Handle_t ManifoldUncompressorFacade::conquest(const Triangle &v)
{
    int next_triangle = 0;
    HDS::Handle_t t = HDS::Halfedge_t::create_face(v, next_triangle++);
    mesh->add(t);
    return t;
}

bool ManifoldUncompressorFacade::has_attributes(int &v)
{
    v = reqVertex();
    return true;
}

int ManifoldUncompressorFacade::decodeProps(const int val, const HDS::Handle_t active_edge)
{
    int v;
    if (val < 0)
	v = reqDummyVertex();
    else if (has_attributes(v) == true) {
	for (std::vector<AttribFun>::iterator i = p.begin(); i != p.end(); ++i)
	    (*i)(*mesh, v, active_edge);
    }
    initFreeSlots(v, std::abs(val));
    return v;
}

}
