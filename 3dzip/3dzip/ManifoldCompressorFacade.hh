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
#include <boost/function.hpp>
#include "AIMesh.hh"
#include "StitchSymbol.hh"

namespace VBE {

class ManifoldCompressorFacade : public AIMesh
{
protected:
    typedef boost::function<void(const int, const unsigned)> ImmediateOutput;

    std::vector<bool> visited, tri_conq;
    std::vector<int> stitches;

    typedef boost::function<void(const HDS::Mesh&, const HDS::Handle_t, const HDS::Handle_t)> AttribFun;
    std::vector<AttribFun> p;
    typedef boost::function<void(const int face_idx)> FaceCallback;
    FaceCallback face_cb;

    void markVisited(const HDS::Handle_t v) {
	visited.at(v->Org()) = true;
    }
    virtual bool has_attributes(const HDS::Handle_t v) {
	return mesh->isDummy(v) == false;
    }

public:
    ManifoldCompressorFacade(HDS::Mesh& m);
    virtual ~ManifoldCompressorFacade() {}
    virtual bool isVisited(const int v) const {
	return visited.at(v);
    }
    bool isVisited(const HDS::Handle_t v) const {
	return isVisited(v->Org());
    }
    virtual bool isConquered(const size_t i) const {
	return tri_conq.at(i) == true;
    }
    virtual void conquest(const HDS::Handle_t);
    int valence(const HDS::Handle_t v) const {
	return mesh->isDummy(v) == false ? freeSlots(v) : -(int)freeSlots(v);
    }
    void encodeProps(const HDS::Handle_t v, const HDS::Handle_t active_edge = NULL) {
	if (has_attributes(v) == true)
	    for (std::vector<AttribFun>::iterator i = p.begin(); i != p.end(); ++i)
		(*i)(*mesh, v, active_edge);
    }
    void addProperty(const AttribFun& f) {
	p.push_back(f);
    }
    void setFaceCallback(const FaceCallback& f) {
	face_cb = f;
    }
};

}
