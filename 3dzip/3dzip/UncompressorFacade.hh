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

#ifndef UNCOMPRESSOR_FACADE_HH
#define UNCOMPRESSOR_FACADE_HH

#include "ManifoldUncompressorFacade.hh"

namespace VBE {

struct StitchSymbol;

class UncompressorFacade : public ManifoldUncompressorFacade
{
    typedef boost::function<void(StitchSymbol&)> StitchInput;
    StitchInput sip;
    std::vector<int> stitches;

    virtual bool has_attributes(int &v);
    int reqStitch(const size_t n) {
	int i = mesh->addStitch(n);
	free_slots.push_back(-1);
	return i;
    }

public:
    UncompressorFacade(HDS::Mesh& m, const unsigned num_ver, const StitchInput& sip):
	ManifoldUncompressorFacade(m, num_ver), sip(sip) {}
    virtual ~UncompressorFacade() {}
};

}

#endif
