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
#include <deque>
#include "ActiveList.hh"

namespace VBE {

typedef std::deque<ActiveList*> ALStack;

class ActiveStack
{
    ALStack stack;
public:
	bool isIn(const int ref, const HDS::Handle_t v, int& idx, int& al_index, ActiveList::Iterator& i, int& skip);
	const ActiveList& operator[](const int i) const {
	    return *stack[stack.size() - 1 - i];
	}
	ActiveList& operator[](const int i) {
	    return *stack[stack.size() - 1 - i];
	}
	bool empty() const {
	    return stack.empty();
	}
	ALStack::size_type size() const {
	    return stack.size();
	}
	ActiveList* top() {
	    return stack.back();
	}
	const ActiveList* top() const {
	    return stack.back();
	}
	void pop() {
	    stack.pop_back();
	}
	void push(ActiveList *al) {
	    if (al->hasAtLeast(3) == true)
		stack.push_back(al);
	}
};

}
