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

#include "UncompressorFacade.hh"
#include "StitchSymbol.hh"

namespace VBE {

bool UncompressorFacade::has_attributes(int &v)
{
    StitchSymbol sym(stitches.size());
    sip(sym);

    switch (sym.type) {
	case ST_NONE:
	    v = reqVertex();
	    return true;
	case ST_PUSH:
	    v = reqVertex();
	    stitches.push_back(mesh->element(v));
	    return true;
	case ST_GET:
	    {
	    v = reqStitch(stitches.at(sym.index));
	    return false;
	    }
	default:
	    throw std::logic_error("Invalid stitch symbol read");
    }
    return false;
}

}