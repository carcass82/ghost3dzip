/*
    Copyright (C) 2009 Paolo Simone Gasparello <djgaspa@gmail.com>

    This file is part of libhds.

    libhds is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libhds is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libhds.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HDS_HH
#define HDS_HH

#include <map>
#include <deque>
#include <climits>
#include <iosfwd>
#include "halfedge.h"

namespace HDS {

typedef Halfedge<int> Halfedge_t;
typedef Halfedge_t::Handle Handle_t;

class Mesh
{
    typedef std::vector<Handle_t> Connectivity;
    Connectivity he_tri;
    std::deque<int> stitch;
    
    typedef std::deque<std::map<int, Handle_t> > EdgeContainer;
    EdgeContainer border;
    void build(const unsigned *first, const unsigned *last);
    void build(const Mesh &);
    void close();
    void close(const Handle_t h);

public:
    template <typename T>
    void add(const T& v, const int f);
    void add(const Handle_t); 
    void manifoldize();

public:
    Mesh() {}
    Mesh(const unsigned *first, const unsigned *last, const int num_ver, const bool correct_non_manifold = true);
    Mesh(const Mesh &);
    virtual ~Mesh();
    Mesh &operator=(const Mesh &);
    Handle_t operator()(const size_t i) const {
	return he_tri.at(i);
    }
    size_t size() const {
	return he_tri.size();
    }
    int getValence(const Handle_t v) const;
    bool isDummy(const Handle_t v) const {
	return isDummy(v->Org());
    }
    bool isDummy(const int v) const {
	return stitch.at(v) == INT_MAX;
    }
    bool isDummyTriangle(const Handle_t t) const {
	return t->Lface() == Halfedge_t::NoFace || isDummy(t->Org()) || isDummy(t->Lnext()->Org()) || isDummy(t->Lprev()->Org());
    }
    bool isDummyTriangle(const int i) const {
	return isDummyTriangle(he_tri.at(i));
    }
    bool isStitch(const Handle_t v) const {
	return stitch.at(v->Org()) < 0;
    }
    int addVertex(const int i) {
	stitch.push_back(i);
	border.resize(border.size() + 1);
	return stitch.size() - 1;
    }
    int addStitch(const int i) {
	return addVertex(-i - 1);
    }
    unsigned element(const HDS::Handle_t v) const {
	return element(v->Org());
    }
    unsigned element(const int v) const {
	int val = stitch[v];
	return val >= 0 ? val : -val - 1;
    }
    unsigned n_vertices() const {
	return stitch.size();
    }

    static Handle_t createTriangle();
    static void deleteTriangle(const Handle_t t);
    void erase(std::vector<bool> &v);
    template <typename Iter>
    void triangles(Iter tri) const;

    friend std::ostream &operator<<(std::ostream &os, const Mesh &m);
};

template <typename T>
void Mesh::add(const T& v, const int f)
{
    if (v[0] == v[1] || v[1] == v[2] || v[2] == v[0])
	return;
    Handle_t t = Halfedge_t::create_face(v, f);
    add(t);
}

template <typename OutputIter>
void Mesh::triangles(OutputIter first) const
{
    for (Connectivity::const_iterator i = he_tri.begin(); i != he_tri.end(); ++i) {
	const Handle_t h(*i);
	if (isDummyTriangle(h) == true)
	    continue;
	*first++ = element(h);
	*first++ = element(h->Lnext());
	*first++ = element(h->Lprev());
    }
}

}

#endif
