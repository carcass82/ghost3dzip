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

#include "ActiveStack.hh"

namespace VBE {

bool
ActiveStack::isIn(const int ref, const HDS::Handle_t v, int& idx, int& al_index, ActiveList::Iterator& it, int& skip)
{
	if (stack.empty() == true)
		return false;

	al_index = (int)stack.size() - 1;
	ALStack::iterator i = stack.begin();
	while (i != stack.end() && (*i)->isIn(ref, v, idx, it, skip) == false)
		++i, --al_index;

	return i != stack.end();
}

}
