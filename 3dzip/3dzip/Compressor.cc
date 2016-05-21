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

#include <sstream>
#include "Compressor.hh"
#include "AIMesh.hh"
#include "Symbol.hh"

namespace VBE {

Compressor::Compressor(AIMesh &m, const SymbolOutputFunc& f):
    mesh(m), output(f), tri_id(0)
{
}

void Compressor::start()
{
    if (terminated == true)
	return;

    while (tri_id < mesh.size() && mesh.isConquered(tri_id) == true)
	tri_id++;
    if (tri_id == mesh.size()) {
	terminated = true;
	output(Symbol(END_OF_MESH));
	return;
    }

    HDS::Handle_t v = mesh(tri_id);
    for (int i = 0; i < 3; i++, v = v->Lnext()) {
	output(Symbol(ADD, mesh.valence(v)));
	mesh.encodeProps(v);
    }
    mesh.conquest(v);
    s.push(new ActiveList(mesh, v));
}

void Compressor::step()
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
    const HDS::Handle_t v = al.activeEdge()->Dnext();
    if (al.isFocusFull() == true) {
    }
    else {
	int index, al_index, skip;
	if (mesh.isVisited(v) == false)
	    sym = Symbol(ADD, mesh.valence(v));
	else if (al.isIn(al.focus(), v, index, i, skip))
	    sym = Symbol(SPLIT, index, skip);
	else if (s.isIn(al.focus(), v, index, al_index, i, skip))
	    sym = Symbol(MERGE, index, skip, al_index);
	else {
	    std::ostringstream oss;
	    oss << "Unknown vertex: " << v->Org();
	    throw std::logic_error(oss.str());
	}
	output(sym);
	if (sym.type == ADD)
	    mesh.encodeProps(v, al.activeEdge());
    }
    mesh.conquest(v);
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
