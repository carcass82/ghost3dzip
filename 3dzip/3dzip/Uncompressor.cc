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

#include "Uncompressor.hh"
#include "AOMesh.hh"
#include "Symbol.hh"

namespace VBE {

Uncompressor::Uncompressor(AOMesh &m, const SymbolInputFunc& f):
    mesh(m), input(f)
{
}

void Uncompressor::start()
{
    if (terminated == true)
	return;

    AOMesh::Triangle t;
    for (int i = 0; i < 3; ++i) {
	Symbol sym;
	input(sym);
	if (sym.type == END_OF_MESH) {
	    terminated = true;
	    return;
	}
	if (sym.type != ADD)
	    throw std::logic_error("Three ADD operation needed");
	t[i] = mesh.decodeProps(sym.val1);
    }
    HDS::Handle_t v = mesh.conquest(t);
    s.push(new ActiveList(mesh, v));
}

void Uncompressor::step()
{
    if (s.empty() == true) {
	start();
	return;
    }
    ActiveList &al = *s.top();
    if (al.empty() == true) {
	delete &al;
	s.pop();
	return;
    }
    ActiveList::Iterator i;
    Symbol sym;
    AOMesh::Triangle t;
    const HDS::Handle_t active_edge = al.activeEdge();
    t[1] = active_edge->Dest();
    t[2] = active_edge->Org();
    if (al.isFocusFull() == true) {
	t[0] = al[1]->h->Org();
    }
    else {
	input(sym);
	switch (sym.type) {
	    case ADD:
		t[0] = mesh.decodeProps(sym.val1, active_edge);
	    break;
	    case SPLIT:
		i = al[sym.val1];
		t[0] = i->h->Org();
	    break;
	    case MERGE:
		i = s[sym.val3][sym.val1];
		t[0] = i->h->Org();
	    break;
	    default:
		throw std::logic_error("Invalid symbol received");
	}
    }
    HDS::Handle_t v = mesh.conquest(t);
    switch (sym.type) {
	case NONE:
	    al.remove();
	break;
	case ADD:
	    al.add();
	break;
	case SPLIT:
	    s.pop();
	    s.push(al.split(i, sym.val2));
	    s.push(&al);
	break;
	case MERGE:
	    al.merge(s[sym.val3], i, sym.val2);
	break;
	default:
	    throw std::logic_error("Invalid symbol received");
    }
}

}
