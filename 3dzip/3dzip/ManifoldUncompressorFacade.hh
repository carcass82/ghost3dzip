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

#ifndef MANIFOLD_UNCOMPRESSOR_FACADE_HH
#define MANIFOLD_UNCOMPRESSOR_FACADE_HH

#include <boost/function.hpp>
#include "AOMesh.hh"

namespace VBE {

class ManifoldUncompressorFacade : public AOMesh
{
protected:
    int next_vertex;

    typedef boost::function<void(const HDS::Mesh&, const int, const HDS::Handle_t)> AttribFun;
    std::vector<AttribFun> p;

    int reqDummyVertex() {
	return reqVertex(INT_MAX);
    }
    int reqVertex() {
	return reqVertex(next_vertex++);
    }
    int reqVertex(const int n) {
	int i = mesh->addVertex(n);
	free_slots.push_back(-1);
	return i;
    }

    virtual bool has_attributes(int &v);

public:
    ManifoldUncompressorFacade(HDS::Mesh& m, const unsigned num_ver):
	AOMesh(m, num_ver), next_vertex(0) {}
    virtual ~ManifoldUncompressorFacade() {}
    virtual HDS::Handle_t conquest(const Triangle &v);
    virtual int decodeProps(const int val, const HDS::Handle_t active_edge = NULL);
    void addProperty(const AttribFun& ap) {
	p.push_back(ap);
    }
};

}

#endif
