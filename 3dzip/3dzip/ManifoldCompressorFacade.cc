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

#include "ManifoldCompressorFacade.hh"

namespace VBE {

ManifoldCompressorFacade::ManifoldCompressorFacade(HDS::Mesh& m) : AIMesh(m)
{
    free_slots.resize(mesh->n_vertices());
    for (unsigned i = 0; i < size(); ++i) {
	HDS::Handle_t t = (*this)(i);
	free_slots[t->Org()]++;
	free_slots[t->Lnext()->Org()]++;
	free_slots[t->Lprev()->Org()]++;
    }
    visited.resize(mesh->n_vertices());
    tri_conq.resize(size());
}

void ManifoldCompressorFacade::conquest(const HDS::Handle_t t)
{
    HDS::Handle_t it = t;
    markVisited(t);
    markVisited(t->Lnext());
    markVisited(t->Lprev());
    tri_conq[t->Lface()] = true;
    if (face_cb && mesh->isDummyTriangle(t->Lface()) == false)
	face_cb(t->Lface());
}

}
