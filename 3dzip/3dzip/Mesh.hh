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

#ifndef VBE_MESH_HH
#define VBE_MESH_HH

#include <stdexcept>
#include "hds/HDS.hh"

namespace VBE {

class Mesh
{
protected:
    HDS::Mesh* mesh;
    std::vector<int> free_slots;

    void initFreeSlots(const int v, const unsigned val) {
	int &i = free_slots.at(v);
	if (i != -1)
	    throw std::logic_error("Vertex already initialized");
	i = val;
    }

public:
    Mesh(HDS::Mesh& m, const size_t num_ver = 0);
    virtual ~Mesh() {}
    size_t size() const {
	return mesh->size();
    }
    HDS::Handle_t operator()(const size_t i) const {
	return mesh->operator()(i);
    }
    int freeSlots(const int v) const {
	return free_slots.at(v);
    }
    int freeSlots(const HDS::Handle_t v) const {
	return freeSlots(v->Org());
    }
    virtual bool isConquered(const size_t) const { return false; }
    virtual void selectProps() const {}
};

}

#endif
