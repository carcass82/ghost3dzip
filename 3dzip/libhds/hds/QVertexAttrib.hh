/*
    Copyright (C) 2009 Paolo Simone Gasparello <djgaspa@gmail.com>

    This file is part of libhds.

    libhds is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libhds is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libhds.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QVERTEX_ATTRIB_HH
#define QVERTEX_ATTRIB_HH

#include <climits>
#include <cfloat>
#include <Eigen/Core>
#include <Eigen/Array>

template <typename T = float>
class BoundingBox
{
public:
    typedef Eigen::Matrix<T, Eigen::Dynamic, 1> value_type;

private:
    const int N;
    static T maxAbsValue();

public:
    value_type offset, scale;

    explicit BoundingBox(const int dim) : N(dim), offset(N), scale(N) {}
    BoundingBox(const T* first, const T* last, const size_t dim);

    void serialize(std::ostream& os) const;
    void unserialize(std::istream& os);
};

template <typename T>
BoundingBox<T>::BoundingBox(const T* first, const T* last, const size_t dim):
    N(dim), offset(N), scale(N)
{
    Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > v(first, N, std::max(1, int((last - first) / N)));
    value_type pos_min = value_type::Constant(N, maxAbsValue()), pos_max = value_type::Constant(N, -maxAbsValue());
    for (int i = 0; i < v.cols(); i++) {
    pos_min = pos_min.cwise().min(v.col(i));
    pos_max = pos_max.cwise().max(v.col(i));
    }
    offset = pos_min;
    scale = pos_max - pos_min;
}

template<>
inline
int BoundingBox<int>::maxAbsValue()
{
    return INT_MAX;
}

template<>
inline
float BoundingBox<float>::maxAbsValue()
{
	return FLT_MAX;
}

template<typename T>
void BoundingBox<T>::serialize(std::ostream& os) const
{
	for (int i = 0; i < N; ++i) {
	T tmp = offset[i];
	os.write((const char*)&tmp, sizeof(tmp));
	tmp = scale[i];
	os.write((const char*)&tmp, sizeof(tmp));
	}
}

template<typename T>
void BoundingBox<T>::unserialize(std::istream& is)
{
	for (int i = 0; i < N; ++i) {
	T tmp;
	is.read((char*)&tmp, sizeof(tmp));
	offset[i] = tmp;
	is.read((char*)&tmp, sizeof(tmp));
	scale[i] = tmp;
	}
}

template <typename S = float>
class QVertexAttrib
{
    Eigen::MatrixXi m;
    unsigned char q;
    BoundingBox<S> bb;

	int max_value() const {
	return (1 << q) - 1;
	}
public:
    typedef Eigen::Matrix<int, Eigen::Dynamic, 1> value_type;
    typedef Eigen::MatrixXi::ColXpr reference;

    QVertexAttrib(const S* first, const S* last, const size_t dim, unsigned char q = 12);
    QVertexAttrib(const unsigned char q, const BoundingBox<S>& bb, const unsigned size, const int dim):
    m(dim, std::max(1u, size)), q(q), bb(bb) {}
    int rows() const { return m.rows(); }
    int size() const { return m.cols(); }
    const unsigned char getQ() const {
    return q;
    }
    const BoundingBox<S>& getBoundingBox() const {
    return bb;
    }
    value_type getCenter() const {
    return value_type::Constant(m.rows(), 1, (1 << (q - 1)) - 1);
    }
    template <typename Iter>
    void dequantize(Iter) const;
    void clamp(value_type& v) const {
    v = v.cwise().max(value_type::Zero(m.rows())).cwise().min(value_type::Constant(m.rows(), max_value()));
    }
    Eigen::MatrixXi::ColXpr operator[](int i) {
    return m.col(i);
    }
    const Eigen::MatrixXi::ColXpr operator[](int i) const {
    return m.col(i);
    }

    Eigen::MatrixXi::RowXpr setRow(int i = 0) {
        return m.row(i);
    }
    const Eigen::MatrixXi::RowXpr getRow(int i = 0) {
        return m.row(i);
    }
};

template <typename S>
QVertexAttrib<S>::QVertexAttrib(const S* first, const S* last, const size_t dim, unsigned char q):
    m((int)dim, std::max(1u, unsigned((last - first) / dim))), q(q), bb(first, last, dim)
{
    Eigen::Map<Eigen::Matrix<S, Eigen::Dynamic, Eigen::Dynamic> > v(first, dim, std::max(1u, unsigned((last - first) / dim)));
    const int Q = max_value();
    for (int c = 0; c < m.cols(); ++c) {
    Eigen::Matrix<S, Eigen::Dynamic, 1> val = (v.col(c) - bb.offset).cwise() / bb.scale;
    for (int i = 0; i < m.rows(); i++) {
        val[i] = std::max<S>(0.0f, val[i]);
        val[i] = std::min<S>(1.0f, val[i]);
        m.col(c)[i] = (val[i] == 1.0f) ? Q - 1 : (int)floor(val[i] * Q);
    }
    }
}

template <typename S>
template <typename Iter>
void QVertexAttrib<S>::dequantize(Iter v) const
{
    const int Q = max_value();
    for (int i = 0; i < m.cols(); ++i) {
    value_type m = (*this)[i];
    Eigen::Matrix<S, Eigen::Dynamic, 1> p = ((m.cast<S>().cwise() + 0.5f) / Q).cwise() * bb.scale + bb.offset;
    std::copy(p.data(), p.data() + m.rows(), v + i * m.rows());
    }
}

#endif
