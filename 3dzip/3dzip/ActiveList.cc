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

#include <iostream>
#include <set>
#include <climits>
#include "ActiveList.hh"
#include "Mesh.hh"
#include <hds/Edge.hh>

namespace VBE {

ActiveList::ActiveList(const Mesh &m, const HDS::Handle_t t):
    mesh(m)
{
    HDS::Handle_t it = t;
    for (int i = 0; i < 3; ++i, it = it->Lnext())
	l.push_back(Elem(it, m.freeSlots(it) - 2));
    selectFocus();
}

void ActiveList::add()
{
    --l.back().free_slots;
    --l.front().free_slots;
    l.back().h = l.back().h->Sym()->Lnext();
    HDS::Handle_t v = l.back().h->Lnext();
    int free_slots = mesh.freeSlots(v) - 2;
    if (free_slots < l.front().free_slots)
	l.push_front(Elem(v, free_slots));
    else
	l.push_back(Elem(v, free_slots));
}

bool ActiveList::isIn(const int ref, const int v, int &idx, Iterator &i, int& skip) const
{
    idx = 0;
    i = const_cast<List&>(l).begin();
    while (i != l.end() && !(i->h->Org() == v && can_split(ref, i) == true))
	++idx, ++i;
    if (i == l.end())
	return false;
    HDS::Handle_t h = i->h->Sym()->Lnext();
    skip = 0;
    while (h->Sym()->Lnext()->Dest() != ref) {
	h = h->Sym()->Lnext();
	++skip;
    }
    return true;
}

bool ActiveList::can_split(const int ref, const ConstIterator &i) const
{
    HDS::Handle_t it = i->h;
    while (it->Dest() != ref)
	it = it->Onext();
    const HDS::Handle_t end = circPrev(i)->h;
    while (it->Dest() != end->Org()) {
	if (mesh.isConquered(it->Lface()) == true)
	    return false;
	it = it->Oprev();
    }
    return true;
}

ActiveList *ActiveList::split(const Iterator &i, const int skip)
{
    int free_slots = i->free_slots;
    if (l.back().free_slots > 0) {
	--l.back().free_slots;
	--free_slots;
    }
    --l.front().free_slots;
    --free_slots;
    i->free_slots = skip;
    free_slots -= i->free_slots;
    l.back().h = l.back().h->Sym()->Lnext();
    ActiveList* al = new ActiveList(mesh);
    List& l2 = al->l;
    l2.splice(l2.begin(), l, i, l.end());
    if (free_slots < l.front().free_slots)
	l.push_front(Elem(l2.back().h->Lnext(), free_slots));
    else
	l.push_back(Elem(l2.back().h->Lnext(), free_slots));
    al->selectFocus();
    return al;
}

void ActiveList::merge(ActiveList &al_merge, const Iterator &i, const int skip)
{
    int free_slots = i->free_slots;
    if (l.back().free_slots > 0) {
	--l.back().free_slots;
	--free_slots;
    }
    --l.front().free_slots;
    --free_slots;
    i->free_slots = skip;
    free_slots -= i->free_slots;
    List &l2 = al_merge.l;
    l.back().h = l.back().h->Sym()->Lnext();
    l2.insert(i, Elem(l.back().h->Lnext(), free_slots));
    l.splice(l.end(), l2, i, l2.end());
    l.splice(l.end(), l2, l2.begin(), l2.end());
    selectFocus();
}

void ActiveList::remove()
{
    if (hasAtLeast(4) == false) {
	l.clear();
	return;
    }
    --l.back().free_slots;
    l.back().h = l.back().h->Oprev();
    l.erase(l.begin());
    --l.front().free_slots;
    selectFocus();
}

void ActiveList::selectFocus()
{
    Iterator f = l.end();
    int f_fs = INT_MAX;
    for (Iterator i = l.begin(); i != l.end(); ++i) {
	int i_fs = i->free_slots;
	if (i_fs < f_fs)
	    f = i, f_fs = i_fs;
	if (f_fs < 3)
	    break;
    }
    List::iterator next = circNext(f), prev = circPrev(f);
    if (next->free_slots == 0)
	f = next;
    else if (prev->free_slots == 0)
	f = prev;
    if (f != l.begin())
	l.splice(l.begin(), l, f, l.end());
}

std::ostream &operator <<(std::ostream& out, const VBE::ActiveList& al)
{
    out << "Active List:\n";
    for (ActiveList::List::const_iterator v = al.l.begin(); v != al.l.end(); ++v)
	out << v->h->Org() << " -> " << v->h->Dest() << std::endl;
    return out;
}

}
