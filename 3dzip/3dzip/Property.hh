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

#ifndef PROPERTY_HH
#define PROPERTY_HH

#include <boost/function.hpp>

namespace VBE {

template <class P, template <typename> class PredictionPolicy>
class OutputProperty:
    public PredictionPolicy<P>
{
    typedef typename P::value_type value_type;

    typedef boost::function<void(const value_type&)> OutputFun;
    OutputFun out;

public:
    OutputProperty(const P& p, const OutputFun& out) : PredictionPolicy<P>(p), out(out) {}
    typedef void result_type;
    void operator()(const HDS::Mesh& m, const HDS::Handle_t v, const HDS::Handle_t active_edge) {
	value_type d = PredictionPolicy<P>::output1(m, v, active_edge);
	out(d);
    }
};

template <class P, template <typename> class PredictionPolicy>
class InputProperty:
    public PredictionPolicy<P>
{
    typedef typename P::value_type value_type;

    typedef boost::function<void(value_type&)> InputFun;
    InputFun in;

public:
    InputProperty(P& p, const InputFun& in) : PredictionPolicy<P>(p), in(in) {}
    typedef void result_type;
    void operator()(const HDS::Mesh &m, const int v, const HDS::Handle_t active_edge) {
	value_type input;
	in(input);
	PredictionPolicy<P>::input1(m, input, v, active_edge);
    }
};

}

#endif
