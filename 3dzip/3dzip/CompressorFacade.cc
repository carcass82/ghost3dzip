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

#include <algorithm>
#include "CompressorFacade.hh"

namespace VBE {

CompressorFacade::CompressorFacade(HDS::Mesh& m, const StitchOutput& sop) : ManifoldCompressorFacade(m), sop(sop)
{
}

bool CompressorFacade::has_attributes(const HDS::Handle_t v)
{
    if (mesh->isDummy(v) == true)
	return false;
    if (mesh->isStitch(v) == false) {
	sop(StitchSymbol(ST_NONE));
	return true;
    }
    std::vector<int>::iterator it = std::find(stitches.begin(), stitches.end(), mesh->element(v));
    if (it == stitches.end()) {
	sop(StitchSymbol(ST_PUSH));
	stitches.push_back(mesh->element(v));
	return true;
    }
    unsigned int index = std::distance(stitches.begin(), it);
    sop(StitchSymbol(ST_GET, index, stitches.size()));
    return false;
}

}
