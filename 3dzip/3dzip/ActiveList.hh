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
#include <list>
#include "hds/HDS.hh"

namespace VBE {

class Mesh;

class ActiveList
{
public:
    struct Elem
    {
	HDS::Handle_t h;
	unsigned free_slots;
	Elem(HDS::Handle_t h, unsigned n = 0):
	    h(h), free_slots(n) {}
    };
    typedef std::list<Elem> List;
    typedef List::iterator Iterator;
    typedef List::const_iterator ConstIterator;

private:
    List l;
    const Mesh &mesh;

    Iterator circNext(const Iterator &it) {
	Iterator n = it;
	n++;
	if (n == l.end())
	    n = l.begin();
	return n;
    }
    ConstIterator circNext(const ConstIterator &it) const {
	ConstIterator n = it;
	++n;
	if (n == l.end())
	    n = l.begin();
	return n;
    }
    Iterator circPrev(const Iterator &it) {
	Iterator n = it;
	if (n == l.begin())
	    n = l.end();
	n--;
	return n;
    }
    ConstIterator circPrev(const ConstIterator &it) const {
	ConstIterator n = it;
	if (n == l.begin())
	    n = l.end();
	n--;
	return n;
    }
    bool can_split(const int ref, const ConstIterator &i) const;
    void selectFocus();
    ActiveList(const Mesh &m): mesh(m) {}

public:
    ActiveList(const Mesh &m, const HDS::Handle_t t);
    bool empty() const {
	return l.empty();
    }
    bool isIn(const int ref, const int v, int &idx, Iterator &it, int& skip) const;
    bool isIn(const int ref, const HDS::Handle_t v, int &idx, Iterator &i, int& skip) const {
	return isIn(ref, v->Org(), idx, i, skip);
    }

    void add();
    Iterator operator[](const int idx) {
	Iterator i = l.begin();
	std::advance(i, idx);
	return i;
    }
    ConstIterator operator[](const int idx) const {
	ConstIterator i = l.begin();
	std::advance(i, idx);
	return i;
    }
    ActiveList *split(const Iterator &i, const int skip);
    void merge(ActiveList &al_merge, const Iterator &i, const int skip);

    int focus() const {
	return l.front().h->Org();
    }
    bool isFocusFull() const {
	return l.front().free_slots <= 0;
    }
    HDS::Handle_t activeEdge() const {
	return l.back().h;
    }
    void remove();
    ConstIterator begin() const {
	return l.begin();
    }
    ConstIterator end() const {
	return l.end();
    }
    bool hasAtLeast(const unsigned n) const {
	ConstIterator it = l.begin();
	unsigned count = 0;
	while (count++ < n) {
	    if (it == l.end())
		return false;
	    ++it;
	}
	return true;
    }
    friend std::ostream& operator <<(std::ostream& out, const VBE::ActiveList& al);
};

std::ostream& operator <<(std::ostream& out, const VBE::ActiveList& al);

}
