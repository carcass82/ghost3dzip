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

#ifndef COMPRESSOR_HH
#define COMPRESSOR_HH

#include <boost/function.hpp>
#include "VBE.hh"

namespace VBE {

class AIMesh;
struct Symbol;

class Compressor : public Base
{
    AIMesh &mesh;
    typedef boost::function<void(const Symbol&)> SymbolOutputFunc;
    SymbolOutputFunc output;
    size_t tri_id;

    void start();

public:
    Compressor(AIMesh &m, const SymbolOutputFunc& f);
    void step();
};

}

#endif
