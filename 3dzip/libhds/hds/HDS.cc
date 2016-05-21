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

#include <boost/array.hpp>
#include <set>
#include <list>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <stdexcept>
#include "HDS.hh"

namespace HDS {

struct Data {
    char counter;
    Handle_t p;
    Data(): counter(0), p(NULL) {}
};

Mesh::Mesh(const unsigned *first, const unsigned *last, const int num_ver, const bool correct_non_manifold):
    stitch(num_ver), border(num_ver)
{
    for (size_t i = 0; i < stitch.size(); ++i)
	stitch[i] = i;
    build(first, last);
    if (correct_non_manifold == true)
	manifoldize();
    close();
}

Mesh::Mesh(const Mesh &m):
    stitch(m.stitch)
{
    build(m);
}

Mesh::~Mesh()
{
    for (Connectivity::iterator t = he_tri.begin(); t != he_tri.end(); ++t)
	deleteTriangle(*t);
}

Mesh &Mesh::operator=(const Mesh &m)
{
    if (this == &m)
	return *this;
    for (Connectivity::iterator t = he_tri.begin(); t != he_tri.end(); ++t)
	deleteTriangle(*t);
    stitch = m.stitch;
    he_tri.clear();
    build(m);
    return *this;
}

void Mesh::build(const unsigned *first, const unsigned *last)
{
    int i = 0;
    for (const unsigned *tri = first; tri < last; tri += 3, ++i) {
	boost::array<Halfedge_t::Vertex, 3> v;
	std::copy(tri, tri + 3, v.begin());
	add(v, he_tri.size());
    }
}

void Mesh::build(const Mesh &m)
{
    for (Connectivity::const_iterator i = m.he_tri.begin(); i != m.he_tri.end(); ++i) {
	const Handle_t t = *i;
	boost::array<Halfedge_t::Vertex, 3> v;
	v[0] = t->Org();
	v[1] = t->Lnext()->Org();
	v[2] = t->Lprev()->Org();
	add(v, t->Lface());
    }
}

void Mesh::add(const Handle_t t)
{
    he_tri.push_back(t);
    Handle_t h = t;
    do {
	const int i = std::min(h->Org(), h->Dest());
	const int j = std::max(h->Org(), h->Dest());
	std::map<int, Handle_t>& el = border[i];
	std::map<int, Handle_t>::iterator it = el.find(j);
	if (it == el.end())
	    el[j] = h;
	else if (it->second->Org() == h->Dest()) {
	    Halfedge_t::paste(it->second, h);
	    el.erase(it);
	}
	h = h->Lnext();
    } while (h != t);
}

void Mesh::manifoldize()
{
    std::set<unsigned> marked;
    {
	std::vector<std::map<int, Data> > count(stitch.size());
	for (Connectivity::iterator t = he_tri.begin(); t != he_tri.end(); ++t) {
	    Handle_t it = *t;
	    do {
		int i = std::min(it->Org(), it->Dest());
		int j = std::max(it->Org(), it->Dest());
		Data& d = count[i][j];
		if (d.counter < 2)
		    ++d.counter;
		else {
		    Halfedge_t::cut(d.p);
		    Halfedge_t::cut(it);
		    marked.insert(i);
		    marked.insert(j);
		}
		d.p = it;
		it = it->Lnext();
	    } while (it != *t);
	}
    }

    std::vector<std::vector<Handle_t> > stars(stitch.size());
    for (unsigned i = 0; i < stars.size(); ++i)
	stars[i].reserve(7);
    {
	std::vector<Handle_t> vert(stitch.size());
	for (Connectivity::iterator h = he_tri.begin(); h != he_tri.end(); ++h) {
	    Handle_t it = *h;
	    do {
		vert[it->Org()] = it;
		stars[it->Org()].push_back(it);
		it = it->Lnext();
	    } while (it != *h);
	}

	for (unsigned i = 0; i < vert.size(); ++i) {
	    if (vert[i] == NULL)
		continue;
	    Handle_t h = vert[i], it = h;
	    unsigned counter = 0;
	    do {
		counter++;
		it = it->Lprev()->Sym();
	    } while (it != NULL && it != h);
	    if (it == NULL && h->Sym() != NULL) {
		h = h->Sym();
		it = h;
		do {
		    counter++;
		    it = it->Lnext()->Sym();
		} while (it != NULL);
	    }
	    assert(counter <= stars[i].size());
	    if (counter < stars[i].size())
		marked.insert(i);
	}
    }

    for (std::set<unsigned>::iterator it = marked.begin(); it != marked.end(); ++it) {
	const unsigned h_it = *it;
	typedef std::list<Handle_t> star_t;
	star_t star(stars[h_it].begin(), stars[h_it].end());
	while (star.empty() == false) {
	    int s = addStitch(h_it);
	    Handle_t ref = *star.begin(), it = ref;
	    do {
		star_t::iterator i = std::find(star.begin(), star.end(), it);
		assert(i != star.end());
		star.erase(i);
		it->Org() = s;
		it = it->Lprev()->Sym();
	    } while (it != NULL && it != ref);
	    if (it == NULL && ref->Sym() != NULL) {
		it = ref->Sym();
		do {
		    star_t::iterator i = std::find(star.begin(), star.end(), it->Lnext());
		    assert(i != star.end());
		    star.erase(i);
		    it->Lnext()->Org() = stitch.size() - 1;
		    it = it->Lnext()->Sym();
		} while (it != NULL);
	    }
	}
    }
}

void Mesh::close()
{
    for (EdgeContainer::iterator it = border.begin(); it != border.end(); ++it)
	it->clear();
    for (Connectivity::iterator ref = he_tri.begin(); ref != he_tri.end(); ++ref) {
	Handle_t it = *ref;
	do {
	    const Handle_t h = it;
	    it = it->Lnext();
	    if (h->Sym() != NULL)
		continue;
	    const int i = std::min(h->Org(), h->Dest());
	    const int j = std::max(h->Org(), h->Dest());
	    std::map<int, Handle_t>& el = border[i];
	    std::map<int, Handle_t>::iterator it = el.find(j);
	    if (it == el.end())
		el[j] = h;
	    else if (it->second->Org() == h->Dest()) {
		Halfedge_t::paste(it->second, h);
		el.erase(it);
	    }
	} while (it != *ref);
    }

    for (EdgeContainer::iterator el = border.begin(); el != border.end(); ++el) {
	for (std::map<int, Handle_t>::iterator it = el->begin(); it != el->end(); ++it) {
	    const Handle_t ref = it->second;
	    Handle_t i = ref;
	    do {
		if (i->Sym() == NULL)
		    close(i);
		i = i->Lnext();
	    } while (i != ref);
	}
    }
    border.clear();
}

void Mesh::close(Handle_t h)
{
    if (h->Sym() != NULL)
	return;
    typedef std::vector<Handle_t> loop_t;
    loop_t loop;
    Handle_t it = h;
    do {
	loop.push_back(it);
	it = it->Lnext();
	while (it->Sym() != NULL)
	    it = it->Sym()->Lnext();
    } while (it != h);

    Handle_t first = NULL, last = NULL;
    stitch.push_back(INT_MAX);
    int dummy_idx = stitch.size() - 1;
    for (loop_t::iterator i = loop.begin(); i != loop.end(); ++i) {
	const Handle_t it = *i;
	boost::array<int, 3> v;
	v[0] = it->Dest();
	v[1] = it->Org();
	v[2] = dummy_idx;
	Handle_t t = Halfedge_t::create_face(v, he_tri.size());
	he_tri.push_back(t);
	Halfedge_t::paste(it, t);
	if (first == NULL)
	    first = t->Lnext();
	else
	    Halfedge_t::paste(last, t->Lnext());
	last = t->Lprev();
    }
    Halfedge_t::paste(first, last);
}

int Mesh::getValence(const Handle_t v) const
{
    Handle_t it(v);
    int val = 1;
    while ((it = it->Onext()) != v)
	val++;
    return val;
}

Handle_t Mesh::createTriangle()
{
    std::vector<int> v(3, -1);
    return Halfedge_t::create_face(v);
}

void Mesh::deleteTriangle(const Handle_t t)
{
    HDS::Halfedge_t::delete_face(t);
}

void Mesh::erase(std::vector<bool> &v)
{
    if (v.size() != he_tri.size())
	throw std::logic_error("Erase vector must be as long as the triangle vector");

    for (unsigned i = 0; i < size(); ++i) {
	if (v[i] == false || he_tri[i] == NULL)
	    continue;
	deleteTriangle(he_tri[i]);
	he_tri[i] = NULL;
    }
    boost::unordered_map<unsigned, unsigned> link;
    unsigned valid = 0;
    for (unsigned i = 0; i < size(); ++i) {
	if (he_tri[i] == NULL)
	    continue;
	if (valid != i)
	    he_tri[valid] = he_tri[i];
	link[i] = valid;
	valid++;
    }
    he_tri.resize(valid);
    for (unsigned i = 0; i < he_tri.size(); ++i) {
	he_tri[i]->Lface() = link[he_tri[i]->Lface()];
	he_tri[i]->Lnext()->Lface() = link[he_tri[i]->Lnext()->Lface()];
	he_tri[i]->Lprev()->Lface() = link[he_tri[i]->Lprev()->Lface()];
    }
    v.clear();
    v.resize(he_tri.size(), false);
}

}
