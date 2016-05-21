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

#ifndef PREDICTOR_HH
#define PREDICTOR_HH

#include "hds/HDS.hh"

namespace VBE {

    template <typename T>
    class OutputPredictor
    {
    protected:
	const T& p;
	const typename T::value_type data(const HDS::Mesh &m, const int v) const {
	    return p[m.element(v)];
	}
    public:
	OutputPredictor(const T& p): p(p) {}
	virtual ~OutputPredictor() {}
    };

    template <typename T>
    class InputPredictor
    {
    protected:
	typedef typename T::value_type value_type;
	typedef typename T::reference reference;
	T& p;
	const value_type data(const HDS::Mesh &m, const int v) const {
	    return p[m.element(v)];
	}
	reference data(const HDS::Mesh &m, const int v) {	
	    return p[m.element(v)];
	}

    public:
	InputPredictor(T& p): p(p) {}
	virtual ~InputPredictor() {}
    };

    template <typename T>
    class OutputNullPredictor : public OutputPredictor<T>
    {
	using OutputPredictor<T>::data;
    protected:
	typename OutputPredictor<T>::T output1(const Mesh &m, const HDS::Handle_t v, const HDS::Handle_t active_edge) const {
	    return data(m, v->Org());
	}
    public:
	OutputNullPredictor(const T& p): OutputPredictor<T>(p) {}
    };

    template <typename T>
    class InputNullPredictor : public InputPredictor<T>
    {
	using InputPredictor<T>::data;
    protected:
	void input1(const HDS::Mesh &m, const T &in, const int v, const HDS::Handle_t active_edge) {
	    data(m, v) = in;
	}
    public:
	InputNullPredictor(T& p): InputPredictor<T>(p) {}
    };

    template <typename T>
    typename T::value_type parallelogram_prediction(const T& p, const HDS::Mesh &m, const HDS::Handle_t i)
    {
	if (i == NULL)
	    return p.getCenter();
	typename T::value_type v;
	if (m.isDummyTriangle(i) == true)
	    if (m.isDummy(i->Org()) == true)
		v = p[m.element(i->Dest())];
	    else
		v = p[m.element(i->Org())];
	else
	    v = p[m.element(i->Org())] + p[m.element(i->Dest())] - p[m.element(i->Lnext()->Dest())];
	p.clamp(v);
	return v;
    }

    template <typename T>
    class OutputParallelogramPredictor : public OutputPredictor<T>
    {
	using OutputPredictor<T>::data;
	using OutputPredictor<T>::p;
	typename T::value_type predict(const HDS::Mesh &m, const HDS::Handle_t i) const {
	    return parallelogram_prediction(p, m, i);
	}

	protected:
	typename T::value_type output1(const HDS::Mesh &m, const HDS::Handle_t v, const HDS::Handle_t active_edge) const {
	    return predict(m, active_edge) - data(m, v->Org());
	}
	public:
	OutputParallelogramPredictor(const T& p): OutputPredictor<T>(p) {}
	virtual ~OutputParallelogramPredictor() {}
    };

    template <typename T>
    class InputParallelogramPredictor : public InputPredictor<T>
    {
	using InputPredictor<T>::data;
	using InputPredictor<T>::p;
	typename T::value_type predict(const HDS::Mesh &m, const HDS::Handle_t i) const {
	    return parallelogram_prediction(p, m, i);
	}

	protected:
	void input1(const HDS::Mesh &m, const typename T::value_type &in, const int v, const HDS::Handle_t active_edge) {
	    data(m, v) = predict(m, active_edge) - in;
	}
	public:
	InputParallelogramPredictor(T& p): InputPredictor<T>(p) {}
    };

}

#endif
